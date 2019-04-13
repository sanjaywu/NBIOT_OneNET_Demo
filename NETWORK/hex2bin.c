
#include "hex2bin.h"

/**************************************************************
函数名称: bin_to_hex
函数功能: 字符串转16进制字符串
输入参数: source：源字符串
		  dest：转换后的目标
		  max_dest_len：目标长度
返回值  : 目标实际长度
备注	: 无
**************************************************************/
int bin_to_hex(char *dest, const char *source, int max_dest_len)
{
    int i = 0, j = 0;
    char ch1, ch2;
	
    while (j + 1 < max_dest_len)
    {
        ch1 = (source[i] & 0xF0) >> 4;
        ch2 = source[i] & 0x0F;

        if(ch1 <= 9) 
		{
            *(dest + j) = ch1 + '0';
        } 
		else
		{
            *(dest + j) = ch1 + 'A' - 10;
        }

        if(ch2 <= 9)
		{
            *(dest + j + 1) = ch2 + '0';
        }
		else 
		{
            *(dest + j + 1) = ch2 + 'A' - 10;
        }

        i++;
        j += 2;
    }

    *(dest + j) = '\0';

	return j;
}

/**************************************************************
函数名称: bin_to_hex
函数功能: 16进制字符串转字符串
输入参数: source：源字符串
		  dest：转换后的目标
		  max_dest_len：目标长度
返回值  : 目标实际长度
备注	: 
**************************************************************/
int hex_to_bin(char *dest, const char *source, int max_dest_len)
{
    int i = 0;
    char lower_byte, upper_byte;

    while (source[i] != '\0')
	{
        if(source[i] >= '0' && source[i] <= '9')
		{
            lower_byte = source[i] - '0';
        } 
		else if(source[i] >= 'a' && source[i] <= 'f') 
		{
            lower_byte = source[i] - 'a' + 10;
        }
		else if(source[i] >= 'A' && source[i] <= 'F') 
		{
            lower_byte = source[i] - 'A' + 10;
        } 
		else 
		{
            return 0;
        }

        if(source[i + 1] >= '0' && source[i + 1] <= '9')
		{
            upper_byte = source[i + 1] - '0';
        } 
		else if (source[i + 1] >= 'a' && source[i + 1] <= 'f') 
		{
            upper_byte = source[i + 1] - 'a' + 10;
        } 
		else if(source[i + 1] >= 'A' && source[i + 1] <= 'F')
		{
            upper_byte = source[i + 1] - 'A' + 10;
        } 
		else 
		{
            return 0;
        }

        if((i >> 1) >= max_dest_len) 
		{
            return (i >> 1);
        }

        *(dest + (i >> 1)) = (lower_byte << 4) + upper_byte;
        i += 2;
    }
	
	*(dest + (i >> 1)) = '\0';

	 return (i >> 1);
}

