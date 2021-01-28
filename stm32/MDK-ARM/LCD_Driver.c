#include "LCD_Driver.h"

//向SPI总线传输一个8位数据
void  SPI_WriteData(uint8_t Data)
{
	HAL_SPI_Transmit(&hspi1,&Data,1,10);
}

//向液晶屏写一个8位指令
void Lcd_WriteIndex(uint8_t Index)
{
   //SPI 写命令时序开始
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_RESET);
	 SPI_WriteData(Index);
   HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_SET);
}

//向液晶屏写一个8位数据
void Lcd_WriteData(uint8_t Data)
{
   HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_SET);
	 SPI_WriteData(Data);
   HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_SET);
}
//向液晶屏写一个16位数据
void LCD_WriteData_16Bit(uint16_t Data)
{
   HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_RESET);
   HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_SET);
	 SPI_WriteData(Data>>8); 	//写入高8位数据
	 SPI_WriteData(Data); 			//写入低8位数据
   HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_SET); 
}

void Lcd_WriteReg(uint8_t Index,uint8_t Data)
{
	Lcd_WriteIndex(Index);
  Lcd_WriteData(Data);
}

void Lcd_Reset(void)
{
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port,LCD_RST_Pin,GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port,LCD_RST_Pin,GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port,LCD_RST_Pin,GPIO_PIN_SET);
	HAL_Delay(120);
}

//LCD Init For 1.44Inch LCD Panel with ST7735R.
void Lcd_Init(void)
{	
	Lcd_Reset(); //Reset before LCD Init.

	//LCD Init For 1.44Inch LCD Panel with ST7735R.
	Lcd_WriteIndex(0x11);//Sleep exit 
	HAL_Delay (120);
		
	//ST7735R Frame Rate
	Lcd_WriteIndex(0xB1); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 

	Lcd_WriteIndex(0xB2); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 

	Lcd_WriteIndex(0xB3); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 
	
	Lcd_WriteIndex(0xB4); //Column inversion 
	Lcd_WriteData(0x07); 
	
	//ST7735R Power Sequence
	Lcd_WriteIndex(0xC0); 
	Lcd_WriteData(0xA2); 
	Lcd_WriteData(0x02); 
	Lcd_WriteData(0x84); 
	Lcd_WriteIndex(0xC1); 
	Lcd_WriteData(0xC5); 

	Lcd_WriteIndex(0xC2); 
	Lcd_WriteData(0x0A); 
	Lcd_WriteData(0x00); 

	Lcd_WriteIndex(0xC3); 
	Lcd_WriteData(0x8A); 
	Lcd_WriteData(0x2A); 
	Lcd_WriteIndex(0xC4); 
	Lcd_WriteData(0x8A); 
	Lcd_WriteData(0xEE); 
	
	Lcd_WriteIndex(0xC5); //VCOM 
	Lcd_WriteData(0x0E); 
	
	Lcd_WriteIndex(0x36); //MX, MY, RGB mode 
	Lcd_WriteData(0xC0); 
	
	//ST7735R Gamma Sequence
	Lcd_WriteIndex(0xe0); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x1a); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x18); 
	Lcd_WriteData(0x2f); 
	Lcd_WriteData(0x28); 
	Lcd_WriteData(0x20); 
	Lcd_WriteData(0x22); 
	Lcd_WriteData(0x1f); 
	Lcd_WriteData(0x1b); 
	Lcd_WriteData(0x23); 
	Lcd_WriteData(0x37); 
	Lcd_WriteData(0x00); 	
	Lcd_WriteData(0x07); 
	Lcd_WriteData(0x02); 
	Lcd_WriteData(0x10); 

	Lcd_WriteIndex(0xe1); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x1b); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x17); 
	Lcd_WriteData(0x33); 
	Lcd_WriteData(0x2c); 
	Lcd_WriteData(0x29); 
	Lcd_WriteData(0x2e); 
	Lcd_WriteData(0x30); 
	Lcd_WriteData(0x30); 
	Lcd_WriteData(0x39); 
	Lcd_WriteData(0x3f); 
	Lcd_WriteData(0x00); 
	Lcd_WriteData(0x07); 
	Lcd_WriteData(0x03); 
	Lcd_WriteData(0x10);  
	
	Lcd_WriteIndex(0x2a);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x7f);

	Lcd_WriteIndex(0x2b);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x9f);
	
	Lcd_WriteIndex(0xF0); //Enable test command  
	Lcd_WriteData(0x01); 
	Lcd_WriteIndex(0xF6); //Disable ram power save mode 
	Lcd_WriteData(0x00); 
	
	Lcd_WriteIndex(0x3A); //65k mode 
	Lcd_WriteData(0x05); 
	
	
	Lcd_WriteIndex(0x29);//Display on	 
	Lcd_Clear(BLACK);
	ForeFont_Color=GREEN;
	Backgound_Color=BLACK;
}


/*************************************************
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void Lcd_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end)
{		
	Lcd_WriteIndex(0x2a);
	Lcd_WriteData(0x00);
	Lcd_WriteData(x_start);//Lcd_WriteData(x_start+2);
	Lcd_WriteData(0x00);
	Lcd_WriteData(x_end+2);

	Lcd_WriteIndex(0x2b);
	Lcd_WriteData(0x00);
	Lcd_WriteData(y_start+0);
	Lcd_WriteData(0x00);
	Lcd_WriteData(y_end+1);
	
	Lcd_WriteIndex(0x2c);

}

/*************************************************
函数名：LCD_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void Lcd_SetXY(uint16_t x,uint16_t y)
{
  	Lcd_SetRegion(x,y,x,y);
}

	
/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
void Gui_DrawPoint(uint16_t x,uint16_t y,uint16_t Data)
{
	Lcd_SetRegion(x,y,x+1,y+1);
	LCD_WriteData_16Bit(Data);

}    

/*****************************************
 函数功能：读TFT某一点的颜色                          
 出口参数：color  点颜色值                                 
******************************************/
unsigned int Lcd_ReadPoint(uint16_t x,uint16_t y)
{
  unsigned int Data;
  Lcd_SetXY(x,y);

  //Lcd_ReadData();//丢掉无用字节
  //Data=Lcd_ReadData();
  Lcd_WriteData(Data);
  return Data;
}
/*************************************************
函数名：Lcd_Clear
功能：全屏清屏函数
入口参数：填充颜色COLOR
返回值：无
*************************************************/
void Lcd_Clear(uint16_t Color)               
{	
   unsigned int i,m;
   Lcd_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);
   Lcd_WriteIndex(0x2C);
   for(i=0;i<X_MAX_PIXEL;i++)
    for(m=0;m<Y_MAX_PIXEL;m++)
    {	
	  	LCD_WriteData_16Bit(Color);
    }   
}

