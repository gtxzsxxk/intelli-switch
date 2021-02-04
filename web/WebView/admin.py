from django.contrib import admin
from .models import ServerInfo,SampleData,DeviceControl,UserApp
# Register your models here.

admin.site.register(ServerInfo)
admin.site.register(SampleData)
admin.site.register(DeviceControl)
admin.site.register(UserApp)