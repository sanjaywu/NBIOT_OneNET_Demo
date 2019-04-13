#include "string.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"
#include "timer.h"
#include "usart1.h"
#include "usart3.h"
#include "customer_define.h"
#include "led.h"
#include "nbiot.h"
#include "sht20_i2c.h"
#include "sht20.h"
#include "mqtt.h"
#include "socket.h"
#include "wdg.h"

void hardware_init(void);
extern void nbiot_socket_send_data_to_server_process(void);

int main(void)
{	
	hardware_init();
	printf("hello nbiot...\r\n");
	
#if 1

	nbiot_power_on();
#ifdef __NBIOT_OPEN_SLEEP_MODE__
	nbiot_sleep_config(1);		/* 打开模组休眠模式 */
#else
	nbiot_sleep_config(0);		/* 关闭模组休眠模式 */
#endif
	nbiot_hardware_reset();

#ifdef __NBIOT_MQTT__
	nbiot_mqtt_app();
#endif

#ifdef __NBIOT_SOCKET__
		TIM3_Int_Init(9999, 7199);	/* 定时1s */
		nbiot_socket_app();
#endif


	vTaskStartScheduler();

#else

	while(1)
	{
		nbiot_socket_send_data_to_server_process();
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
	}

#endif
}

/**************************************************************
函数名称: hardware_init
函数功能: 硬件初始化
输入参数: 无
返回值  : 无
备注	: 无
**************************************************************/
void hardware_init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	usart1_init(115200);
	usart3_init(115200);
	
	TIM4_Int_Init(7999, 7199);	/* 定时800ms喂独立看门狗 */
	iwdg_init(4, 625);			/* 初始化独立看门狗溢出时间为1s */
	sht20_i2c_init();
	
	led_init();	
	nbiot_power_init();
}

























