#include "main.h"
#include "LCD_Driver.h"


extern const unsigned char asc16[1521];
extern uint16_t ForeFont_Color;
extern uint16_t Backgound_Color;
extern uint8_t line_ctl;
void GUI_PrintOnScreen(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s);
void g_print(uint8_t* dat);
void l_print(uint8_t* dat,uint8_t line);



//ÊıÂë¹Ü×ÖÌå  %  ¡æ
//const unsigned char sz32[];
