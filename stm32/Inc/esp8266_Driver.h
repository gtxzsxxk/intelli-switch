#ifndef ESP8266_h_
#define ESP8266_h_
#include "main.h"

#define ESP_WIFI_CONN_TIMEOUT_MS 12000
#define ESP_REV_BUFFER_SIZE 1024
#define ESP_SERVER_TRANSMIT_STANDBY_TIMEOUT_MS 6000

typedef struct
{
	uint16_t DataLength;
	uint8_t IsConnected;
	uint8_t Available;
} ESP_TCP_CLIENT;


extern char ESP_SSID[16];
extern char ESP_PASSWD[20];
extern char ESP_TCP_IPADDR[20];
extern char ESP_TCP_PORT[8];//65536

extern char ESP_LAN_MAC_ADDR[20];
extern char ESP_LAN_IP_ADDR[20];

//接收缓存
extern uint8_t ESP_RECEIVE_BUFFER[ESP_REV_BUFFER_SIZE];
//接收计数器
extern uint16_t ESP_REV_CNT;

/*========================移植时需要确定的函数=================================*/
void ESP8266_WatchDog(void);
//ESP发送AT指令
void ESP_SendAT(const char* dat);
//ESP发送数据
void ESP_SendData(char* dat);
//调试输出，可裁剪为直接返回
void ESP_DEBUG_OUTPUT(char* output);
//在中断内调用，接收单个字符串的函数
void ESP_GETS(uint8_t c);
//在规定时间内，同步接收规定的字符串
void ESP_SYNC_RECEIVE(uint16_t timeout_ms);
//打印buffer缓冲区内的数据
void ESP_DEBUG_INFO(void);

/*=============================================================================*/

uint16_t ESP_LIB_CONTAINS(char* substr,char* source);
//命令：复位
void ESP_CMD_RESET(void);
//命令：退出透传模式
void ESP_CMD_ExitMUX(void);
//命令：开启多连接
void ESP_CMD_MultipleConn(void);
//命令：开启透传
void ESP_CMD_TransConn(void);
uint8_t ESP_CMD_EnterTransMode(void);
//进入STATION模式，连接路由器
void ESP_CMD_SET_STATION(void);
//进入AP模式，作为热点
void ESP_CMD_SET_AP(void);
//设置SLEEP模式，低功耗
void ESP_CMD_SET_SLEEP(void);
//加入AP，该函数执行超时时间取决于ESP_WIFI_CONN_TIMEOUT_MS
uint8_t ESP_CMD_JoinAP(char* SSID,char* PASSWORD);
//待实现：获取本机IP地址。Only in station mode
void ESP_CMD_GETADDRS(void);
uint8_t ESP_OPEN_TCP_CLIENT(char* ip,char* port);
uint8_t ESP_OPEN_TCP_SERVER(char* port);
//ESP正常初始化，连接远程服务器
uint8_t ESP_ClientInit(void);
//ESP配网初始化，作为远程服务器
void ESP_ServerInit(void);
//ESP作为服务器时，处理数据的主循环
void ESP_SERVER_LOOP(void);

void ESP_SERVER_HTTP_HANDLE(uint8_t clientID,char* URL);
#endif
