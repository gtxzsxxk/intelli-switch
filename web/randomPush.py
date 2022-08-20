import requests
import random
import xxtea
import time
import base64

cnt=0
property_id=['1','2','3']
password="e10adc3949ba59abbe56e057f20f883e"

tosend_data={
    "value":"",
    "username":"admin",
    "signature":""
}

#tosend_data["signature"]=base64.b64encode(xxtea.encrypt(bytes(tosend_data["value"],'utf-8'),bytes(password[0:16],'utf-8'))).decode()

while True:
    if cnt==3:
        cnt=0
        time.sleep(15)
    if cnt==0:
        tosend_data["value"]=str(random.randrange(250,320)/10)
        # tosend_data["value"]=str('29.3')
    elif cnt==1:
        tosend_data["value"]=str(random.randrange(9900,10200)/10)
    elif cnt==2:
        tosend_data["value"]=str([random.randrange(18200,18300)/1000,random.randrange(109400,109800)/1000])
    src_bytes=xxtea.encrypt(tosend_data["value"],password[0:16])
    tosend_data["signature"]=base64.b64encode(src_bytes).decode()
    print(src_bytes)
    r=requests.post("http://localhost:8080/device_detail/"+property_id[cnt],tosend_data)
    print(r.text)
    cnt+=1
