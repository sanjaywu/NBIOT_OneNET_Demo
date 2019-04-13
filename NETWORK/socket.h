
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "sys.h"


enum ESOC_DOMAIN
{
	IPV4 = 1,
	IPV6 = 2
};

enum ESOC_TYPE
{
	TCP = 1,
	UDP = 2,
	RAW = 3
};

enum ESOC_PROROCOL
{
	IP = 1,
	ICMP = 2,
	UDP_LITE = 3
};





void nbiot_socket_app(void);






#endif


