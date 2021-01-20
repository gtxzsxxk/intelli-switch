#include "illuminanceMeas.h"

double illuminance=0;

void bh1750_getIlluminance(void)
{
	uint8_t askcmd=0x23;
	uint8_t response[]={0,0};
	uint8_t buffer[14];
	HAL_I2C_Master_Transmit(&hi2c1,BH1750_ADDR,&askcmd,1,10);
	HAL_I2C_Master_Receive(&hi2c1,BH1750_ADDR,response,2,10);
	illuminance=((double)(response[0]<<8|response[1]))/1.2;
	//´òÓ¡ÐÅÏ¢
	memset(buffer,0,sizeof(buffer));
	sprintf((char*)buffer,"ILLU:%.1f lx",illuminance);
	l_print(buffer,6);
}

