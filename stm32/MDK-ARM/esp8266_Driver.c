#include "esp8266_Driver.h"
#include "esp_http_pages.h"

char ESP_SSID[16]="yourown";
char ESP_PASSWD[20]="yourown";
char ESP_TCP_IPADDR[20]="0.0.0.0";
char ESP_TCP_PORT[8]="0";//65536

char ESP_LAN_MAC_ADDR[20]="";
char ESP_LAN_IP_ADDR[20]="";

//���ջ���
uint8_t ESP_RECEIVE_BUFFER[ESP_REV_BUFFER_SIZE];
//���ռ�����
uint16_t ESP_REV_CNT=0;



//���ӷ������Ŀͻ���
ESP_TCP_CLIENT ESP_CLIENTS[5];

//����������ģʽ
uint16_t ESP_SERVER_MODE=0;
//����������Ԥ���ݿ�ʼ���ձ�־
uint16_t ESP_SERVER_IPD_REV_FLAG=0;
//���������ݿ�ʼ���ձ�־
uint16_t ESP_SERVER_REV_FLAG=0;
//��������Ҫ���յ����ݴ�С
uint16_t ESP_SERVER_REV_COUNT=0;
//���ڽ������ݵ�CLIENT
uint8_t ESP_CLIENT_REV_INDEX=0;

//���������ݱ�־
uint8_t ESP_SERVER_TRANSMIT_AVAILABLE=0;
//���ݴ�����ɱ�־2
uint8_t ESP_SERVER_TRANSMIT_OK=0;


//HTTP������
char* ESP_SERVER_HTTP_RESPONSE_HEAD="HTTP/1.1 200 OK\r\nServer: libmcuesp/intellisw\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n";



/*========================��ֲʱ��Ҫȷ���ĺ���=================================*/

//������ܿɲü�
//ESP8266���Ź������ӷ������յ���ʱ�������Ϣ120����û�и��£����ж�ESP8266崻�������λESP8266
//ESP8266���Ź����򴥷���Display_Time��ʱ����º���
void ESP8266_WatchDog(void)
{
	short deltaTime=0;
	deltaTime=(now_Time.Hours-server_Time.Hours)*60*60+(now_Time.Minutes-server_Time.Minutes)*60
		+(now_Time.Seconds-server_Time.Seconds);
	if(deltaTime>120)
	{
		ESP_CMD_RESET();
		ESP_ClientInit();
		HOME_SCREEN();
	}
}

//ESP����ATָ��
void ESP_SendAT(const char* dat)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)dat,strlen(dat),50000);
	HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,100);
}

//ESP��������
void ESP_SendData(char* dat)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)dat,strlen(dat),50000);
	//HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,100);
}

//ESP���Ͷ�������
void ESP_SendDataLen(char* dat,uint16_t len)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)dat,len,50000);
	//HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,100);
}

//����������ɲü�Ϊֱ�ӷ���
void ESP_DEBUG_OUTPUT(char* output)
{
	HAL_UART_Transmit(&huart2,(uint8_t*)output,strlen(output),1000);
}

//���ж��ڵ��ã����յ����ַ����ĺ���
void ESP_GETS(uint8_t c)
{
	ESP_RECEIVE_BUFFER[ESP_REV_CNT++]=c;
	if(ESP_REV_CNT==ESP_REV_BUFFER_SIZE)
		ESP_REV_CNT=0;
	if(ESP_SERVER_MODE)
	{
		if(c=='>')
		{
			ESP_SERVER_TRANSMIT_AVAILABLE=1;
		}
		if(ESP_SERVER_TRANSMIT_AVAILABLE)
		{
			if(c=='K')
			{
				ESP_SERVER_TRANSMIT_OK=1;
			}
		}
		if(!ESP_SERVER_REV_FLAG)
		{
			if(c=='\n'&&ESP_SERVER_REV_FLAG==0)
			{
				uint16_t result_close=ESP_LIB_CONTAINS(",CLOSED",(char*)ESP_RECEIVE_BUFFER);
				uint16_t result_conn=ESP_LIB_CONTAINS(",CONNECT",(char*)ESP_RECEIVE_BUFFER);
				if(result_close)
				{
					uint8_t clientID=ESP_RECEIVE_BUFFER[result_close-1-1]-'0';
					if(clientID<5)
					{
						ESP_CLIENTS[clientID].IsConnected=0;
					}
				}
				if(result_conn)
				{
					uint8_t clientID=ESP_RECEIVE_BUFFER[result_close-1-1]-'0';
					if(clientID<5)
					{
						ESP_CLIENTS[clientID].IsConnected=1;
					}
				}
				ESP_REV_CNT=0;
				memset(ESP_RECEIVE_BUFFER,0,sizeof(ESP_RECEIVE_BUFFER));
				return;
			}
			else if(c=='+')
			{
				ESP_SERVER_IPD_REV_FLAG=1;
				ESP_SERVER_REV_COUNT=ESP_REV_CNT-1;
				return;
			}
			else if(c==':'&&ESP_SERVER_IPD_REV_FLAG)
			{
				char torevdata[16];
				memset(torevdata,0,sizeof(torevdata));
				memcpy(torevdata,ESP_RECEIVE_BUFFER+ESP_SERVER_REV_COUNT,ESP_REV_CNT-ESP_SERVER_REV_COUNT+1);
				sscanf(torevdata,"+IPD,%hhu,%hu:",&ESP_CLIENT_REV_INDEX,&ESP_SERVER_REV_COUNT);
				ESP_SERVER_IPD_REV_FLAG=0;
				ESP_SERVER_REV_FLAG=1;
				ESP_REV_CNT=0;
				memset(ESP_RECEIVE_BUFFER,0,sizeof(ESP_RECEIVE_BUFFER));
				//for(uint16_t i=0;i<ESP_SERVER_REV_COUNT;i++)
				//	ESP_RECEIVE_BUFFER[i]=0;
				ESP_CLIENTS[ESP_CLIENT_REV_INDEX].DataLength=ESP_SERVER_REV_COUNT;
				return;
			}
		}
		
		if(ESP_SERVER_REV_FLAG&&ESP_SERVER_REV_COUNT>0)
		{
			ESP_SERVER_REV_COUNT--;
			if(ESP_SERVER_REV_COUNT==0)
			{
				//�������
				ESP_SERVER_REV_FLAG=0;
				ESP_CLIENTS[ESP_CLIENT_REV_INDEX].Available=1;
			}
		}
	}
}

//�ڹ涨ʱ���ڣ�ͬ�����չ涨���ַ���
void ESP_SYNC_RECEIVE(uint16_t timeout_ms)
{
	ESP_REV_CNT=0;
	memset(ESP_RECEIVE_BUFFER,0,sizeof(ESP_RECEIVE_BUFFER));
	ESP_REV_CNT=0;
	HAL_Delay(timeout_ms);
	//ESP_SET_INTERRUPT(timeout_ms);
	//while(ESP_REV_FLAG==0);
}

//��ӡbuffer�������ڵ�����
void ESP_DEBUG_INFO(void)
{
	ESP_DEBUG_OUTPUT("\r\n========\r\nGot from ESP:\r\n");
	ESP_DEBUG_OUTPUT((char*)ESP_RECEIVE_BUFFER);
	ESP_DEBUG_OUTPUT("\r\n=====End====\r\n");
}

uint16_t ESP_LIB_CONTAINS(char* substr,char* source)
{
	uint16_t sub_len=strlen(substr),source_len=strlen(source);
	uint16_t i=0,j=0;
	for(i=0;i<source_len+1-sub_len;i++)
	{
		uint16_t equals=0;
		for(j=0;j<sub_len;j++)
		{
			if(substr[j]==source[j+i])
				equals++;
		}
		if(equals==sub_len)
			return 1+i;
	}
	return 0;
}

/*=======================���õ������=================================*/

//�����λ
void ESP_CMD_RESET(void)
{
	HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port,ESP8266_RST_Pin,GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port,ESP8266_RST_Pin,GPIO_PIN_SET);
	HAL_Delay(8000);
}

//����˳�͸��ģʽ
void ESP_CMD_ExitMUX(void)
{
	ESP_SendAT("+++");
	HAL_Delay(100);
}

//�������������
void ESP_CMD_MultipleConn(void)
{
	ESP_SendAT("AT+CIPMUX=1");
	HAL_Delay(100);
	ESP_SendAT("AT+CIPMODE=0");
	HAL_Delay(100);
}

//�������͸��
void ESP_CMD_TransConn(void)
{
	ESP_SendAT("AT+CIPMUX=0");
	HAL_Delay(100);
	ESP_SendAT("AT+CIPMODE=1");
	HAL_Delay(100);
}

uint8_t ESP_CMD_EnterTransMode(void)
{
	ESP_SendAT("AT+CIPSEND");
	ESP_SYNC_RECEIVE(2500);
	ESP_DEBUG_INFO();
	if(ESP_LIB_CONTAINS("OK",(char*)ESP_RECEIVE_BUFFER)
		&&ESP_LIB_CONTAINS(">",(char*)ESP_RECEIVE_BUFFER))
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - �ɹ�����͸��ģʽ��\r\n");
		return 1;
	}
	else
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - �޷�����͸��ģʽ�����������Ϣ��\r\n");
		return 0;
	}
}

//����STATIONģʽ������·����
void ESP_CMD_SET_STATION(void)
{
	ESP_SendAT("AT+CWMODE=1");
	HAL_Delay(100);
}

//����APģʽ����Ϊ�ȵ�
void ESP_CMD_SET_AP(void)
{
	ESP_SendAT("AT+CWMODE=2");
	HAL_Delay(100);
}

//����SLEEPģʽ���͹���
void ESP_CMD_SET_SLEEP(void)
{
	ESP_SendAT("AT+SLEEP=2");
	HAL_Delay(100);
}

//����AP���ú���ִ�г�ʱʱ��ȡ����ESP_WIFI_CONN_TIMEOUT_MS
uint8_t ESP_CMD_JoinAP(char* SSID,char* PASSWORD)
{
	char AT_CWJAP[64];
	memset(AT_CWJAP,0,sizeof(AT_CWJAP));
	sprintf(AT_CWJAP,"AT+CWJAP=\"%s\",\"%s\"",SSID,PASSWORD);
	ESP_SendAT(AT_CWJAP);
	ESP_SYNC_RECEIVE(ESP_WIFI_CONN_TIMEOUT_MS);
	ESP_DEBUG_INFO();
	//��֤�Ӵ�WIFI GOT IP�Ƿ���ESP_RECEIVE_BUFFER��
	if(ESP_LIB_CONTAINS("WIFI GOT IP",(char*)ESP_RECEIVE_BUFFER))
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - ����ָ���ȵ�ɹ���\r\n");
		return 1;
	}
	else
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - �޷�����ָ���ȵ㡣���������Ϣ��\r\n");
		return 0;
	}
}

//����AP���ú���ִ�г�ʱʱ��ȡ����ESP_WIFI_CONN_TIMEOUT_MS
uint8_t ESP_CMD_EstablishAP(char* SSID,char* PASSWORD)
{
	char AT_CWJAP[64];
	memset(AT_CWJAP,0,sizeof(AT_CWJAP));
	sprintf(AT_CWJAP,"AT+CWSAP=\"%s\",\"%s\",11,3",SSID,PASSWORD);
	ESP_SendAT(AT_CWJAP);
	ESP_SYNC_RECEIVE(ESP_WIFI_CONN_TIMEOUT_MS);
	ESP_DEBUG_INFO();
	if(ESP_LIB_CONTAINS("OK",(char*)ESP_RECEIVE_BUFFER))
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - ����ָ���ȵ�ɹ���\r\n");
		return 1;
	}
	else
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - �޷�����ָ���ȵ㡣���������Ϣ��\r\n");
		return 0;
	}
}

//��ʵ�֣���ȡ����IP��ַ��Only in station mode
void ESP_CMD_GETADDRS(void)
{
	ESP_SendAT("AT+CIFSR");
	//ESP_SYNC_RECEIVE(300);
	//ESP_DEBUG_INFO();
	HAL_Delay(100);
}

void ESP_CMD_SendToClient_PerKB(uint8_t clientID,char* dat,uint16_t len)
{
	char cmd[20];uint16_t TIMEOUT_CNT=ESP_SERVER_TRANSMIT_STANDBY_TIMEOUT_MS;
	memset(cmd,0,sizeof(0));
	sprintf(cmd,"AT+CIPSEND=%hhu,%hu",clientID,len);
	ESP_SERVER_TRANSMIT_AVAILABLE=0;
	ESP_SendAT(cmd);
	while(ESP_SERVER_TRANSMIT_AVAILABLE==0&&TIMEOUT_CNT-->0)
	{
		HAL_Delay(1);
	}
	//ESP_SYNC_RECEIVE(1000);
	//ESP_DEBUG_INFO();
	if(ESP_SERVER_TRANSMIT_AVAILABLE)
	{
		ESP_SERVER_TRANSMIT_OK=0;
		TIMEOUT_CNT=ESP_SERVER_TRANSMIT_STANDBY_TIMEOUT_MS;
		ESP_SendDataLen(dat,len);
		while(ESP_SERVER_TRANSMIT_OK==0&&TIMEOUT_CNT-->0)
		{
			HAL_Delay(1);
		}
		if(ESP_SERVER_TRANSMIT_OK==1)
			ESP_DEBUG_OUTPUT("\r\nIntelliSw - ����ָ�����ݳɹ���\r\n");
		else
			ESP_DEBUG_OUTPUT("\r\nIntelliSw - ESP8266��������æ�����������Ϣ��\r\n");
		
		ESP_SERVER_TRANSMIT_AVAILABLE=0;
		return;
	}
	else
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - ESP8266�ܾ��������ݡ����������Ϣ��\r\n");
		return;
	}
}

void ESP_CMD_SendToClient(uint8_t CLIENT_ID,char* dat)
{
	uint16_t len=strlen(dat);
	if(len<=2048)
	{
		ESP_CMD_SendToClient_PerKB(CLIENT_ID,dat,len);
	}
	else
	{
		for(uint16_t i=0;i<len;i+=2048)
		{
			if(i+2048<len)
			{
				//0 1 2 3 4 5 6 7
				//i=6 len=8
				ESP_CMD_SendToClient_PerKB(CLIENT_ID,dat+i,2048);
			}
			else
			{
				ESP_CMD_SendToClient_PerKB(CLIENT_ID,dat+i,len-i);
			}
		}
	}
}

uint8_t ESP_OPEN_TCP_CLIENT(char* ip,char* port)
{
	char AT_CIPSTART[80];
	memset(AT_CIPSTART,0,sizeof(AT_CIPSTART));
	sprintf(AT_CIPSTART,"AT+CIPSTART=\"TCP\",\"%s\",%s",ip,port);
	ESP_SendAT(AT_CIPSTART);
	ESP_SYNC_RECEIVE(5000);
	ESP_DEBUG_INFO();
	if(ESP_LIB_CONTAINS("CONNECT",(char*)ESP_RECEIVE_BUFFER)&&
		ESP_LIB_CONTAINS("OK",(char*)ESP_RECEIVE_BUFFER))
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - ����ָ���������ɹ���\r\n");
		return 1;
	}
	else
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - �޷�����ָ�������������������Ϣ��\r\n");
		return 0;
	}
}

uint8_t ESP_OPEN_TCP_SERVER(char* port)
{
	char AT_CIPSERVER[30];
	memset(AT_CIPSERVER,0,sizeof(AT_CIPSERVER));
	sprintf(AT_CIPSERVER,"AT+CIPSERVER=1,%s",port);
	ESP_SendAT(AT_CIPSERVER);
	ESP_SYNC_RECEIVE(1000);
	ESP_DEBUG_INFO();
	if(ESP_LIB_CONTAINS("OK",(char*)ESP_RECEIVE_BUFFER))
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - ����ָ���������ɹ���\r\n");
		return 1;
	}
	else
	{
		ESP_DEBUG_OUTPUT("\r\nIntelliSw - �޷�����ָ�������������������Ϣ��\r\n");
		return 0;
	}
}

/*uint8_t ESP_ESP826601_CONF(void)
{
	HAL_Delay(100);
	ESP_CMD_ExitMUX();
	ESP_CMD_RESET();
	ESP_CMD_SET_AP();
	if(ESP_CMD_EstablishAP("intellisw","12345678"))
	{
		
	}
	return 0;
}*/

//ESP������ʼ��������Զ�̷�����
uint8_t ESP_ClientInit(void)
{
	HAL_Delay(100);
	f_print("Network Initialize");
	ESP_CMD_ExitMUX();
	ESP_CMD_RESET();
	
	
	ESP_CMD_SET_STATION();
	ESP_CMD_SET_SLEEP();
	if(ESP_CMD_JoinAP(ESP_SSID,ESP_PASSWD))
	{
		f_print("Wi-Fi Connected");
		ESP_CMD_TransConn();
		if(ESP_OPEN_TCP_CLIENT(ESP_TCP_IPADDR,ESP_TCP_PORT))
		{
			f_print("TCP Server Connected");
			return ESP_CMD_EnterTransMode();
			//1 success
			//0 fail to enter trans mode
		}
		else{
			f_print("Fail to Connect to Server");
			HAL_Delay(20000);
			return 3;//fail to conn server
		}
	}
	else
	{
		f_print("Fail to Connect to Wi-Fi");
		HAL_Delay(20000);
		return 2; //fail to join ap
	}
}

void ESP_ServerInit(void)
{
	HAL_Delay(100);
	f_print("Network Configuration");
	ESP_CMD_ExitMUX();
	ESP_CMD_RESET();
	ESP_CMD_MultipleConn();
	ESP_CMD_SET_AP();
	ESP_CMD_SET_SLEEP();
	if(ESP_CMD_EstablishAP("intellisw","123456789"))
	{
		HAL_Delay(1);
		f_print("AP has established.Please connect to AP:");
		HAL_Delay(1);
		f_print("SSID=intellisw,PassWD=123456789");
		HAL_Delay(1);
		if(ESP_OPEN_TCP_SERVER("80"))
		{
			f_print("HTTP SERVER IS RUNNING");
			HAL_Delay(1);
			ESP_SERVER_LOOP();
		}
	}
}

void ESP_SERVER_LOOP(void)
{
	memset(ESP_CLIENTS,0,sizeof(ESP_CLIENTS));
	ESP_SERVER_MODE=1;
	ESP_REV_CNT=0;
	while(1)
	{
		//��ѯ��Ѱ�ҿ��õĿͻ���
		for(uint8_t i=0;i<5;i++)
		{
			if(ESP_CLIENTS[i].Available)
			{
				ESP_CLIENTS[i].Available=0;
				/*char IPD[2]={i+'0',0};
				ESP_DEBUG_OUTPUT("\r\nRev From Client ");
				ESP_DEBUG_OUTPUT(IPD);
				ESP_DEBUG_OUTPUT(":\r\n");
				ESP_DEBUG_OUTPUT((char*)ESP_RECEIVE_BUFFER);
				ESP_DEBUG_OUTPUT("\r\n");*/
				//GET /,HTTP/
				int16_t url_start=ESP_LIB_CONTAINS("GET /",(char*)ESP_RECEIVE_BUFFER)-1+4
					,url_end=ESP_LIB_CONTAINS("HTTP/",(char*)ESP_RECEIVE_BUFFER)-1-1-1;
				if(url_start>=0&&url_end>=0)
				{
					char URL[128];
					memset(URL,0,sizeof(URL));
					memcpy(URL,ESP_RECEIVE_BUFFER+url_start,url_end-url_start+1);
					ESP_SERVER_HTTP_HANDLE(i,URL);
				}
				ESP_REV_CNT=0;
				memset(ESP_RECEIVE_BUFFER,0,sizeof(ESP_RECEIVE_BUFFER));
			}
		}
	}
}

void ESP_SERVER_HTTP_ERROR404(uint8_t clientID,char* URL)
{
	char error[200];
	char content_length[32];
	uint16_t content_len=0;
	memset(error,0,sizeof(error));
	memset(content_length,0,sizeof(content_length));
	sprintf(error,"The URL ('%s') you request for is not found.Please Check the Route Table configured in the Firmware.",URL);
	content_len=strlen(ESP_SERVER_HTTP_ERRPAGE_404_1)+strlen(error)+strlen(ESP_SERVER_HTTP_ERRPAGE_404_2);
	sprintf(content_length,"Content-Length: %hu\r\n\r\n",content_len);
	//ESP_SERVER_HTTP_RESPONSE_HEAD
	ESP_CMD_SendToClient(clientID,ESP_SERVER_HTTP_RESPONSE_HEAD);
	ESP_CMD_SendToClient(clientID,content_length);
	ESP_CMD_SendToClient(clientID,ESP_SERVER_HTTP_ERRPAGE_404_1);
	ESP_CMD_SendToClient(clientID,error);
	ESP_CMD_SendToClient(clientID,ESP_SERVER_HTTP_ERRPAGE_404_2);
}

void ESP_SERVER_HTTP_STATICPAGE(uint8_t clientID,char* page)
{
	char content_length[32];
	uint16_t content_len=0;
	memset(content_length,0,sizeof(content_length));
	
	content_len=strlen(page);
	sprintf(content_length,"Content-Length: %hu\r\n\r\n",content_len);
	//ESP_SERVER_HTTP_RESPONSE_HEAD
	ESP_CMD_SendToClient(clientID,ESP_SERVER_HTTP_RESPONSE_HEAD);
	ESP_CMD_SendToClient(clientID,content_length);
	ESP_CMD_SendToClient(clientID,page);
}

void ESP_SERVER_HTTP_ROUTE_view(uint8_t clientID,char* URL)
{
	char temp[80];
	memset(temp,0,sizeof(temp));
	FlashRead();
	if(ESP_SSID[0]>='0'&&ESP_SSID[0]<='z')
	{
		sprintf(temp,"%s %s %s %s",ESP_SSID,ESP_PASSWD,ESP_TCP_IPADDR,ESP_TCP_PORT);
	}
	else
	{
		sprintf(temp,"Invalid Invalid 0.0.0.0 0");
	}
	ESP_SERVER_HTTP_STATICPAGE(clientID,temp);
}

void ESP_SERVER_HTTP_ROUTE_update(uint8_t clientID,char* URL)
{
	/*     /update/801/bao32801/192.168.0.150/10010/            */
	uint8_t slash_cnt=0;uint8_t splitCNT=0;
	uint8_t url_len=strlen(URL);
	const char split[2]="/";
	for(uint8_t i=0;i<url_len;i++)
	{
		if(URL[i]==split[0]) //��Լ�ռ�
			slash_cnt++;
	}
	if(slash_cnt==6)
	{
		//sscanf(URL,"/update/%s/%s/%s/%s/",ESP_SSID,ESP_PASSWD,ESP_TCP_IPADDR,ESP_TCP_PORT);
		char temp_url[128];
		memset(temp_url,0,sizeof(temp_url));
		memcpy(temp_url,URL,url_len);
		char* token;
		token=strtok(temp_url,split);
		while(token!=NULL)
		{
			token=strtok(NULL,split);
			if(splitCNT==0)
			{
				memset(ESP_SSID,0,sizeof(ESP_SSID));
				memcpy(ESP_SSID,token,strlen(token));
			}
			else if(splitCNT==1)
			{
				memset(ESP_PASSWD,0,sizeof(ESP_PASSWD));
				memcpy(ESP_PASSWD,token,strlen(token));
			}
			else if(splitCNT==2)
			{
				memset(ESP_TCP_IPADDR,0,sizeof(ESP_TCP_IPADDR));
				memcpy(ESP_TCP_IPADDR,token,strlen(token));
			}
			else if(splitCNT==3)
			{
				memset(ESP_TCP_PORT,0,sizeof(ESP_TCP_PORT));
				memcpy(ESP_TCP_PORT,token,strlen(token));
			}
			splitCNT++;
		}
		FlashWrite();
		ESP_SERVER_HTTP_STATICPAGE(clientID,"success");
	}
	else
		ESP_SERVER_HTTP_STATICPAGE(clientID,"failed to upload.check your input.");
}

void ESP_SERVER_HTTP_HANDLE(uint8_t clientID,char* URL)
{
	uint8_t URL_LEN=strlen(URL);
	//����·�ɣ�����Ϊ�˽�ʡ��Դ��ֱ���ڳ�����ʵ�֡�
	if(URL[0]=='/'&&URL_LEN==1)
	{
		//return index.html
		ESP_SERVER_HTTP_STATICPAGE(clientID,ESP_SERVER_HTTP_INDEXPAGE);
	}
	else if(ESP_LIB_CONTAINS("/update/",URL))
	{
		ESP_SERVER_HTTP_ROUTE_update(clientID,URL);
	}
	else if(ESP_LIB_CONTAINS("/view/",URL))
	{
		ESP_SERVER_HTTP_ROUTE_view(clientID,URL);
	}
	else
		ESP_SERVER_HTTP_ERROR404(clientID,URL);
}
