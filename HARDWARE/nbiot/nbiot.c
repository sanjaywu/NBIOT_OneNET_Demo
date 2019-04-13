#include "string.h"
#include "stdarg.h"	 	 
#include "stdio.h"	
#include "stdlib.h"
#include "nbiot.h"
#include "usart1.h"
#include "usart3.h"
#include "delay.h"

/**************************************************************
函数名称: nbiot_power_init
函数功能: nbiot电源初始化
输入参数: 无
返回值  : 无
备注	: GPIO_Pin_4：nbiot POWER_ON引脚
		  GPIO_Pin_5：nbiot RESET引脚
		  GPIO_Pin_6：nbiot WAKE_UP引脚
**************************************************************/
void nbiot_power_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);/* 使能GPIOC时钟 */
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 /* 推挽输出 */
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6);
}

/**************************************************************
函数名称: nbiot_power_on
函数功能: nbiot 开机
输入参数: 无
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_power_on(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);/* 拉低power引脚开机 */
	delay_ms(500);	
	GPIO_SetBits(GPIOC, GPIO_Pin_4);	
}

/**************************************************************
函数名称: nbiot_hardware_reset
函数功能: nbiot硬件复位
输入参数: 无
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_hardware_reset(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_5);	/* 拉低RESET引脚开始复位 */
	delay_ms(500);		
	GPIO_SetBits(GPIOC, GPIO_Pin_5);
}

/**************************************************************
函数名称: nbiot_wake_up
函数功能: nbiot 外部唤醒
输入参数: 无
返回值  : 无
备注	: 下降沿触发唤醒
**************************************************************/
void nbiot_wake_up(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_6);
	GPIO_SetBits(GPIOC, GPIO_Pin_6);
}

/**************************************************************
函数名称: nbiot_sleep_config
函数功能: 模组睡眠开关
输入参数: mode：0，关闭睡眠模式；1：开启睡眠模式
返回值  : 0：成功
		  1：失败
备注	: 配置完之后，模组reboot后无效
**************************************************************/
u8 nbiot_sleep_config(u8 mode)
{
	u8 res;

	if(mode)
	{
		res = nbiot_send_cmd("AT+SM=UNLOCK_FOREVER", "OK", 300);
		printf("AT+SM=UNLOCK_FOREVER\r\n");
	}
	else
	{
		res = nbiot_send_cmd("AT+SM=LOCK_FOREVER", "OK", 300);
		printf("AT+SM=LOCK_FOREVER\r\n");
	}
	
	return res;
}


/**************************************************************
函数名称: nbiot_at_response
函数功能: 将收到的AT指令应答数据返回给电脑串口
输入参数: mode：0,不清零USART3_RX_STA;
				1,清零USART3_RX_STA;
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_at_response(u8 mode)
{
	if(USART3_RX_STA & 0X8000) /* 接收到一次数据了 */
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]	= 0;	/* 添加结束符 */
		printf("%s\r\n", USART3_RX_BUF);			/* 发送到串口 */
		if(mode)
		{
			USART3_RX_STA = 0;
		}
	} 
}

/**************************************************************
函数名称: nbiot_clear_recv
函数功能: 清空接收缓冲区
输入参数: 无
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_clear_recv(void)
{
	u16 i = 0;
	
	for(i = 0; i < USART3_MAX_RECV_LEN; i++)
	{	
		USART3_RX_BUF[i] = 0;	
	}
	USART3_RX_STA = 0;
}

/**************************************************************
函数名称: nbiot_check_cmd
函数功能: nbiot 发送命令后,检测接收到的应答
输入参数: str:期待的应答结果
返回值  : NULL:没有得到期待的应答结果
		  其他,期待应答结果的位置(str的位置)
备注	: 无
**************************************************************/
char* nbiot_check_cmd(char *str)
{
	char *strx = 0;
	if(USART3_RX_STA & 0X8000)					/* 接收到一次数据了 */
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF] = 0;/* 添加结束符 */
		strx = strstr((const char*)USART3_RX_BUF, (const char*)str);
	} 
	return strx;
}

/**************************************************************
函数名称: nbiot_send_cmd
函数功能: nbiot 发送命令
输入参数: cmd:发送的命令字符串
		  当cmd<0XFF的时候,发送数字(比如发送0X1A),
		  大于的时候发送字符串.
		  ack:期待的应答结果,如果为空,则表示不需要等待应答
		  waittime:等待时间(单位:10ms)
返回值  : 0:发送成功(得到了期待的应答结果)
		  1:发送失败
备注	: 用此函数发送命令时，命令不需加\r\n
**************************************************************/
u8 nbiot_send_cmd(char *cmd, char *ack, u16 waittime)
{	
	USART3_RX_STA = 0;
	if((u32)cmd <= 0XFF)
	{
		while(0 == (USART3->SR & 0X40));/* 等待上一次数据发送完成 */
		USART3->DR = (u32)cmd;
	}
	else
	{	
		usart3_printf("%s\r\n", cmd);//发送命令
	}
	if(ack && waittime)		/* 需要等待应答 */
	{
		while(--waittime)	/* 等待倒计时 */
		{
			if(USART3_RX_STA & 0X8000)/* 接收到期待的应答结果 */
			{
				if(nbiot_check_cmd(ack))/* 得到有效数据 */
				{
					break;
				}
                USART3_RX_STA = 0;
			} 
			delay_ms(10);
		}
		if(0 == waittime)
		{
			return 1;
		}
	}
	return 0;
} 





