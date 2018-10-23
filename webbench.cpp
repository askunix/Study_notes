1.源码下载链接：
http://home.tiscali.cz/~cz210552/webbench.html

2.webbench源码所走流程：
（1）解析命令行参数，根据命令行指定参数设定变量，可以认为是初始化配置。
（2）根据指定的配置构造 HTTP 请求报文格式。
（3）开始执行 bench 函数，先进行一次 socket 连接建立与断开，测试是否可以正常访问。
（4）建立管道，派生根据指定进程数派生子进程。
（5）每个子进程调用 benchcore 函数，先通过 sigaction 安装信号，用 alarm 设置闹钟函数，接着不断建立 socket 进行通信，与服务器交互数据，直到
    收到信号结束访问测试。子进程将访问测试结果写进管道。
（6）父进程读取管道数据，汇总子进程信息，收到所有子进程消息后，输出汇总信息，结束。
 
3.webbench参数代表含义：
 -f  | --force             //不等待服务器返回数据
 -r  | --reload           // 失败时再次发送请求
  -t  | --time <sec>     //指定压力测试运行时间（默认30秒）
  -p | --proxy <server:port>    //使用代理服务器发起请求
  -c | --clients <n>     //指定多少个客户端发起测试请求
  -9 | --http09              //使用协议版本HTTP/0.9 
  -1 | --http10             // 使用协议版本HTTP/1.0
  -2 | --http11            //使用协议版本HTTP/1.1
  --get                       //使用GET请求方法
  --head                    //使用HEAD请求方法
  --options               //使用OPTIONS请求方法
  --trace                 //使用TRACE请求方法
  -? | -h | --help            //帮助信息
  -V | --version             //显示webbench版本信息
  
  
4.源码解读
socket.c
/* $Id: socket.c 1.1 1995/01/01 07:11:14 cthuang Exp $
 *
 * This module has been modified by Radim Kolar for OS/2 emx
 */

/***********************************************************************
  module:       socket.c
  program:      popclient
  SCCS ID:      @(#)socket.c    1.5  4/1/94
  programmer:   Virginia Tech Computing Center
  compiler:     DEC RISC C compiler (Ultrix 4.1)
  environment:  DEC Ultrix 4.3 
  description:  UNIX sockets code.
 ***********************************************************************/
 
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int Socket(const char *host, int clientPort)
{
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;        //声明一个ipv4类型的结构体
    struct hostent* hp;
    
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;     //协议族选择ipv4协议

    inaddr = inet_addr(host);    //将点分十进制字符串表示的ipv4地址转化为用网络字节序二进制表示的ipv4地址
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));   //给ad结构体中的sin_addr成员赋值
    else
    {
        hp = gethostbyname(host);   //可通过主机名来获取一个结构体，里面包含有主机的ip地址
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort);     //将主机字节序转化为网络字节序（端口号）
    
    sock = socket(AF_INET, SOCK_STREAM, 0);  //创建套接字
    if (sock < 0)
        return sock;
    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)   //与远程服务器建立连接
        return -1;
    return sock;
}

 
webbench.c
/*
 * (C) Radim Kolar 1997-2004
 * This is free software, see GNU Public License version 2 for
 * details.
 *
 * Simple forking WWW Server benchmark:
 *
 * Usage:
 *   webbench --help
 *
 * Return codes:
 *    0 - sucess
 *    1 - benchmark failed (server is not on-line)
 *    2 - bad param
 *    3 - internal error, fork failed
 * 
 */ 
#include "socket.c"
#include <unistd.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <getopt.h>
#include <strings.h>
#include <time.h>
#include <signal.h>

/* values */
volatile int timerexpired=0;  //用来检测测试时长是否达到指定时长（加上volatile保证每次数据从内存中获得，避免从寄存器中取带来的错误）
int speed=0;                  //记录得到服务器响应的数量
int failed=0;                 //记录失败的连接数量
int bytes=0;                 //记录读取成功的字节数
/* globals */
int http10=1; /* 0 - http/0.9, 1 - http/1.0, 2 - http/1.1 */
/* Allow: GET, HEAD, OPTIONS, TRACE */
                             //默认为http/1.0
#define METHOD_GET 0         //定义一个GET方法的宏
#define METHOD_HEAD 1        //定义一个HEAD方法的宏
#define METHOD_OPTIONS 2     //定义选择请求方法的宏
#define METHOD_TRACE 3      //
#define PROGRAM_VERSION "1.5"
int method=METHOD_GET;      //设置请求方法为GET
int clients=1;              //初始化客户端连接数为1，可通过-c选项设置客户端数量
int force=0;               //是否等待从服务器获取数据，初始化0表示要等待，1表示不等待
int force_reload=0;        //失败时重新请求  
int proxyport=80;          //设置代理服务器端口号为80
char *proxyhost=NULL;     //表示代理服务器的IP
int benchtime=30;         //默认的压力测试时长为30S
/* internal */
int mypipe[2];            //声明两个管道，父进程要从管道读端读取子进程的输出信息
char host[MAXHOSTNAMELEN];   //服务器ip
#define REQUEST_SIZE 2048
char request[REQUEST_SIZE];   //http请求信息

static const struct option long_options[]=
{
    {"force",no_argument,&force,1},
    {"reload",no_argument,&force_reload,1},
    {"time",required_argument,NULL,'t'},
    {"help",no_argument,NULL,'?'},
    {"http09",no_argument,NULL,'9'},       //对应协议版本http/0.9
    {"http10",no_argument,NULL,'1'},       //对应协议版本http/1.0
    {"http11",no_argument,NULL,'2'},       //对应协议版本http/1.1
    {"get",no_argument,&method,METHOD_GET},
    {"head",no_argument,&method,METHOD_HEAD},
    {"options",no_argument,&method,METHOD_OPTIONS},
    {"trace",no_argument,&method,METHOD_TRACE},
    {"version",no_argument,NULL,'V'},
    {"proxy",required_argument,NULL,'p'},
    {"clients",required_argument,NULL,'c'},
    {NULL,0,NULL,0}
};

/* prototypes */    //需要实现的三个函数
static void benchcore(const char* host,const int port, const char *request);
static int bench(void);
static void build_request(const char *url);

//时钟信号的处理函数，webbench采用时钟信号的方式控制对服务端的访问时长
//函数中将timerexpired设置为1，执行时检测此变量为1时停止对服务器端的访问。
static void alarm_handler(int signal)
{
   timerexpired=1;
}	

 //当命令行参数传入不正确时调用usage函数提示用户怎么用，以下是webbench使用时的参数选择
static void usage(void)     
{
   fprintf(stderr,
	"webbench [option]... URL\n"
	"  -f|--force               Don't wait for reply from server.\n"
	"  -r|--reload              Send reload request - Pragma: no-cache.\n"
	"  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"
	"  -p|--proxy <server:port> Use proxy server for request.\n"
	"  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"
	"  -9|--http09              Use HTTP/0.9 style requests.\n"
	"  -1|--http10              Use HTTP/1.0 protocol.\n"
	"  -2|--http11              Use HTTP/1.1 protocol.\n"
	"  --get                    Use GET request method.\n"
	"  --head                   Use HEAD request method.\n"
	"  --options                Use OPTIONS request method.\n"
	"  --trace                  Use TRACE request method.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"
	);
};
//主要的几个参数代表的含义：
//  -f     指定- f时不等待服务器数据返回
//  -t     指定压力测试运行时间
//  -c     指定多少个客户端发起测试请求
//  -9  -1  -2 分别为指定 HTTP/0.9   HTTP/1.0   HTTP/1.1

int main(int argc, char *argv[])
{
    int opt=0;
    int options_index=0;
    char *tmp=NULL;

    if(argc==1)
    {
	     usage();
         return 2;
    } 
	// getopt_long 为命令行解析的库函数，可通过 man 3 getopt_long 查看 
    while((opt=getopt_long(argc,argv,"912Vfrt:p:c:?h",long_options,&options_index))!=EOF )  //long_options是一个结构体数组
    {
        switch(opt)    //如果有返回对应的命令行参数 
        {
           case  0 : break;
		   case 'f': force = 1; break;            //设置为1表示需要等待服务器端返回数据
           case 'r': force_reload=1;break;        //失败时重新请求
           case '9': http10=0;break;              //代表协议版本http/0.9
           case '1': http10=1;break;              //代表协议版本http/1.0
           case '2': http10=2;break;              //代表协议版本http/1.1
           case 'V': printf(PROGRAM_VERSION"\n");exit(0);
           case 't': benchtime=atoi(optarg);break;	     //传进来的时长保存在benchtime变量
           case 'p': 
	            /* proxy server parsing server:port */
	                 tmp=strrchr(optarg,':');         //查找冒号在optarg字符串中最后一次出现的位置，实际在找端口号
	       proxyhost=optarg;
	       if(tmp==NULL)
	       {
		      break;
	       }
	       if(tmp==optarg)       //没有输入IP
	       {
		      fprintf(stderr,"Error in option --proxy %s: Missing hostname.\n",optarg);
		      return 2;
	       }
	       if(tmp==optarg+strlen(optarg)-1)     //没有输入端口号
	       {
		      fprintf(stderr,"Error in option --proxy %s Port number is missing.\n",optarg);
		      return 2;
	       }
	       *tmp='\0';
	       proxyport=atoi(tmp+1);            //将端口号存在proxyport变量中
		   break;
          
		   case ':':
           case 'h':
           case '?': usage();
			         return 2;
					 break;
           case 'c': clients=atoi(optarg);
			         break;      //客户端数量存在clients变量中
    }
 }
	//optind 被 getopt_long 设置为命令行参数中未读取的下一个元素下标值
  if(optind==argc) 
  {
      fprintf(stderr,"webbench: Missing URL!\n");
	  usage();
	  return 2;
   }
  //不能指定客户端数和请求时间为 0
  if(clients==0)           
	 clients=1;
  if(benchtime==0) 
	 benchtime=60;
  /* Copyright */
  fprintf(stderr,"Webbench - Simple Web Benchmark "PROGRAM_VERSION"\n"
	 "Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.\n" );
  build_request(argv[optind]);      //构造HTTP请求到request数组 
  /* print bench info */
  printf("\nBenchmarking: ");
  switch(method)
  {
	 case METHOD_GET:
	 default:
		 printf("GET");break;
	 case METHOD_OPTIONS:
		 printf("OPTIONS");break;
	 case METHOD_HEAD:
		 printf("HEAD");break;
	 case METHOD_TRACE:
		 printf("TRACE");break;
   }
  printf(" %s",argv[optind]);
  switch(http10)
  {
	 case 0: printf(" (using HTTP/0.9)");
		 break;
	 case 2: printf(" (using HTTP/1.1)");
		 break;
  }
  printf("\n");
  if(clients==1) 
	  printf("1 client");
  else
      printf("%d clients",clients);

  printf(", running %d sec", benchtime);    //和上面连起来代表多少个客户端连接持续了多少秒
  if(force)
	  printf(", early socket close");
  if(proxyhost!=NULL) 
	  printf(", via proxy server %s:%d",proxyhost,proxyport);   //得出代理服务器的ip和port
  if(force_reload)
	  printf(", forcing reload");
      printf(".\n");
  return bench();                   //开始压力测试，返回 bench 函数执行结果 
}

void build_request(const char *url)
{
  char tmp[10];
  int i;

  bzero(host,MAXHOSTNAMELEN);         //初始化
  bzero(request,REQUEST_SIZE);
  //判断应该使用的http协议版本
  if(force_reload && proxyhost!=NULL && http10<1) 
	  http10=1;
  if(method==METHOD_HEAD && http10<1)
	  http10=1;
  if(method==METHOD_OPTIONS && http10<2) 
	  http10=2;
  if(method==METHOD_TRACE && http10<2) 
	  http10=2;
  //获得method方法存到request中
  switch(method)
  {    
	  default:
	  case METHOD_GET: strcpy(request,"GET");break;
	  case METHOD_HEAD: strcpy(request,"HEAD");break;
	  case METHOD_OPTIONS: strcpy(request,"OPTIONS");break;
	  case METHOD_TRACE: strcpy(request,"TRACE");break;
  }
		  
  strcat(request," ");    //在请求方法后面拼接上一个空格
  //判断URL的合法性
  if(NULL==strstr(url,"://"))
  {
	  fprintf(stderr, "\n%s: is not a valid URL.\n",url);
	  exit(2);
  }
  if(strlen(url)>1500)    
  {
      fprintf(stderr,"URL is too long.\n");
	  exit(2);
  }
  //webbench只支持http协议
  if(proxyhost==NULL)
	   if (0!=strncasecmp("http://",url,7)) 
	   { fprintf(stderr,"\nOnly HTTP protocol is directly supported, set --proxy for others.\n");
             exit(2);
           }
  /* protocol/host delimiter */
  i=strstr(url,"://")-url+3;        //找到域名开始的位置
  /* printf("%d\n",i); */

  if (strchr(url + i, '/') == NULL)     // 域名或者IP都必须以 / 作为结束标志
  {
      fprintf(stderr,"\nInvalid URL syntax - hostname don't ends with '/'.\n");
      exit(2);
  }
  if (proxyhost == NULL)
  {
      /* get port from hostname */
      if(index(url+i,':')!=NULL && index(url+i,':')<index(url+i,'/'))
      {
	       strncpy(host,url+i,strchr(url+i,':')-url-i);
	       bzero(tmp,10);
	       strncpy(tmp,index(url+i,':')+1,strchr(url+i,'/')-index(url+i,':')-1);
	       /* printf("tmp=%s\n",tmp); */
	       proxyport=atoi(tmp);
	       if(proxyport==0) proxyport=80;
       } 
      else
      {
           strncpy(host,url+i,strcspn(url+i,"/"));
      }
       // printf("Host=%s\n",host);
      strcat(request+strlen(request),url+i+strcspn(url+i,"/"));
  }
  else      //如果proxyhost != NULL,有代理主机则直接将url拼接到请求方法后面
  {
      // printf("ProxyHost=%s\nProxyPort=%d\n",proxyhost,proxyport);
      strcat(request,url);
  }
  //再将协议版本拼接到请求方法和url后面，保存在request中
  if(http10==1)         
	  strcat(request," HTTP/1.0");
  else if (http10==2)
	  strcat(request," HTTP/1.1");
  strcat(request,"\r\n");           
 
  if(http10>0)
	  strcat(request,"User-Agent: WebBench "PROGRAM_VERSION"\r\n");    //继续拼接WebBench的版本信息
  
  if(proxyhost==NULL && http10>0)
  {
	  strcat(request,"Host: ");
	  strcat(request,host);
	  strcat(request,"\r\n");
  }
  if(force_reload && proxyhost!=NULL)
  {
	  strcat(request,"Pragma: no-cache\r\n");
  }
  if(http10>1)
	  strcat(request,"Connection: close\r\n");
  /* add empty line at end */
  if(http10>0) strcat(request,"\r\n"); 
  // printf("Req=%s\n",request);
}

/* vraci system rc error kod */
static int bench(void)       //派生子进程，父子进程管道通信最后输出计算结果
{
    int i,j,k;	
    pid_t pid=0;
    FILE *f;

    /* check avaibility of target server */
    i=Socket(proxyhost==NULL ? host:proxyhost,proxyport);   
    if(i<0)     //连接失败的情况
    { 
	    fprintf(stderr,"\nConnect to server failed. Aborting benchmark.\n");
        return 1;
    }
    close(i);
    /* create pipe */
    if(pipe(mypipe))
    {
	    perror("pipe failed.");
	      return 3;
    }

    /* not needed, since we have alarm() in childrens */
    /* wait 4 next system clock tick */
    /*
        cas=time(NULL);
        while(time(NULL)==cas)
           sched_yield();
    */

    /* fork childs */   //有多少个客户端就fork出多少个子进程
    for(i=0;i<clients;i++)
    {
	     pid=fork();
	     if(pid <= (pid_t) 0)
	     {
		      /* child process or error*/
	           sleep(1); /* make childs faster */
		       break;         //创建完子进程立马跳出该循环，否则该子进程还要继续fork
	     }
    }

    if( pid< (pid_t) 0)
    {
        fprintf(stderr,"problems forking worker no. %d\n",i);
	    perror("fork failed.");
	    return 3;
     }
	 //子进程响应一个实际的请求
     if(pid== (pid_t) 0)
     {
         /* I am a child */
         if(proxyhost==NULL)
               benchcore(host,proxyport,request);
         else
               benchcore(proxyhost,proxyport,request);

         /* write results to pipe */
	     f=fdopen(mypipe[1],"w");      //fdopen以w写的形式打开管道的写端
	     if(f==NULL)
	     {
		       perror("open pipe for writing failed.");
		       return 3;
	     }
	     /* fprintf(stderr,"Child - %d %d\n",speed,failed); */
	     fprintf(f,"%d %d %d\n",speed,failed,bytes);
	     fclose(f);
	     return 0;
      } 
     else     //父进程要做的事
     {
	     f=fdopen(mypipe[0],"r");
	     if(f==NULL) 
	     {
		      perror("open pipe for reading failed.");
		      return 3;
	      }
	      setvbuf(f,NULL,_IONBF,0);
		  speed = 0;     // 传输速度 
		  failed = 0;   // 失败请求数 
		  bytes = 0;   // 传输字节数

	    while(1)
	    {
		   pid=fscanf(f,"%d %d %d",&i,&j,&k);      //从管道中读取子进程写进来的数据
		   if(pid<2)
           {
               fprintf(stderr,"Some of our childrens died.\n");
                break;
           }
		  speed+=i;
		  failed+=j;
		  bytes+=k;
		  /* fprintf(stderr,"*Knock* %d %d read=%d\n",speed,failed,pid); */
		  if (--clients == 0)      //子进程是否读取完
			  break;
	  }
	  fclose(f);

      printf("\nSpeed=%d pages/min, %d bytes/sec.\nRequests: %d susceed, %d failed.\n",
		  (int)((speed+failed)/(benchtime/60.0f)),
		  (int)(bytes/(float)benchtime),
		  speed,
		  failed);
  }
  return i;
}

void benchcore(const char *host,const int port,const char *req) //每个子进程的实际发起请求函数
{
     int rlen;
     char buf[1500];
     int s,i;
     struct sigaction sa;

     /* setup alarm signal handler */
     sa.sa_handler = alarm_handler;       //设置信号发生时的处理函数
     sa.sa_flags=0;
    if(sigaction(SIGALRM,&sa,NULL))      //捕捉SIGALRM这个信号
        exit(3);
    alarm(benchtime);                   //设置闹钟

    rlen=strlen(req);
    nexttry:while(1)
    {
       if(timerexpired)   //timerexpired用来检测测试时长是否达到指定时长
       {
          if(failed>0)
          {
              /* fprintf(stderr,"Correcting failed by signal\n"); */
              failed--;
          }
         return;
       }
	   //创建socket并建立HTTP 请求 
      s=Socket(host,port);                          
      if(s<0)
	  {
		  failed++;
		  continue;
	  } 
      if(rlen!=write(s,req,rlen)) 
	  {
		  failed++;
		  close(s);
		  continue;
	  }
      if(http10==0)     //HTTP/0.9 的处理
	    if(shutdown(s,1)) 
		{
			failed++;
			close(s);
			continue;
		}
		
	   if(force==0)     //-f 选项是不读取服务器回复
       {
            /* read all available data from socket */
	      while(1)
	      {
              if(timerexpired)
				  break; 
	          i=read(s,buf,1500);
              /* fprintf(stderr,"%d\n",i); */
	          if(i<0) 
              { 
                 failed++;
                 close(s);
                 goto nexttry;
               }
	          else
		         if(i==0) break;
		         else
			       bytes+=i;
	        }
        }
        if(close(s)) 
		{
			failed++;
			continue;
		}
        speed++;
     }
}
