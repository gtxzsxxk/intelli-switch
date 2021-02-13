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

//���ջ���
extern uint8_t ESP_RECEIVE_BUFFER[ESP_REV_BUFFER_SIZE];
//���ռ�����
extern uint16_t ESP_REV_CNT;

/*========================��ֲʱ��Ҫȷ���ĺ���=================================*/
void ESP8266_WatchDog(void);
//ESP����ATָ��
void ESP_SendAT(const char* dat);
//ESP��������
void ESP_SendData(char* dat);
//����������ɲü�Ϊֱ�ӷ���
void ESP_DEBUG_OUTPUT(char* output);
//���ж��ڵ��ã����յ����ַ����ĺ���
void ESP_GETS(uint8_t c);
//�ڹ涨ʱ���ڣ�ͬ�����չ涨���ַ���
void ESP_SYNC_RECEIVE(uint16_t timeout_ms);
//��ӡbuffer�������ڵ�����
void ESP_DEBUG_INFO(void);

/*=============================================================================*/

uint16_t ESP_LIB_CONTAINS(char* substr,char* source);
//�����λ
void ESP_CMD_RESET(void);
//����˳�͸��ģʽ
void ESP_CMD_ExitMUX(void);
//�������������
void ESP_CMD_MultipleConn(void);
//�������͸��
void ESP_CMD_TransConn(void);
uint8_t ESP_CMD_EnterTransMode(void);
//����STATIONģʽ������·����
void ESP_CMD_SET_STATION(void);
//����APģʽ����Ϊ�ȵ�
void ESP_CMD_SET_AP(void);
//����SLEEPģʽ���͹���
void ESP_CMD_SET_SLEEP(void);
//����AP���ú���ִ�г�ʱʱ��ȡ����ESP_WIFI_CONN_TIMEOUT_MS
uint8_t ESP_CMD_JoinAP(char* SSID,char* PASSWORD);
//��ʵ�֣���ȡ����IP��ַ��Only in station mode
void ESP_CMD_GETADDRS(void);
uint8_t ESP_OPEN_TCP_CLIENT(char* ip,char* port);
uint8_t ESP_OPEN_TCP_SERVER(char* port);
//ESP������ʼ��������Զ�̷�����
uint8_t ESP_ClientInit(void);
//ESP������ʼ������ΪԶ�̷�����
void ESP_ServerInit(void);
//ESP��Ϊ������ʱ���������ݵ���ѭ��
void ESP_SERVER_LOOP(void);

void ESP_SERVER_HTTP_HANDLE(uint8_t clientID,char* URL);
#endif
