#ifndef __NBIOT_H__
#define __NBIOT_H__

#include "sys.h"

#define NBIOT_WAKE_UP_HIGH	GPIO_SetBits(GPIOC, GPIO_Pin_6)
#define NBIOT_WAKE_UP_LOW	GPIO_ResetBits(GPIOC, GPIO_Pin_6)

void nbiot_power_init(void);
void nbiot_power_on(void);
void nbiot_hardware_reset(void);
void nbiot_wake_up(void);

void nbiot_at_response(u8 mode);
void nbiot_clear_recv(void);
u8 nbiot_send_cmd(char *cmd, char *ack, u16 waittime);
u8 nbiot_sleep_config(u8 mode);




#endif










