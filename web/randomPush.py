import requests
import random
import xxtea
import time
import base64
import json

case=1
if case==0:
    device_name="S22+"
    password="e10adc3949ba59abbe56e057f20f883e"

    tosend_data={
        "value":"",
        "username":"admin",
        "signature":""
    }

    #tosend_data["signature"]=base64.b64encode(xxtea.encrypt(bytes(tosend_data["value"],'utf-8'),bytes(password[0:16],'utf-8'))).decode()

    while True:
            
        temp=str(random.randrange(250,320)/10)
        pressure=str(random.randrange(9900,10200)/10)
        gps_track=str([random.randrange(18200,18300)/1000,random.randrange(109400,109800)/1000])
        value_obj=[temp,pressure,gps_track]
        value_str=json.dumps(value_obj)
        tosend_data["value"]=value_str
        src_bytes=xxtea.encrypt(tosend_data["value"],password[0:16])
        tosend_data["signature"]=base64.b64encode(src_bytes).decode()
        print(src_bytes)
        r=requests.post("http://localhost:8080/device_details/"+device_name,tosend_data)
        print(r.text)
        time.sleep(15)
elif case==1:
    device_name="Test-01"
    password="e10adc3949ba59abbe56e057f20f883e"

    tosend_data={
        "value":"",
        "username":"admin",
        "signature":""
    }

    #tosend_data["signature"]=base64.b64encode(xxtea.encrypt(bytes(tosend_data["value"],'utf-8'),bytes(password[0:16],'utf-8'))).decode()

    while True:
            
        temp=str(random.randrange(250,320)/10)
        pressure=str(random.randrange(9900,10200)/10)
        velocity=str(random.randrange(100,500)/100)
        throttle=str(random.randint(0,100))
        brake=str(random.randint(0,10)>5)
        value_obj=[temp,pressure,velocity,throttle,brake]
        value_str=json.dumps(value_obj)
        tosend_data["value"]=value_str
        src_bytes=xxtea.encrypt(tosend_data["value"],password[0:16])
        tosend_data["signature"]=base64.b64encode(src_bytes).decode()
        print(src_bytes)
        r=requests.post("http://localhost:8080/device_details/"+device_name,tosend_data)
        print(r.text)
        time.sleep(15)
