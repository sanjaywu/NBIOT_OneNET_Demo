#include "customer_define.h"

#ifdef __NBIOT_MQTT__

#include "string.h"
#include "stdarg.h"	 	 
#include "stdio.h"	
#include "stdlib.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart1.h"
#include "usart3.h"
#include "mqtt.h"
#include "nbiot.h"
#include "hex2bin.h"
#include "led.h"

#define EMQNEW_TIME_OUT	1200
#define EMQNEW_BUFSIZE	256
#define MQTT_KEEP_ALIVE	3600

/* 使用时，将以下信息替换为你自己的信息 */
char *g_mqtt_server_addr = "183.230.40.39";
char *g_mqtt_server_port = "6002";
char *g_mqtt_client_id = "505550697";
char *g_mqtt_username = "194320";
char *g_mqtt_password = "LnEhym2IW1pTfBhFMjKK8s7HFJQ=";

int g_mqtt_id = 0;

char *g_emqsub_mqtt_topic = "sub_topic";
char *g_emqpub_mqtt_topic = "pub_topic";

int g_urc_emqpub_mqtt_id;
int g_urc_emqpub_mqtt_qos;
int g_urc_emqpub_mqtt_retained;
int g_urc_emqpub_mqtt_dup;
int g_urc_emqpub_mqtt_msg_len;

char g_urc_emqpub_mqtt_topic_src[64];
char g_urc_emqpub_mqtt_msg_src[64];
char g_urc_emqpub_mqtt_topic_dest[64];
char g_urc_emqpub_mqtt_msg_dest[64];

/* 任务句柄 */
TaskHandle_t g_process_mqtt_back_result_task_handle = NULL;


/**************************************************************
函数名称: nbiot_mqtt_emqnew
函数功能: 建立新的mqtt
输入参数: server：服务器地址
		  port：服务器端口
		  timeout：AT 命令等待时间，以毫秒为基本单位
		  bufsize：无符号整型，发送和接收缓冲器的大小
返回值  : 0：成功
		  1：失败
备注	: bufsize建议不要超过 10K, 这个表示临时存储发送和接收的
MQTT PDU的大小。根据需要发送大小设置，如若设置太大会造成内存浪费
**************************************************************/
u8 nbiot_mqtt_emqnew(char *server, char *port, int timeout, int bufsize)
{
	u8 res;
	char cmd_string[64];

	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+EMQNEW=\"%s\",\"%s\",%d,%d", server, port, timeout, bufsize);

	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;
}


/**************************************************************
函数名称: nbiot_mqtt_emqcon
函数功能: 向MQTT服务器发送链接报文
输入参数: mqtt_id：MQTT连接id，AT+EMQNEW的返回值
		  mqtt_version：MQTT 版本, 3 = 3.1，4 = 3.1.1
		  client_id：客服端ID，应该是唯一
		  keep_alive：保活时间
		  cleansession：清理回话，0 或者 1。
		  （0：Client 断开连接后，Server 应该保存 Client 的订阅信息。
		  1：表示 Server 应该立刻丢弃任何会话状态信息）
		  will_flag：will flag，0 或者 1（为1：表示后面带选项）
		  will_option：如果will_flag为1必包含此选项，可选。格式:
		  topic=xxx,QoS=xxx,retained=xxx,message_id=xxx,message=xxx
		  username：用户名，可选
		  password：密码，可选
返回值  : 0：成功
		  1：失败
备注	: 不建议keep_alive设置成太小的值
**************************************************************/
u8 nbiot_mqtt_emqcon(int mqtt_id, int mqtt_version, char *client_id, 
							int keep_alive, int cleansession, int will_flag, 
							char *will_option, char *username, char *password)
{
	u8 res;
	char cmd_string[256];

	memset(cmd_string, 0, sizeof(cmd_string));
	
	if(will_flag)
	{
		if((NULL == username) || (NULL == password))
		{
			sprintf(cmd_string, "AT+EMQCON=%d,%d,\"%s\",%d,%d,%d,\"%s\"", mqtt_id, mqtt_version, \
					client_id, keep_alive, cleansession, will_flag, will_option);
		}
		else
		{
			sprintf(cmd_string, "AT+EMQCON=%d,%d,\"%s\",%d,%d,%d,\"%s\",\"%s\",\"%s\"", mqtt_id, mqtt_version, \
					client_id, keep_alive, cleansession, will_flag, will_option, username, password);
		}
	}
	else
	{
		if((NULL == username) || (NULL == password))
		{
			sprintf(cmd_string, "AT+EMQCON=%d,%d,\"%s\",%d,%d,%d", mqtt_id, mqtt_version, \
					client_id, keep_alive, cleansession, will_flag);
		}
		else
		{
			sprintf(cmd_string, "AT+EMQCON=%d,%d,\"%s\",%d,%d,%d,\"%s\",\"%s\"", mqtt_id, mqtt_version, \
					client_id, keep_alive, cleansession, will_flag, username, password);
		}
	}

	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;
}

/**************************************************************
函数名称: nbiot_mqtt_emqdiscon
函数功能: 断开与MQTT服务器的连接
输入参数: mqtt_id：MQTT连接id，AT+EMQNEW的返回值
返回值  : 0：成功
		  1：失败
备注	: 无
**************************************************************/
u8 nbiot_mqtt_emqdiscon(int mqtt_id)
{
	u8 res;
	char cmd_string[64];

	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+EMQDISCON=%d", mqtt_id);

	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;							
}

/**************************************************************
函数名称: nbiot_mqtt_emqsub
函数功能: 发送 MQTT 订阅报文
输入参数: mqtt_id：MQTT连接id，AT+EMQNEW的返回值
		  topic：订阅消息的主题
		  qos：消息的 QoS, 0, 1 或者 2
返回值  : 0：成功
		  1：失败
备注	: topic 长度不能超过命令”AT+EMQNEW”设置的缓冲区大小，
因为 MQTT PDU含有 topic，整个 MQTT PDU 不能超过缓冲区大小。
**************************************************************/
u8 nbiot_mqtt_emqsub(int mqtt_id, char *topic, int qos)
{
	u8 res;
	char cmd_string[64];

	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+EMQSUB=%d,\"%s\",%d", mqtt_id, topic, qos);

	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;
}

/**************************************************************
函数名称: nbiot_mqtt_emqunsub
函数功能: 发送 MQTT 取消订阅报文
输入参数: mqtt_id：MQTT连接id，AT+EMQNEW的返回值
		  topic：订阅消息的主题
		  qos：消息的 QoS, 0, 1 或者 2
返回值  : 0：成功
		  1：失败
备注	: 无
**************************************************************/
u8 nbiot_mqtt_emqunsub(int mqtt_id, char *topic, int qos)
{
	u8 res;
	char cmd_string[64];

	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+EMQUNSUB=%d,\"%s\",%d", mqtt_id, topic, qos);

	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;
}

/**************************************************************
函数名称: nbiot_mqtt_emqpub
函数功能: 发送 MQTT 发布报文
输入参数: mqtt_id：MQTT连接id，AT+EMQNEW的返回值
		  topic：订阅消息的主题
		  qos：消息的 QoS, 0, 1 或者 2
		  retained：保留标志, 0 或者 1(0：不保留消息。1：表示是保留消息)
		  dup：重复标志， 0 或者 1（0：第一次发送；1 重复发送）
		  msg_len：发布消息的长度
		  msg：发布消息内容，必须是 ascii 码串，不支持其它字符，否则发送数据为空
返回值  : 0：成功
		  1：失败
备注	: 无
**************************************************************/
u8 nbiot_mqtt_emqpub(int mqtt_id, char *topic, int qos, 
								int retained, int dup, int msg_len, char *msg)
{
	u8 res;
	char cmd_string[64];

	memset(cmd_string, 0, sizeof(cmd_string));
	sprintf(cmd_string, "AT+EMQPUB=%d,\"%s\",%d,%d,%d,%d,\"%s\"", mqtt_id, topic, qos, retained, dup, msg_len, msg);

	printf("%s\r\n", cmd_string);
	res = nbiot_send_cmd(cmd_string, "OK", 300);

	return res;
}

/**************************************************************
函数名称: nbiot_mqtt_parse_emqnew
函数功能: 解析 EMQNEW返回的mqtt_id
输入参数: 要解析的参数
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_mqtt_parse_emqnew(int *mqtt_id)
{
	char *token = NULL;

	token = strstr((const char*)USART3_RX_BUF, "+EMQNEW");
	token = token + 9;
	*mqtt_id = token[0] - '0';
}

/**************************************************************
函数名称: nbiot_mqtt_parse_emqnew
函数功能: 解析 断开连接的mqtt_id
输入参数: 要解析的参数
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_mqtt_parse_emqdiscon(int *mqtt_id)
{
	char *token = NULL;

	token = strstr((const char*)USART3_RX_BUF, "+EMQDISCON");
	token = token + 11;
	*mqtt_id = token[0] - '0';
}

/**************************************************************
函数名称: nbiot_mqtt_parse_emqpub
函数功能: 解析 MQTT服务器下发给设备的消息或主题
输入参数: source：源
		  dest：目标
		  str_len：目标长度
返回值  : 无
备注	: 用于去掉消息或主题带有的：""
**************************************************************/
void nbiot_mqtt_parse_urc_emqpub_topic_msg(char *dest, char *source, int dest_len)
{
	char *msg_start = NULL;

	msg_start = strchr(source, '"');
	msg_start++;
	memcpy(dest, msg_start, dest_len);
	dest[dest_len] = '\0';
	//printf("dest:%s\r\n", dest);										
}

/**************************************************************
函数名称: nbiot_mqtt_parse_emqpub
函数功能: 解析 MQTT服务器下发给设备的消息
输入参数: 要解析的参数
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_mqtt_parse_emqpub(int *mqtt_id, char *topic, int *qos, 
										int *retained, int *dup, int *msg_len, char *msg)
{
	char *token = NULL;
	
	/* 用strtok函数解析 */
	token = strtok(strstr((const char*)USART3_RX_BUF, "+EMQPUB"), ", ");
	token = strtok(NULL, ", ");
	*mqtt_id = atoi(token);
	token = strtok(NULL, ", ");
	//sprintf((char*)topic, "%s", token);
	nbiot_mqtt_parse_urc_emqpub_topic_msg(topic, token, (strlen(token) - 2));
	
	token = strtok(NULL, ", ");
	*qos = atoi(token);
	token = strtok(NULL, ", ");
	*retained = atoi(token);
	token = strtok(NULL, ", ");
	*dup = atoi(token);
	token = strtok(NULL, ", ");
	*msg_len = atoi(token);
	token = strtok(NULL, ", ");
	//sprintf((char*)msg, "%s", token);
	nbiot_mqtt_parse_urc_emqpub_topic_msg(msg, token, (strlen(token) - 2));
}

/**************************************************************
函数名称: nbiot_mqtt_emqpub_msg_execute_callback
函数功能: 执行订阅到到消息回调函数
输入参数: parameter
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_mqtt_emqpub_msg_execute_callback(char *msg)
{
	printf("nbiot_mqtt_emqpub_msg_execute_callback, msg:%s\r\n", msg);

	if(0 == strcmp("LED:1", msg))
	{
		LED1 = 1;
	}
	else if(0 == strcmp("LED:0", msg))
	{
		LED1 = 0;
	}
	else
	{
		printf("error msg\r\n");
	}
}

/**************************************************************
函数名称: nbiot_process_mqtt_back_result_task
函数功能: 模组mqtt相关任务
输入参数: parameter
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_process_mqtt_back_result_task(void *parameter)
{
	while(1)
	{
		if(USART3_RX_STA & 0X8000)
		{
			if(strstr((const char*)USART3_RX_BUF, "+IP"))	/* 查到IP，说明模组开机或者重启后注册上网络了 */
			{
				nbiot_at_response(1);
				printf("nbiot network registed...\r\n");
				nbiot_mqtt_emqnew(g_mqtt_server_addr, g_mqtt_server_port, EMQNEW_TIME_OUT, EMQNEW_BUFSIZE);
			}
			else if(strstr((const char*)USART3_RX_BUF, "+EMQNEW"))
			{
				nbiot_mqtt_parse_emqnew(&g_mqtt_id);
				printf("g_mqtt_id:%d\r\n", g_mqtt_id);
				if(0 == nbiot_mqtt_emqcon(g_mqtt_id, 4, g_mqtt_client_id, MQTT_KEEP_ALIVE, 0, 0, NULL, g_mqtt_username, g_mqtt_password))
				{
					printf("connect mqtt server success\r\n");
				}
				if(0 == nbiot_mqtt_emqsub(g_mqtt_id, g_emqsub_mqtt_topic, MQTT_QOS_LEVEL1))
				{
					printf("subscribe mqtt topic success, topic:%s\r\n", g_emqsub_mqtt_topic);
				}
				nbiot_at_response(1);
			}
			else if(strstr((const char*)USART3_RX_BUF, "+EMQDISCON"))
			{
				printf("server disconnect device\r\n");
				nbiot_mqtt_emqdiscon(g_mqtt_id);
				nbiot_at_response(1);
			}
			else if(strstr((const char*)USART3_RX_BUF, "+EMQPUB:"))
			{
				memset(g_urc_emqpub_mqtt_topic_src, 0, sizeof(g_urc_emqpub_mqtt_topic_src));
				memset(g_urc_emqpub_mqtt_msg_dest, 0, sizeof(g_urc_emqpub_mqtt_msg_dest));
				memset(g_urc_emqpub_mqtt_topic_src, 0, sizeof(g_urc_emqpub_mqtt_topic_src));
				
				nbiot_mqtt_parse_emqpub(&g_urc_emqpub_mqtt_id, g_urc_emqpub_mqtt_topic_src, &g_urc_emqpub_mqtt_qos, &g_urc_emqpub_mqtt_retained, &g_urc_emqpub_mqtt_dup, &g_urc_emqpub_mqtt_msg_len, g_urc_emqpub_mqtt_msg_src);
				hex_to_bin(g_urc_emqpub_mqtt_msg_dest, g_urc_emqpub_mqtt_msg_src, (g_urc_emqpub_mqtt_msg_len * 2));
				nbiot_mqtt_emqpub_msg_execute_callback(g_urc_emqpub_mqtt_msg_dest);
				nbiot_at_response(1);
			}
			else if(strstr((const char*)USART3_RX_BUF, "auto-reboot"))/* 模组发生异常重启现象 */
			{
				nbiot_at_response(1);
			}
			else
			{
				nbiot_at_response(1);
			}
		}	
	}
}

/**************************************************************
函数名称: nbiot_process_mqtt_back_result_task_init
函数功能: 创建模组返回结果处理r任务函数
输入参数: 无
返回值  : 无
备注	: 无
**************************************************************/
void nbiot_process_mqtt_back_result_task_init(void)
{
	if(g_process_mqtt_back_result_task_handle == NULL) 
	{
		xTaskCreate(nbiot_process_mqtt_back_result_task,
			"nbiot_process_mqtt_back_result_task",
			1024 * 4 / sizeof(portSTACK_TYPE),
			NULL,
			4,
			&g_process_mqtt_back_result_task_handle);
	}
}


void nbiot_mqtt_app(void)
{
	nbiot_process_mqtt_back_result_task_init();
}

#endif


