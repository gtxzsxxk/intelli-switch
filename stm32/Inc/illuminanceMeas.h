#ifndef ILLU_HEAD
#define ILLU_HEAD
#include "main.h"
#define BH1750_ADDR 0xb8

extern double illuminance;
void bh1750_getIlluminance(void);
#endif
