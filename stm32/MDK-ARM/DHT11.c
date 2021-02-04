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
	//�⴫����̫NT��Ϊ�˽�ĿЧ������һ���߼��任
	if(dht11_humidity>128)
	{
		dht11_humidity-=128;
	}
	if(dht11_humidity>50&&dht11_humidity<90)
	{
		//��ӡ��Ϣ
		uint8_t buffer[14];
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer,"HUMI:%d",dht11_humidity);
		l_print(buffer,7);
	}
	
}
*/
//�Լ��������ޣ�д��ʵ�ڵ���ͨ����лhttps://blog.csdn.net/qq153471503/article/details/103009000
#define DHT11_IO_OUT DHT11_GPIO_OUT
#define DHT11_IO_IN DHT11_GPIO_IN
#define DHT11_DQ_IN HAL_GPIO_ReadPin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin)
//��λDHT11
void DHT11_Rst(void)
{
    DHT11_IO_OUT(); 	//SET OUTPUT
    HAL_GPIO_WritePin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin,GPIO_PIN_RESET); 	//����DQ
    HAL_Delay(20);    	//��������18ms,(DHT22 500us)
    HAL_GPIO_WritePin(DHT11_PORT_GPIO_Port,DHT11_PORT_Pin,GPIO_PIN_SET); 	//DQ=1
    delay_us(30);     	//��������20~40us
}

//�ȴ�DHT11�Ļ�Ӧ
//����1:δ��⵽DHT11�Ĵ���
//����0:����
uint8_t DHT11_Check(void)
{
    uint8_t retry=0;
    DHT11_IO_IN();//SET INPUT
    while (DHT11_DQ_IN&&retry<100)//DHT11������40~80us
    {
        retry++;
        delay_us(1);
    };
    if(retry>=100)return 1;
    else retry=0;
    while (!DHT11_DQ_IN&&retry<100)//DHT11���ͺ���ٴ�����40~80us
    {
        retry++;
        delay_us(1);
    };
    if(retry>=100)return 1;
    return 0;
}

//��DHT11��ȡһ��λ
//����ֵ��1/0
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry=0;
    while(DHT11_DQ_IN&&retry<100)//�ȴ���Ϊ�͵�ƽ
    {
        retry++;
        delay_us(1);
    }
    retry=0;
    while(!DHT11_DQ_IN&&retry<100)//�ȴ���ߵ�ƽ
    {
        retry++;
        delay_us(1);
    }
    delay_us(40);//�ȴ�40us
    if(DHT11_DQ_IN)return 1;
    else return 0;
}

//��DHT11��ȡһ���ֽ�
//����ֵ������������
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

//��DHT11��ȡһ������
//temp:�¶�ֵ(��Χ:0~50��)
//humi:ʪ��ֵ(��Χ:20%~90%)
//����ֵ��0,����;1,��ȡʧ��
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi)
{
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    if(DHT11_Check()==0)
    {
        for(i=0; i<5; i++) //��ȡ40λ����
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
        for(i=0; i<5; i++) //��ȡ40λ����
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

//��ʼ��DHT11��IO�� DQ ͬʱ���DHT11�Ĵ���
//����1:������
//����0:����
uint8_t DHT11_Init(void)
{
    uint8_t ret = 1;
    DHT11_Rst();  //��λDHT11
    ret = DHT11_Check();
    return ret;
}

uint8_t DHT11_HUMIDITY=0;

void DHT11_Output(void)
{
	uint8_t a;
	DHT11_Init();
	DHT11_Read_Data(&a,&DHT11_HUMIDITY);
	//��ӡ��Ϣ
	char buffer[14];
	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"HUMI:%hhu per",DHT11_HUMIDITY);
	if(!AppMode)
		l_print(buffer,5,Left);
}
