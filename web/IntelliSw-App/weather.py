import requests
import json
import time
#guiStr:str="   APP MODE   Weather ReportLoc:[10]TD:[11]TM:[11][14]"
guiStr:str="   APP MODE   Weather ReportLoc:%sTD:%sTM:%s%s"

WAIT_TIME=30

url='http://www.weather.com.cn/data/sk/101310201.html'
Location='Jiyang Dt.'
City=''
TempNow=''
TempLow=''
TempHigh=''

def FillStr(_str:str,len):
    output=_str
    for i in range(len-_str.__len__()):
        output=output+" "
    return output

def GetWeather():
    global TempNow,TempLow,TempHigh
    response=requests.get("https://tianqiapi.com/api?version=v1&appid=74697218&appsecret=1f0cjpzU&city=%E4%B8%89%E4%BA%9A")
    jsonStr:str=response.text
    jsonObj=json.loads(jsonStr)
    weatherDict=jsonObj['data'][1]
    TempNow=jsonObj['data'][0]['tem'].replace("℃","'C")
    TempToday=weatherDict['tem'].replace("℃","'C")
    TempLow=weatherDict['tem2'].replace("℃","'C")
    TempHigh=weatherDict['tem1'].replace("℃","'C")
    msgstr=guiStr%(FillStr(Location,10),FillStr(TempNow,11),FillStr(TempToday,11),FillStr(("%s->%s"%(TempLow,TempHigh)),14))
    return msgstr

while(True):
    MSGDATA=GetWeather()
    print(MSGDATA)
    if MSGDATA.__len__()!=84:
        raise Exception("字符串长度不一致")
    param={"apikey":"pmcsixbdkrefuwtl","scrdata":MSGDATA,"appid":"3","c":"appscrupd"}
    response=requests.get('http://127.0.0.1:8000/command/',params=param)
    print(response.text)
    time.sleep(WAIT_TIME)
    exitparam={"apikey":"pmcsixbdkrefuwtl","appid":"3","c":"appexit"}
    response=requests.get('http://127.0.0.1:8000/command/',params=exitparam)
    print(response.text)
    time.sleep(WAIT_TIME)



