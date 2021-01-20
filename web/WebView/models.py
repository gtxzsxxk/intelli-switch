from django.db import models
from django.utils import timezone
import datetime
import time

# Create your models here.

class ServerInfo(models.Model):
    servername=models.CharField('服务器名称',max_length=128)
    remoteIp=models.CharField('远程IP',max_length=128)
    localIp=models.CharField('内网IP',max_length=128)
    startup=models.DateTimeField('上线时间',default=datetime.datetime.now())
    totalDataPack=models.IntegerField('总数据包',default=0)

    def __str__(self):
        return self.servername

class SampleData(models.Model):
    report_time=models.DateTimeField('报告时间',default=datetime.datetime.now())
    temperature=models.FloatField('温度',default=0.0)
    humidity=models.FloatField('相对湿度',default=0.0)
    atmospressure=models.FloatField('室内气压',default=0.0)
    illuminance=models.FloatField('光照强度',default=0.0)
    switchStatus=models.TextField('开关状态',max_length=512)

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

    