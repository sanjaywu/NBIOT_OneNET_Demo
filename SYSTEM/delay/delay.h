#ifndef __DELAY_H__
#define __DELAY_H__

#include "sys.h"  

void delay_init(void);
void delay_us(u32 nus);
void delay_ms(u32 nms);
void os_delay_ms(u32 nms);

#endif





























