from django.db import models
from django.utils import timezone
import datetime
import random

# Create your models here.

class ServerInfo(models.Model):
    servername=models.CharField('服务器名称',max_length=128)
    remoteIp=models.CharField('远程IP',max_length=128)
    localIp=models.CharField('内网IP',max_length=128)
    startup=models.DateTimeField('上线时间',default=datetime.datetime.now())
    totalDataPack=models.IntegerField('总数据包',default=0)
    #目前仅支持字符模式
    guisize_x=models.IntegerField('MCU板上显示屏可供显示的X字符范围',default=14)
    guisize_y=models.IntegerField('MCU板上显示屏可供显示的Y字符范围',default=6)
    word_x=models.IntegerField('一个字符的X大小',default=6)
    word_y=models.IntegerField('一个字符的Y大小',default=8)
    chars_pixel=models.TextField('JSON格式：字符的字模',default="{}")

    def __str__(self):
        return self.servername

class SampleData(models.Model):
    report_time=models.DateTimeField('报告时间',default=datetime.datetime.now()) #22 bytes
    temperature=models.FloatField('温度',default=0.0) #4 bytes
    humidity=models.FloatField('相对湿度',default=0.0) 
    atmospressure=models.FloatField('室内气压',default=0.0)
    illuminance=models.FloatField('光照强度',default=0.0)
    switchStatus=models.TextField('开关状态',max_length=512) #512 bytes but never used

    def __str__(self):
        return self.report_time.strftime('%Y-%m-%d %H:%M')

class DeviceControl(models.Model):

    STATUS_CHOICES=(
        ('SWITCH','SWITCH'),
        ('PROGRESS','PROGRESS')
    )
    name=models.CharField('设备名称',max_length=32)
    deviceId=models.IntegerField('设备号')
    rwType=models.CharField('设备读写类型',max_length=64,choices=STATUS_CHOICES)
    deviceStatus=models.CharField('设备状态',max_length=64)
    deviceInformation=models.TextField('设备信息',max_length=256)

    def __str__(self):
        return self.name

class UserApp(models.Model):

    def GenerateKey():
        return ''.join(random.sample('zyxwvutsrqponmlkjihgfedcba',16))

    name=models.CharField('APP名称',max_length=32)
    appid=models.IntegerField('APP ID',auto_created=True)
    description=models.CharField('解释',max_length=128)
    active=models.BooleanField('激活',default=False)
    app_key=models.CharField('APP访问KEY',max_length=16,default=GenerateKey())
    downstream_port=models.IntegerField('下行数据传输端口',default=random.randint(10011,10190))

    def __str__(self):
        return self.name
    

    