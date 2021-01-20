#include "bmp280.h"

uint8_t bmp280_Read(uint8_t addr)
{
	uint8_t ret=255,isok=0;
	HAL_I2C_Master_Transmit(&hi2c1,BMP280_ADDR,&addr,1,10);
	isok=HAL_I2C_Master_Receive(&hi2c1,BMP280_ADDR,&ret,1,10);
	if(isok!=0)
		return HAL_I2C_GetError(&hi2c1);
	return ret;
}

void bmp280_Write(uint8_t addr,uint8_t dat)
{
	uint8_t mydat[2]={addr,dat};
	HAL_I2C_Master_Transmit(&hi2c1,BMP280_ADDR,mydat,2,10);
}
uint8_t chipid=0;


uint16_t dig_T1=0;
int16_t dig_T2=0;
int16_t dig_T3=0;

uint16_t dig_P1=0;
int16_t	dig_P2=0;
int16_t	dig_P3=0;
int16_t	dig_P4=0;
int16_t	dig_P5=0;
int16_t	dig_P6=0;
int16_t	dig_P7=0;
int16_t	dig_P8=0;
int16_t	dig_P9=0;

void bmp280_init(void)
{
	uint8_t lsb=0,msb=0;
	//复位BMP280
	bmp280_Write(BMP280_RESET_REG,BMP280_RESET_VALUE);
	HAL_Delay(1);
	//获取CHIP IP
	chipid=bmp280_Read(BMP280_CHIPID_REG);
	//设置数据精度
	bmp280_Write(BMP280_CTRLMEAS_REG,0xff);
	//设置采样时间
	bmp280_Write(BMP280_CONFIG_REG,0x20);
	
	//读取补偿值 dig_XX
	lsb=bmp280_Read(BMP280_DIG_T1_LSB_REG);
	msb=bmp280_Read(BMP280_DIG_T1_MSB_REG);
	dig_T1=msb<<8|lsb;
	
	lsb=bmp280_Read(BMP280_DIG_T2_LSB_REG);
	msb=bmp280_Read(BMP280_DIG_T2_MSB_REG);
	dig_T2=msb<<8|lsb;
	
	lsb=bmp280_Read(BMP280_DIG_T3_LSB_REG);
	msb=bmp280_Read(BMP280_DIG_T3_MSB_REG);
	dig_T3=msb<<8|lsb;
	
	lsb = bmp280_Read(BMP280_DIG_P1_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P1_MSB_REG);
	dig_P1 = msb << 8 | lsb;
	lsb = bmp280_Read(BMP280_DIG_P2_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P2_MSB_REG);
	dig_P2 = msb << 8 | lsb;
	lsb = bmp280_Read(BMP280_DIG_P3_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P3_MSB_REG);
	dig_P3 = msb << 8 | lsb;
	lsb = bmp280_Read(BMP280_DIG_P4_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P4_MSB_REG);
	dig_P4 = msb << 8 | lsb;
	lsb = bmp280_Read(BMP280_DIG_P5_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P5_MSB_REG);
	dig_P5 = msb << 8 | lsb;
	lsb = bmp280_Read(BMP280_DIG_P6_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P6_MSB_REG);
	dig_P6 = msb << 8 | lsb;
	lsb = bmp280_Read(BMP280_DIG_P7_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P7_MSB_REG);
	dig_P7 = msb << 8 | lsb;
	lsb = bmp280_Read(BMP280_DIG_P8_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P8_MSB_REG);
	dig_P8 = msb << 8 | lsb;
	lsb = bmp280_Read(BMP280_DIG_P9_LSB_REG);
	msb = bmp280_Read(BMP280_DIG_P9_MSB_REG);
	dig_P9 = msb << 8 | lsb;
	
}

double bmp_280_temperature=0.0;
double bmp_280_atmospressure=0.0;
double t_fine=0;

void bmp280_getTemperature()
{
	uint8_t buffer[14];
	uint8_t xlsb=0,lsb=0,msb=0;
	int32_t adc_T=0;
	double var1=0,var2=0;
	msb=bmp280_Read(BMP280_TEMPERATURE_MSB_REG);
	lsb=bmp280_Read(BMP280_TEMPERATURE_LSB_REG);
	xlsb=bmp280_Read(BMP280_TEMPERATURE_XLSB_REG);
	adc_T=(msb<<12)|(lsb<<4)|(xlsb>>4);
	var1 = (((double) adc_T) / 16384.0 - ((double) dig_T1) / 1024.0)
            *((double) dig_T2);
  var2 = ((((double) adc_T) / 131072.0 - ((double) dig_T1) / 8192.0)
            *(((double) adc_T) / 131072.0 - ((double) dig_T1) / 8192.0))
            *((double) dig_T3);
	t_fine=var1+var2;
	bmp_280_temperature=(var1+var2)/5120.0;
						//打印信息
	memset(buffer,0,sizeof(buffer));
	sprintf((char*)buffer,"TEMP:%.2f 'C ",bmp_280_temperature);
	l_print(buffer,4);
}

void bmp280_getAtmosPressure()
{
	uint8_t buffer[14];
	uint8_t msb=0,lsb=0,xlsb=0;
	int32_t adc_P=0;
	double var1, var2, pressure;
	
	msb=bmp280_Read(BMP280_PRESSURE_MSB_REG);
	lsb=bmp280_Read(BMP280_PRESSURE_LSB_REG);
	xlsb=bmp280_Read(BMP280_PRESSURE_XLSB_REG);
	adc_P=(msb<<12)|(lsb<<4)|(xlsb>>4);
 
	var1 = ((double) t_fine / 2.0) - 64000.0;
	var2 = var1 * var1 * ((double) dig_P6) / 32768.0;
	var2 = var2 + var1 * ((double) dig_P5) * 2.0;
	var2 = (var2 / 4.0) + (((double) dig_P4) * 65536.0);
	var1 = (((double) dig_P3) * var1 * var1 / 524288.0
					+ ((double) dig_P2) * var1) / 524288.0;
	var1 = (1.0 + var1 / 32768.0) * ((double) dig_P1);

	if (var1 == 0.0) {
		bmp_280_atmospressure=0; // avoid exception caused by division by zero
		return;
	}

	pressure = 1048576.0 - (double) adc_P;
	pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;
	var1 = ((double) dig_P9) * pressure * pressure / 2147483648.0;
	var2 = pressure * ((double) dig_P8) / 32768.0;
	pressure = pressure + (var1 + var2 + ((double) dig_P7)) / 16.0;
	bmp_280_atmospressure=pressure;
	//打印信息
	memset(buffer,0,sizeof(buffer));
	sprintf((char*)buffer,"QNH:%.2fhPa",bmp_280_atmospressure/100);
	l_print(buffer,5);
}
