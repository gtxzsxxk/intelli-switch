from django.contrib import admin
from .models import ServerInfo,SampleData,DeviceControl
# Register your models here.

admin.site.register(ServerInfo)
admin.site.register(SampleData)
admin.site.register(DeviceControl)