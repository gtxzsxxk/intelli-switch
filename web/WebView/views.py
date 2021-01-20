from django.db.models.lookups import IsNull
from django.shortcuts import render,HttpResponse
from WebView.models import ServerInfo,SampleData,DeviceControl
from django.utils import timezone
import datetime
import time
from hashlib import md5
import socket

# Create your views here.

availablePin=['SET YOUR OWN']

def get_client_ip(request):
    x_forwarded_for = request.META.get('HTTP_X_FORWARDED_FOR')
    if x_forwarded_for:
        ip = x_forwarded_for.split(',')[-1].strip()
    else:
        ip = request.META.get('REMOTE_ADDR')
    return ip

def IphIDE(ip):
    source=ip.split('.')
    print(source)
    return "%s.***.%s.***"%(source[0],source[2])

def getDeviceMD5(request,pin):
    ua=str(request.META["HTTP_USER_AGENT"]+get_client_ip(request))
    md5code=md5(ua.encode('UTF-8')).hexdigest()+md5((ua+pin).encode('UTF-8')).hexdigest()
    return md5code

def HasAuth(request):
    for i in availablePin:
        if 'device_auth' not in request.COOKIES:
            return False
        userfp=request.COOKIES['device_auth']
        validfp=getDeviceMD5(request,i)
        print("用户指纹：",userfp)
        print("验证指纹：",validfp)
        if userfp==validfp:
            deviceauth="当前设备已获权"
            return True
    return False

def getDeviceStatus(deviceStatus,value):
    if deviceStatus=='SWITCH':
        if value=='1':
            return 'On'
        elif value=='0':
            return 'Off'
        else:
            raise Exception("未知的SWITCH状态",value)
    else:
        raise Exception("未知的状态",value)

def IndexPage(request):
    context={}
    serverinfoObj=ServerInfo.objects.all()[0]
    dnow=datetime.datetime.now()
    sonline=serverinfoObj.startup
    #deltaTime=int(str(dnow-sonline).split('.')[0].split(':')[0])*24*60\
    #    +int(str(dnow-sonline).split('.')[0].split(':')[1])*60\
    #    +int(str(dnow-sonline).split('.')[0].split(':')[2])/60
    deltaTime="%.1f"%((dnow-sonline).total_seconds()/60)
    
    
    alldata=SampleData.objects.all()
    lastestData:SampleData=None
    if alldata.__len__()==0:
        lastestData=SampleData()
    else:
        lastestData=alldata[alldata.__len__()-1]
        
    updateDeltaTime=str(int((dnow-lastestData.report_time).total_seconds()))

    deviceauth="当前设备权限已冻结"
    context['authstatus']=deviceauth
    if HasAuth(request):
        deviceauth="当前设备已获权，用户指纹："+request.COOKIES['device_auth'][0:8]
        context['authstatus']="已获得设备访问权限"
    #lastestData.report_time.strftime("%Y-%m-%d %H:%M:%S")
    context['serverinfo']=("%s；服务器（%s）；公网IP：%s；内网IP：%s；正常运行时间：%s 分钟；总数据包：%d；数据更新时间：%s秒前"%\
        (deviceauth,serverinfoObj.servername,IphIDE(get_client_ip(request)),serverinfoObj.localIp,deltaTime,alldata.__len__(),\
            updateDeltaTime)).replace("；","<br />")
        
    context['temperature']="%.1f"%lastestData.temperature
    context['humidity']=int(lastestData.humidity)
    context['atmospressure']=int(lastestData.atmospressure)
    context['illuminance']="%.1f"%lastestData.illuminance
    context['serverStatus']='ACTIVE(RUNNING)'
    disabled=''
    if (dnow-lastestData.report_time).total_seconds()>120:
        context['serverStatus']='NORESPONSE'
        context['towarn']='Warn'
        disabled='disabled'

    dev_template="<div class=\"controlBox\"><span style=\"float:left;line-height:30px;font-weight:700\" onclick='Talert(\"+++Description+++\")'><i class=\"fa fa-cogs fa-1x\" style=\"margin-right:5px\"></i> +++name+++ </span><input type=\"button\" id=\"devbtn+++deviceid+++\" class=\"btn btn-danger\" style=\"float:right;width:53px\" onclick=\"device_ctl(+++deviceid+++,'switch')\" value=\"+++status+++\" "+disabled+"></div>"
    #Devices
    totalTemplate=""
    for i in DeviceControl.objects.all():
        template=dev_template.replace("+++Description+++",i.deviceInformation)\
            .replace("+++name+++",i.name).replace("+++deviceid+++",str(i.deviceId))\
                .replace("+++status+++",getDeviceStatus(i.rwType,i.deviceStatus))
        if getDeviceStatus(i.rwType,i.deviceStatus)=='Off':
            template=template.replace("btn-danger","btn-primary")
        totalTemplate=totalTemplate+template
    
    context['devicecontrols']=totalTemplate

    return render(request,"index.html",context)

"""
在线用户终端
除了登录指令以外，其他指令都需要验证cookie
指令格式：
login [pincode]
logout

"""
def commandExec(request):
    ucmd=request.GET.get('c')
    print("WEB终端执行命令："+ucmd)
    print("UA信息：",request.META['HTTP_USER_AGENT'])
    paras=ucmd.split(' ')
    response=HttpResponse("$webClient/ServerTerminal: "+ucmd)
    if paras[0]=='login' and paras.__len__()==2:
        if 'device_auth' not in request.COOKIES and paras[1] in availablePin:
            fingerprint=getDeviceMD5(request,paras[1])
            print("用户获权，指纹：",fingerprint)
            response.set_cookie('device_auth',fingerprint,max_age=15*86400)
            response.write("\r\n获权成功。")
        else:
            response.write("\r\n获权失败。您可能已经获合法或非法权，或者输入的Pin码不在Pin集内。")
    elif paras[0]=='logout' and paras.__len__()==1:
        if 'device_auth' not in request.COOKIES:
            response.write("\r\n您的权限已被冻结。")
        else:
            response.delete_cookie('device_auth')
            response.write("\r\n您的权限已冻结。")
    elif paras[0]=='devedit' and paras.__len__()==3:
        if 'device_auth' not in request.COOKIES:
            response.write("\r\nerrs:您没有权限访问这个设备。")
        else:
            deviceid=int(paras[1])
            device=DeviceControl.objects.get(deviceId=deviceid)
            if device.rwType!='SWITCH':
                response.write("\r\nerrs:Cannot switch a none-switched device.")
            else:
                if device.deviceStatus=='1':
                    device.deviceStatus='0'
                elif device.deviceStatus=='0':
                    device.deviceStatus='1'
                else:
                    raise Exception("未知的SWITCH设备状态",device.deviceStatus)
                device.save()
                try:
                    mysock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
                    mysock.connect(('127.0.0.1',10010))
                    tosend='$ch%d:%s#'%(deviceid,device.deviceStatus)
                    mysock.send(tosend.encode('utf-8'))
                    mysock.close()
                except Exception as e:
                    print("无法上传至本地数据上报接口。",e)
                response.write("\r\n"+getDeviceStatus(device.rwType,device.deviceStatus))
    elif paras[0]=='iotrpt':
        #物联网终端上报数据
        #iot:物联网 rpt:report             大气压力   光照强度    （可选）设备信息，由设备返回的json序列，‘’为设备号
        #格式：iotrpt temperature humidity pressure illuminance {'1':1,'2':1,'3':1,'4':1}
        """
        HTTP请求格式：
        GET /command/?c=iotrpt+13.0+79+1013+212 HTTP/1.1
        User-Agent: app/iotserver
        """
        if paras.__len__()==5:
            smp=SampleData(temperature=float(paras[1]),humidity=float(paras[2]),\
                atmospressure=float(paras[3]),illuminance=float(paras[4]),\
                    report_time=datetime.datetime.now())
            smp.save()
            response.write('\r\nsaved.'+datetime.datetime.now().strftime("DT%a %H:%M:%S"))
        elif paras.__len__()==6:
            pass
        else:
            response.write('errs:can\'t resolve command.')
    else:
        response.write("\r\n无法识别输入的指令。")
    return response