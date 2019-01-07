ng: utf-8 -*-
import re
import requests
from bs4 import BeautifulSoup
import io
import sys
sys.stdout = io.TextIOWrapper(sys.stdout.buffer,encoding='utf-8')

r = requests.get('http://www.weather.com.cn/weather/101120301.shtml',timeout = 30)
r.raise_for_status()
r.encoding = 'utf-8'

rdata = re.findall(r'<h1>.*?</h1>',r.text)
rwea = re.findall(r'\"wea\">.*?</p>',r.text)
rtemp1 = re.findall(r'\/<i>.*?</i>',r.text)
rtemp2 = re.findall(r'<span>\d+\.?\d*</span>',r.text)


for i in range(6):
    data = rdata[i].split('>')[1].split('<')[0]
    wea = rwea[i].split('>')[1].split('<')[0]
    temp1 = rtemp1[i].split('>')[1].split('<')[0]
    temp2 = rtemp2[i].split('>')[1].split('<')[0]
    # temp2 = rtemp2[i].split('>')[1].split('<')[0]
    # tplt = "{0:^10}\t{1:{6}^10}\t{2:}\t{3:<}\t{4:}\t{5:}"
    tplt = "{0:^10}\t{1:{4}^10}\t{2:}\t{3:<}\t{4:}"
    print(tplt.format(data,wea,temp1,"~" + temp2 ,chr(12288)))
