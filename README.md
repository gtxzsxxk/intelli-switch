# IntelliSw 智能WiFi开关

- [x] Concept Video Uploaded:https://www.bilibili.com/video/BV14A411u7TP
- [x] Web平台更新
- [x] STM32平台更新（可能用IOT终端 or MCU来代替这一说法）
- [x] 配网功能（请见于stm32目录）
- [x] ESP8266实现HTTP服务器（同上）

## Web平台

部署Web平台需要下载Font-Awesome，并添加至Web目录下的static目录。  
Web平台采用Django框架构建，建立了3个模型，分别是

- ServerInfo，储存服务器信息。

- SampleData，储存IOT终端上报的数据。IOT终端通过ESP8266连接到互联网，为了保持与服务器的长连接，访问与本Web平台共同部署在一起的数据上报接口（iot-uploader.py）。数据上报接口通过GET请求Web平台内定义的/command命令执行接口来更新数据。或者Web平台在View.py内调用socket访问数据上报接口，调用内部已经与IOT终端建立连接的socket发送设备更改或时间同步命令。

- DeviceControl，设备控制，储存自定义设备的状态。

## STM32平台

### Instruction to STM32 code

- This Code Is Running On STM32F401CCU6.You can buy it on Taobao.
- This is the basic STM32CUBEMX project files.Attention.I have deleted all innecessary driver files such as timer driver and usart driver.But I keep all the module drivers for the
followings.

  - ESP8266(The Wifi module)
  - BMP280(Temperature and Atmosphere Pressure Sensor)
  - DHT11(Temperature and Humidity Sensor) 
  - BF1750(Digital Lightness Sensor)

#### MCU数据传输过程

- MCU ↑↓(By USART)
- ESP8266 ↑↓ (By long TCP connection and USART)
- iot-uploader.py in SERVER ↑↓ (By long TCP connection and local TCP connection)
- Django Framework in SERVER ↑↓ (By local TCP connection)

> 最后更新于2月13日。这个项目进入生产模式已经15天，出现过几次掉线，但也自动重连成功。迫于升学压力，可能这是我最后一次维护这个项目，得益于多多思考了整个系统的架构，项目还算稳定。不打算再动这个项目太多，简单的小bug会修复。感谢自己的支持。
> zhy
