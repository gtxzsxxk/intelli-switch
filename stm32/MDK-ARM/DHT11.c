#include "DHT11.h"

uint8_t dht11_bits[8]={1,2,4,8,16,32,64,128};
uint8_t dht11_temperature=0;
uint8_t dht11_humidity=255;

void DHT11_GPIO_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pin : DHT11_PORT_Pin */
  GPIO_InitStruct.Pin = DHT11_PORT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_PORT_GPIO_Port, &GPIO_InitStruct);
}

void DHT11_GPIO_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pin : DHT11_PORT_Pin */
  GPIO_InitStruct.Pin = DHT11_PORT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	//GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_PORT_GPIO_Port, &GPIO_InitStruct);
}
/*
void DHT11_Init(void)
{
	uint8_t recv_cnt=0,read_data=0,cnt=0;
	DHT11_GPIO_OUT();
	HAL_GPIO_WritePin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin,1);
	HAL_Delay(5);
	HAL_GPIO_WritePin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin,0);
	HAL_Delay(20);
	HAL_GPIO_WritePin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin,1);
	delay_us(35);
	DHT11_GPIO_IN();
	delay_us(20);
	if(HAL_GPIO_ReadPin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin)==0)
	{
		while(HAL_GPIO_ReadPin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin)==0);
		while(HAL_GPIO_ReadPin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin)==1);
		for(cnt=0;cnt<8;cnt++)
		{
			recv_cnt=0;
			while(HAL_GPIO_ReadPin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin)==0);
			while(HAL_GPIO_ReadPin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin)==1)
			{
				delay_us(1);
				recv_cnt++;
			}
			if(recv_cnt>10)
				read_data+=1*dht11_bits[cnt];
		}
		
		dht11_humidity=read_data;
		HAL_Delay(4);
		DHT11_GPIO_OUT();
		HAL_GPIO_WritePin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin,1);
	}
	//这传感器太NT，为了节目效果进行一下逻辑变换
	if(dht11_humidity>128)
	{
		dht11_humidity-=128;
	}
	if(dht11_humidity>50&&dht11_humidity<90)
	{
		//打印信息
		uint8_t buffer[14];
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer,"HUMI:%d",dht11_humidity);
		l_print(buffer,7);
	}
	
}
*/
//自己能力有限，写的实在调不通，感谢https://blog.csdn.net/qq153471503/article/details/103009000
#define DHT11_IO_OUT DHT11_GPIO_OUT
#define DHT11_IO_IN DHT11_GPIO_IN
#define DHT11_DQ_IN HAL_GPIO_ReadPin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin)
//复位DHT11
void DHT11_Rst(void)
{
    DHT11_IO_OUT(); 	//SET OUTPUT
    HAL_GPIO_WritePin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin,0); 	//拉低DQ
    HAL_Delay(20);    	//拉低至少18ms,(DHT22 500us)
    HAL_GPIO_WritePin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin,1); 	//DQ=1
    delay_us(30);     	//主机拉高20~40us
}

//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
uint8_t DHT11_Check(void)
{
    uint8_t retry=0;
    DHT11_IO_IN();//SET INPUT
    while (DHT11_DQ_IN&&retry<100)//DHT11会拉低40~80us
    {
        retry++;
        delay_us(1);
    };
    if(retry>=100)return 1;
    else retry=0;
    while (!DHT11_DQ_IN&&retry<100)//DHT11拉低后会再次拉高40~80us
    {
        retry++;
        delay_us(1);
    };
    if(retry>=100)return 1;
    return 0;
}

//从DHT11读取一个位
//返回值：1/0
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry=0;
    while(DHT11_DQ_IN&&retry<100)//等待变为低电平
    {
        retry++;
        delay_us(1);
    }
    retry=0;
    while(!DHT11_DQ_IN&&retry<100)//等待变高电平
    {
        retry++;
        delay_us(1);
    }
    delay_us(40);//等待40us
    if(DHT11_DQ_IN)return 1;
    else return 0;
}

//从DHT11读取一个字节
//返回值：读到的数据
uint8_t DHT11_Read_Byte(void)
{
    uint8_t i,dat;
    dat=0;
    for (i=0; i<8; i++)
    {
        dat<<=1;
        dat|=DHT11_Read_Bit();
    }
    return dat;
}

//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi)
{
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    if(DHT11_Check()==0)
    {
        for(i=0; i<5; i++) //读取40位数据
        {
            buf[i]=DHT11_Read_Byte();
        }
        if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
        {
            *humi=buf[0];
            *temp=buf[2];
        }
    } else return 1;
    return 0;
}

uint8_t DHT11_Read_Data_Float(float *temp,float *humi)
{
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    if(DHT11_Check()==0)
    {
        for(i=0; i<5; i++) //读取40位数据
        {
            buf[i]=DHT11_Read_Byte();
        }
        if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
        {
			*humi=((buf[0] << 8) + buf[1]) / 10.0;
			*temp=((buf[2] << 8) + buf[3]) / 10.0;
        }
    } else return 1;
    return 0;
}

//初始化DHT11的IO口 DQ 同时检测DHT11的存在
//返回1:不存在
//返回0:存在
uint8_t DHT11_Init(void)
{
    uint8_t ret = 1;
    DHT11_Rst();  //复位DHT11
    ret = DHT11_Check();
    return ret;
}

uint8_t DHT11_HUMIDITY=0;

void DHT11_Output(void)
{
	uint8_t a;
	DHT11_Init();
	DHT11_Read_Data(&a,&DHT11_HUMIDITY);
	//打印信息
	uint8_t buffer[14];
	memset(buffer,0,sizeof(buffer));
	sprintf((char*)buffer,"HUMI:%hhu per",DHT11_HUMIDITY);
	l_print(buffer,7);
}
