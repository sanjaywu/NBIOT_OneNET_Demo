#include "customer_define.h"

#ifdef __NBIOT_SOCKET__

#include "string.h"
#include "stdarg.h"	 	 
#include "stdio.h"	
#include "stdlib.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart1.h"
#include "usart3.h"
#include "nbiot.h"
#include "socket.h"
#include "hex2bin.h"
#include "sht20.h"

char *g_socket_remote_addr = "183.230.40.33";
int   g_socket_remote_port = 80;
int   g_socket_id = 0;

extern _sht20_data_t sht20_data_t;
char *g_post_data_stream_name = "temperature";
char g_post_value_point[5];
char g_socket_esosend_data_src[256];
char g_socket_esosend_data_dest[512];

/* 使用时，dev_id和api_key替换为自己的 */
char *g_dev_id = "505619290";
char *g_api_key = "SlxhH3MCLvuuvXJ0N=a14Yo6EAQ=";

int g_socket_error_socket_id = 0;
int g_socket_error_code = 0;

int g_socket_esonmi_socket_id = 0;
int g_socket_esonmi_data_len = 0;
char g_socket_esonmi_data_src[512];
char g_socket_esonmi_data_dest[256];

u16 g_hardware_times = 0;
u8 g_post_data_flag = 0;
#define NBIOT_SOCKET_SEND_TIME 300
#define NBIOT_SOCKET_SEND_MAX_LEN	1024
/* 任务句柄 */
TaskHandle_t g_process_socket_back_result_task_handle = NULL;

/**************************************************************
函数名称: nbiot_socket_esoc
函数功能: 创建一个 TCP/UDP
输入参数: domain：1 - IPv4, 2 - IPv6
		  type：1 - TCP, 2 - UDP, 3 - RAW
		  protocol：1 - IP, 2 - ICMP
返回值  : 0：成功
		  1：失败
备注	: 发送成功之后，会生成一个socket_id,范围0-4
**************************************************************/
u8 nbiot_socket_esoc(int domain, int type, int protocol)
{
	u8 res;
	char cmd_string[64];
	
	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+ESOC=%d,%d,%d", domain, type, protocol);
	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;
}

/**************************************************************
函数名称: nbiot_socket_esob
函数功能: 绑定当地地址和端口，主要用于设置本地端口
输入参数: socket_id：范围0-4，AT+ESOC返回的socket_id
		  local_port：本地端口
		  local_addr：本地地址
返回值  : 0：成功
		  1：失败
备注	: 无
**************************************************************/
u8 nbiot_socket_esob(int socket_id, int local_port, char *local_addr)
{
	u8 res;
	char cmd_string[64];
	
	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+ESOB=%d,%d,\"%s\"", socket_id, local_port, local_addr);
	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;

}

/**************************************************************
函数名称: nbiot_socket_esocon
函数功能: 连接远程服务器
输入参数: socket_id：范围0-4，AT+ESOC返回的socket_id
		  remote_port：远程服务器端口
		  remote_addr：远程服务器地址
返回值  : 0：成功
		  1：失败
备注	: 无
**************************************************************/
u8 nbiot_socket_esocon(int socket_id, int remote_port, char *remote_addr)
{
	u8 res;
	char cmd_string[64];
	
	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+ESOCON=%d,%d,\"%s\"", socket_id, remote_port, remote_addr);
	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;
}

/**************************************************************
函数名称: nbiot_socket_esosend
函数功能: 发送数据
输入参数: socket_id：范围0-4，AT+ESOC返回的socket_id
		  data_len：十六进制格式 ASCII 码数据长度de的1/2
		  data：十六进制格式 ASCII 码数据
		  flag：发送标志位，1-ack不延迟. 2–没有nagle算法,可选
返回值  : 0：成功
		  1：失败
备注	: 无
**************************************************************/
u8 nbiot_socket_esosend(int socket_id, int data_len, char *data, int flag)
{
	u8 res;
	char cmd_string[NBIOT_SOCKET_SEND_MAX_LEN];
	
	memset(cmd_string, 0, sizeof(cmd_string));
	if((1 == flag) || (2 == flag))
	{
		sprintf(cmd_string, "AT+ESOSEND=%d,%d,%s,%d", socket_id, data_len, data, flag);
	}
	else
	{
		sprintf(cmd_string, "AT+ESOSEND=%d,%d,%s", socket_id, data_len, data);
	}
	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;

}

/**************************************************************
函数名称: nbiot_socket_esocl
函数功能: 关闭socket
输入参数: socket_id：范围0-4，AT+ESOC返回的socket_id
返回值  : 0：成功
		  1：失败
备注	: 无
**************************************************************/
u8 nbiot_socket_esocl(int socket_id)
{
	u8 res;
	char cmd_string[32];
	
	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+ESOCL=%d", socket_id);
	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;
}

/**************************************************************
函数名称: nbiot_socket_parse_esoc
函数功能: 解析 ESOC返回的socket_id
输入参数: 要解析的参数
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_socket_parse_esoc(int *socket_id)
{
	char *token = NULL;

	token = strstr((const char*)USART3_RX_BUF, "+ESOC");
	token = token + 6;
	*socket_id = token[0] - '0';
}

/**************************************************************
函数名称: nbiot_socket_parse_esonmi
函数功能: 解析服务器下发回来的消息数据
输入参数: socket_id：范围0-4，AT+ESOC返回的socket_id
		  data_len：数据长度，十六进制格式 ASCII 码数据的1/2
		  data：数据，十六进制格式 ASCII 码数据
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_socket_parse_esonmi(int *socket_id, int *data_len, char *data)
{
	char *token = NULL;
	
	/* 用strtok函数解析 */
	token = strtok(strstr((const char*)USART3_RX_BUF, "+ESONMI"), ",");
	*socket_id = atoi(token);
	token = strtok(NULL, ",");
	*data_len = atoi(token);
	token = strtok(NULL, ",");
	sprintf((char*)data, "%s", token);
}

/**************************************************************
函数名称: nbiot_socket_parse_esoerr
函数功能: 解析发生ERROR时的socket_id和error_code
输入参数: 要解析的参数
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_socket_parse_esoerr(int *socket_id, int *error_code)
{
	char *token = NULL;
	
	/* 用strtok函数解析 */
	token = strtok(strstr((const char*)USART3_RX_BUF, "+ESONMI"), ",");
	*socket_id = atoi(token);
	token = strtok(NULL, ",");
	*error_code = atoi(token);
}

/**************************************************************
函数名称: nbiot_socket_send_data_to_server_process
函数功能: 处理socket要发送的数据
输入参数: parameter
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_socket_send_data_to_server_process(void)
{
	char post_content[512];
	char post_content_len[4];
	unsigned int data_dest_len = 0;
	
	memset(post_content, 0, sizeof(post_content));
	memset(g_socket_esosend_data_src, 0, sizeof(g_socket_esosend_data_src));
	
	memset(g_post_value_point, 0, sizeof(g_post_value_point));
	sht20_get_value();
	sprintf(g_post_value_point, "%0.1f", sht20_data_t.sht20_temperture);
	g_post_value_point[4] = '\0';
	
	sprintf(post_content,"{\"datastreams\":[{\"id\":\"%s\",\"datapoints\":[{\"value\":%s}]}]}", g_post_data_stream_name, g_post_value_point);
	sprintf(post_content_len,"%d",strlen(post_content));
		
	strcat(g_socket_esosend_data_src, "POST /devices/");
	strcat(g_socket_esosend_data_src, g_dev_id);
	strcat(g_socket_esosend_data_src, "/datapoints HTTP/1.1\r\n");
	strcat(g_socket_esosend_data_src, "api-key:");
	strcat(g_socket_esosend_data_src, g_api_key);
	strcat(g_socket_esosend_data_src, "\r\n");
	strcat(g_socket_esosend_data_src, "Host:api.heclouds.com\r\n");
	strcat(g_socket_esosend_data_src, "Content-Length:");
	strcat(g_socket_esosend_data_src, post_content_len);
	strcat(g_socket_esosend_data_src, "\r\n\r\n");
	strcat(g_socket_esosend_data_src, post_content);
	strcat(g_socket_esosend_data_src, "\r\n\r\n");

	printf("g_socket_esosend_data_src:\r\n%s\r\n", g_socket_esosend_data_src);

	data_dest_len = bin_to_hex(g_socket_esosend_data_dest, g_socket_esosend_data_src, (2 * strlen(g_socket_esosend_data_src)));
	
	printf("g_socket_esosend_data_dest:\r\n%s\r\n", g_socket_esosend_data_dest);
	printf("data_dest_len:\r\n%d\r\n", data_dest_len);
	
}

/**************************************************************
函数名称: nbiot_process_socket_back_result_task
函数功能: 模组返回结果处理任务函数
输入参数: parameter
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_process_socket_back_result_task(void *parameter)
{
	while(1)
	{
		if(USART3_RX_STA & 0X8000)
		{
			if(strstr((const char*)USART3_RX_BUF, "+IP"))	/* 查到IP，说明模组开机或者重启后注册上网络了 */
			{
				nbiot_at_response(1);
				printf("nbiot network registed...\r\n");
				nbiot_socket_esoc(IPV4, TCP, IP);
			}
			else if(strstr((const char*)USART3_RX_BUF, "+ESOC"))
			{
				nbiot_socket_parse_esoc(&g_socket_id);
				printf("g_socket_id:%d\r\n", g_socket_id);

				if(0 == nbiot_socket_esocon(g_socket_id, g_socket_remote_port, g_socket_remote_addr))
				{
					printf("nbiot connect server success\r\n");
				}
				nbiot_socket_send_data_to_server_process();
				if(0 == nbiot_socket_esosend(g_socket_id, (strlen(g_socket_esosend_data_dest)/2), g_socket_esosend_data_dest, 0))
				{
					printf("socket esosend ok\r\n");
					TIM_SetCounter(TIM3, 0);	/* 计数器清空 */
					TIM_Cmd(TIM3, ENABLE);  	/* 使能TIMx */
				}
				nbiot_at_response(1);
			}
			else if(strstr((const char*)USART3_RX_BUF, "+ESONMI"))
			{
				nbiot_socket_parse_esonmi(&g_socket_esonmi_socket_id, &g_socket_esonmi_data_len, g_socket_esonmi_data_src);
				hex_to_bin(g_socket_esonmi_data_dest, g_socket_esonmi_data_src, g_socket_esonmi_data_len);
				printf("\r\n%s\r\n", g_socket_esonmi_data_dest);
				nbiot_at_response(1);
			}
			else if(strstr((const char*)USART3_RX_BUF, "+ESOERR"))
			{
				printf("nbiot socket error\r\n");
				nbiot_socket_parse_esoerr(&g_socket_error_socket_id, &g_socket_error_code);
				printf("+ESOERR=%d,%d", g_socket_error_socket_id, g_socket_error_code);	
				nbiot_socket_esocl(g_socket_error_socket_id);
				nbiot_at_response(1);
			}
			else if(strstr((const char*)USART3_RX_BUF, "auto-reboot"))/* 模组发生异常重启现象 */
			{
				nbiot_at_response(1);
			}
			else if(1 == g_post_data_flag)
			{
				TIM_Cmd(TIM3, DISABLE);  	/* 关闭TIMx */
				nbiot_socket_esoc(IPV4, TCP, IP);
				g_post_data_flag = 0;
				
			}
			else
			{
				nbiot_at_response(1);
			}
		}	
	}
}

/**************************************************************
函数名称: nbiot_process_socket_back_result_task_init
函数功能: 创建模组返回结果处理任务函数
输入参数: 无
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_process_socket_back_result_task_init(void)
{
	if(g_process_socket_back_result_task_handle == NULL) 
	{
		xTaskCreate(nbiot_process_socket_back_result_task,
			"nbiot_process_socket_back_result_task",
			1024 * 4 / sizeof(portSTACK_TYPE),
			NULL,
			4,
			&g_process_socket_back_result_task_handle);
	}
}

/**************************************************************
函数名称: TIM3_IRQHandler
函数功能: 定时器3中断服务函数
输入参数: 无
返回值  : 无
备注	: 定时来post数据
**************************************************************/
void TIM3_IRQHandler(void)
{	
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  /* 检查TIM3更新中断发生与否 */
	{
		printf("g_hardware_times=%d\r\n", g_hardware_times);
		
		g_hardware_times++;
		if(g_hardware_times >= NBIOT_SOCKET_SEND_TIME)
		{
			nbiot_socket_esocl(g_socket_id);
			printf("enter post, g_hardware_times=%d\r\n", g_hardware_times);
			g_post_data_flag = 1;
			#ifdef __NBIOT_OPEN_SLEEP_MODE__
			NBIOT_WAKE_UP_LOW;
			nbiot_send_cmd("AT", "OK", 0);
			NBIOT_WAKE_UP_HIGH;
			#else
			nbiot_send_cmd("AT", "OK", 0);
			#endif
			g_hardware_times = 0;
		}
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  /* 清除TIMx更新中断标志 */
	}
}


void nbiot_socket_app(void)
{
	nbiot_process_socket_back_result_task_init();
}


#endif





