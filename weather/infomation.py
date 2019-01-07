from urllib.request import urlopen
from bs4 import BeautifulSoup
from urllib.error import HTTPError
import smtplib
from email.mime.text import MIMEText
from email.utils import formataddr
import time

"""
爬虫程序是一个需要后期投入很大维护�
tml代码，这就很有可能导致爬虫程序的失败，所以写爬虫程序要尽量做到未雨绸缪，让爬虫程序更健壮，这样你才能睡一个安稳的觉
这个程序很简单，大体分为两个部分，爬虫部分和邮件部分，代码还有很大的优化空间，比如一些常量可以拿到外面，这样代码看起来会更整洁，邮件内容是以html格式的形式发送的，这样你就可以改成自己喜欢的样式。
我不建议你频繁的去发邮件，邮箱可能会被封掉，信件退回，导致发送失败
发送电子邮件的邮箱要开启smtp客户端功能，邮箱——>设置——>开启SMTP服务，获取授权码（就是程序里�ther(url):
    try:
        html=urlopen(url).read()
    except HTTPError as e:
        return None
    try:
        weather_list=[]
        bs0bj=BeautifulSoup(html,"html.parser")
        time.sleep(5)
        weather=bs0bj.find("div",{"class":"condition-icon wx-weather-icon vector"}).next_siblings
        title=bs0bj.body.h1.get_text()
        weather_list.append(title)
        for next in weather:
            weather_list.append(next.get_text())
    except AttributeError as e:
        return None
    return weather_list

#获取未来5天的天气情况
def get_5weathers(url):
    try:
        html=urlopen(url).read()
    except HTTPError as e:
        return None
    try:
        weather5_list=[]
        bs0bj=BeautifulSoup(html,"html.parser")
        weathers=bs0bj.find("table",{"class":"twc-table"}).tbody
        for child in weathers.children:
            list1=[]
            for i in child.children:
                list1.append(i.get_text())
            list1.remove("")
            weather5_list.append(list1)
    except AttributeError as e:
        return None
    return weather5_list

#等到的数据形如一下数据格式
# weather=['北京, 中国', '3°', '晴朗', '体感���度 -1°', '高温 -- 低温 -7°紫外线指数 0（最大值10）']
# weathers=[['今天晚上\n12月 18日', '大部晴朗', '---7°', '0%', '北 15 公里 /小时 ', '40%'], ['星期二12月 19日', '晴朗', '5°-5°', '0%', '西南 21 公里 /小�� ', '32%'], ['星期三12月 20日', '晴朗', '7°-6°', '0%', '西北 22 公里 /小时 ', '33%'], ['星期四12月 21日', '晴朗', '6°-6°', '0%', '西南西 11 公里 /小时 ', '41%'], ['星期五12月 22日', '晴朗', '8°-6°', '0%', '北 16 ��里 /小时 ', '30%'], ['星期六12月 23日', '晴朗', '8°-3°', '0%', '西北西 14 公里 /小时 ', '29%']]


# #***********************发送电子邮件*******************************
#第三方SMTP服务器

def sendEmail():
    msg=MIMEText(mail_msg,"html","utf-8")
    msg["From"]=formataddr(["裤裆人",mail_user])
    msg["To"]=formataddr(["小裤裆人s",receive_address])
    msg["Subject"]="北京天气预报"
    try:
        smtp0bj=smtplib.SMTP_SSL(mail_host,465)
        smtp0bj.login(mail_user,mail_pass)
        smtp0bj.sendmail(mail_user,receive_address,msg.as_string())
        smtp0bj.quit()
        print("mail has been send to %s successfully."%receive_address)
    except smtplib.SMTPException as e:
        print(e)



if __name__=="__main__":
    weather=get_weather("http://www.weather.com")
    weathers=get_5weathers("https://weather.com/zh-CN/weather/5day/l/CHXX0008:1:CH")
    if weather==None:
        exit()
    if weathers==None:
        exit()
    mail_host="smtp.163.com"
    mail_user="150XXxX65@163.com"    #发件人邮箱账号
    mail_pass="jXXxX199XXxX"          #发件人邮箱密码
    receives=[
              "4352XXxX@qq.com",
              "3426XXxX@qq.com",
              "5437XXxX2@qq.com",
              "6353XXxX9@qq.com",
             ]    #收件人邮箱账号
    mail_msg="""
    <h1>裤裆人天气预报</h1>
    <p style="color:#99cc00;font-size:15px">%s-->目前天气状况：温度%s ,%s ,%s ,%s </p>
    <h3>以下是未来5天的天气情况</h3>
    <table width="800px" border="0" cellspacing="1" cellpadding="0" style="text-align:center">
    <thead style="background-color:#3399ff">
        <tr style="line-height:40px;">
            <th>白天</th>
            <th>说明</th>
            <th>高/低</th>
            <th>降雨概率</th>
            <th>风力</th>
            <th>湿度</th>
        </tr>
    </thead>
    <tbody>
        <tr style="background: #ccffcc">
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
        </tr>
        <tr style="background: #ccffcc">
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
        </tr>
        <tr style="background: #ccffcc">
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
        </tr>
        <tr style="background: #ccffcc">
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
        </tr>
        <tr style="background: #ccffcc">
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
        </tr>
    </tbody>
</table>
<p style="color:red;font-size:10px">注意：每天早上八点准时发送邮件，希望小伙伴们，多多关注天气情况，注意��暖！</p>

"""%(weather[0],weather[1],weather[2],weather[3],weather[4],weathers[0][0],weathers[0][1],weathers[0][2],weathers[0][3],weathers[0][4],weathers[0][5],weathers[1][0],weathers[1][1],weathers[1][2],weathers[1][3],weathers[1][4],weathers[1][5],weathers[2][0],weathers[2][1],weathers[2][2],weathers[2][3],weathers[2][4],weathers[2][5],weathers[3][0],weathers[3][1],weathers[3][2],weathers[3][3],weathers[3][4],weathers[3][5],weathers[4][0],weathers[4][1],weathers[4][2],weathers[4][3],weathers[4][4],weathers[4][5])
    for receive_address in receives:
        sendEmail()
        time.sleep(120)
    exit()
