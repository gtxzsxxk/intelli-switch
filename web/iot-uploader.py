import socket
import threading
import datetime
import time
import traceback
import smtplib
from email.mime.text import MIMEText
from email.header import Header

time_out_setting:int=120
listen_port:int=10010

def SendMail():
    # 第三方 SMTP 服务
    #Thanks to runoob
    mail_host="smtp.qq.com"  #设置服务器
    mail_user="2524395907@qq.com"    #用户名
    mail_pass="ktzlliytwsgmdiaj"   #口令 
    
    
    sender = '2524395907@qq.com'
    receivers = ['2524395907@qq.com']  # 接收邮件，可设置为你的QQ邮箱或者其他邮箱

    htmlfile=open('mail.html','r',encoding='utf-8')
    htmldata=htmlfile.read()
    htmlfile.close()
    htmldata=htmldata.replace('{{waittime}}',str(time_out_setting)).replace('{{time}}',\
        time.strftime("%d日 %H时%M分%S秒", time.localtime()))

    mail_msg = htmldata
    message = MIMEText(mail_msg, 'html', 'utf-8')
    message['From'] = Header("服务器数据上报接口", 'utf-8')
    message['To'] =  Header("不在位警告", 'utf-8')
    
    subject = 'IOT终端不在位警告'
    message['Subject'] = Header(subject, 'utf-8')
    try:
        smtpObj = smtplib.SMTP_SSL(host=mail_host) 
        smtpObj.connect(mail_host, 465)    # 25 为 SMTP 端口号
        smtpObj.login(mail_user,mail_pass)  
        smtpObj.sendmail(sender, receivers, message.as_string())
        print("邮件发送成功")
    except smtplib.SMTPException:
        print("Error: 无法发送邮件")


def UploadToHttp(iotdata):
    upload_sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    upload_sock.connect(('127.0.0.1',8000))
    tosend="GET /command/?c=iotrpt+%s+%s+%s+%s HTTP/1.1\r\nUser-Agent: app/iot-uploader\r\nConnection: close\r\nHost: 127.0.0.1:8000\r\n\r\n"%\
        (iotdata[0],iotdata[1],iotdata[2],iotdata[3])

    print(tosend)
    
    upload_sock.send(tosend.encode('utf-8'))
    time.sleep(0.2)
    tosendback=upload_sock.recv(1024)
    upload_sock.close()
    dat_str=tosendback.decode('utf-8')
    len=dat_str.split('\r\n').__len__()
    totellback="$%s#"%dat_str.split('\r\n')[len-1].replace('saved.','')
    print("服务器返回：",totellback)
    return totellback.encode('utf-8')

iot_socket:socket.socket=None

class Listening(threading.Thread):
    mysocket:socket.socket=None
    remote_addr=None
    def __init__(self,sock,addr):
        threading.Thread.__init__(self)
        self.mysocket=sock
        self.remote_addr=addr

    def run(self):
        global iot_socket
        while(True):
            try:
                self.mysocket.settimeout(time_out_setting)
                recv_data=self.mysocket.recv(1024)
                iotdata=recv_data.decode('utf-8')
                if iotdata[0]=='$':
                    if iotdata[1]=='c' and iotdata[2]=='h':
                        print("%s 接收到本地服务器传来数据:%s"%(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),iotdata))
                        if iot_socket!=None:
                            print(iotdata)
                            iot_socket.send(recv_data)
                            print("已将本地服务器要求转发IOT")
                        else:
                            print("尚未获取到IOT_SOCKET")
                        self.mysocket.close()
                        return
                    else:
                        #$25.14:47:101443.33:83.3#
                        print("%s 接收到IOT传来数据:%s"%(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),iotdata))
                        iot_socket=self.mysocket
                        iotdata=iotdata.replace('$','').replace('#','')
                        datas=iotdata.split(':')
                        datas[2]="%.2f"%(float(datas[2])/100)
                        print("温度：%s，湿度：%s，大气压：%s，光照强度：%s"%(datas[0],datas[1],datas[2],datas[3]))
                        #上报服务器
                        totellback=UploadToHttp(datas)
                        self.mysocket.send(totellback)
            except Exception as e:
                if str(e)=='timed out':
                    print("IOT终端超时，未及时上传数据，将断开连接并发送邮件上报。")
                    self.mysocket.close()
                    SendMail()
                    iot_socket=None
                    return
                else:
                    print(e)
                    traceback.print_exc()
                    self.mysocket.close()
                    if iot_socket is self.mysocket:
                        iot_socket=None
                        print("释放IOT_SOCKET")
                    return


serversocket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
addr=('0.0.0.0',listen_port)
serversocket.bind(addr)
serversocket.listen(5)
print("服务器上报程序开始监听端口%d"%listen_port)

while(True):
    client_socket,client_addr=serversocket.accept()
    listener=Listening(client_socket,client_addr)
    listener.start()
    print(client_addr,"已接入服务。")


