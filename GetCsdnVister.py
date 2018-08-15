#-*- coding=utf-8 -*-  
  
import urllib2  
import codecs  
import re  
  
csdn_url = "https://blog.csdn.net/m0_37925202"  
blog_url = ["https://blog.csdn.net/m0_37925202/article/details/81154053",  
       "https://blog.csdn.net/m0_37925202/article/details/81084050",  
       "https://blog.csdn.net/m0_37925202/article/details/81051510",  
       "https://blog.csdn.net/m0_37925202/article/details/81036585",  
       "https://blog.csdn.net/m0_37925202/article/details/80796010",  
       "https://blog.csdn.net/m0_37925202/article/details/80390844",  
       "https://blog.csdn.net/m0_37925202/article/details/71374838",  
       "https://blog.csdn.net/m0_37925202/article/details/81155213",  
       "https://blog.csdn.net/m0_37925202/article/details/81088824",  
       "https://blog.csdn.net/m0_37925202/article/details/81056793",  
       "https://blog.csdn.net/m0_37925202/article/details/81056431",  
       "https://blog.csdn.net/m0_37925202/article/details/81052888",  
       "https://blog.csdn.net/m0_37925202/article/details/81045391",  
       "https://blog.csdn.net/m0_37925202/article/details/81043198",
       "https://blog.csdn.net/m0_37925202/article/details/80907713", 
       "https://blog.csdn.net/m0_37925202/article/details/80899012", 
	   "https://blog.csdn.net/m0_37925202/article/details/80903455",   
	   "https://blog.csdn.net/m0_37925202/article/details/80818561",   
	   "https://blog.csdn.net/m0_37925202/article/details/80816684",   
	   "https://blog.csdn.net/m0_37925202/article/details/80778636",   
	   "https://blog.csdn.net/m0_37925202/article/details/80777376",   
	   "https://blog.csdn.net/m0_37925202/article/details/80771751",   
	   "https://blog.csdn.net/m0_37925202/article/details/80721702",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80668170",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80602319",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80525913",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80495850",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80463210",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80461714",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80450286",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80423297",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80302722",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80302452",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80298998",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80298511",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80293925",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80293925",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80286946",   #
	   "https://blog.csdn.net/m0_37925202/article/details/80258974",   #
       ]  
  
class CSDN(object):  
    def __init__(self):  
        self.csdn_url = csdn_url  
        self.blog_url = blog_url  
        self.headers =  {'User-Agent':'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.6) Gecko/20091201 Firefox/3.5.6',}    
  
    def openCsdn(self):  
        req = urllib2.Request(self.csdn_url, headers = self.headers)  
        response = urllib2.urlopen(req)  
        thePage = response.read()  
        response.close()  
        pattern = "ทรฮสฃบ<span>(\d+)ดฮ</span>"  
        number = ''.join(re.findall(pattern, thePage))  
        print number  
  
    def openBlog(self):  
        for i in range(len(self.blog_url)):  
            req = urllib2.Request(self.blog_url[i], headers = self.headers)  
            response = urllib2.urlopen(req)  
            response.close()  
  
      
for i in range(500):  
    print i  
    csdn = CSDN()  
    csdn.openCsdn()  
    csdn.openBlog()  
    csdn.openCsdn()  
    
    
#python刷取CSDN访问量方法。
#亲测有效，不过稳定性有点问题，一天刷20分钟，可以获得10000左右的访问量。可以同时开多个进程同时执行，这样效率就会比较高。
