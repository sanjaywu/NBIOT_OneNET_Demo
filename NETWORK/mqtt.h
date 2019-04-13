
#ifndef __MQTT_H__
#define __MQTT_H__

#include "sys.h"

enum MQTT_QOS_LEVEL
{
    MQTT_QOS_LEVEL0 = 0,  /* 最多发送一次 */
    MQTT_QOS_LEVEL1 = 1,  /* 最少发送一次  */
    MQTT_QOS_LEVEL2 = 2   /* 只发送一次 */
};

u8 nbiot_mqtt_emqnew(char *server, char *port, int timeout, int bufsize);
u8 nbiot_mqtt_emqcon(int mqtt_id, int mqtt_version, char *client_id, 
							int keep_alive, int cleansession, int will_flag, 
							char *will_option, char *username, char *password);
u8 nbiot_mqtt_emqdiscon(int mqtt_id);
u8 nbiot_mqtt_emqsub(int mqtt_id, char *topic, int qos);
u8 nbiot_mqtt_emqunsub(int mqtt_id, char *topic, int qos);
u8 nbiot_mqtt_emqpub(int mqtt_id, char *topic, int qos, 
								int retained, int dup, int msg_len, char *msg);
void nbiot_mqtt_parse_emqnew(int *mqtt_id);
void nbiot_mqtt_parse_emqdiscon(int *mqtt_id);
void nbiot_mqtt_parse_emqpub(int *mqtt_id, char *topic, int *qos, 
										int *retained, int *dup, int *msg_len, char *msg);

void nbiot_mqtt_app(void);





#endif








