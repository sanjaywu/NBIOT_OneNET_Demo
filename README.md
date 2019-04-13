
- 温湿度传感器：SHT20
- NBIOT模组：所有MT2625方案的NBIOT模组
- 涉及协议：MQTT、TCP（Socket）
- MCU：STM32F103RET6
- 晶振：12M
- 操作系统：FreeRTOS

打开customer_define.h，通过如下两个宏开关来选择用MQTT还是SOCKET

- #define __NBIOT_MQTT__
- #define __NBIOT_SOCKET__

MQTT实现LED灯的控制
SOCKET实现POST/GET数据到OneNET的HTTP服务器