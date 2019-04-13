#include "wdg.h"

/**************************************************************
函数名称: iwdg_init
函数功能: 初始化独立看门狗
输入参数: prer：分频数:0~7(只有低3位有效!)
		  分频因子=4*2^prer.但最大值只能是256!
		  prlr：重装载寄存器值:低11位有效
返回值  : 无
备注	: 时间计算(大概):Tout=((4*2^prer)*rlr)/40 (ms)
**************************************************************/
void iwdg_init(u8 prer, u16 rlr)
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  	/* 使能对寄存器IWDG_PR和IWDG_RLR的写操作 */
	IWDG_SetPrescaler(prer);  						/* 设置IWDG预分频值 */
	IWDG_SetReload(rlr);  							/* 设置IWDG重装载值 */
	IWDG_ReloadCounter();  							/* 按照IWDG重装载寄存器的值重装载IWDG计数器 */
	IWDG_Enable();  								/* 使能IWDG */
}

/**************************************************************
函数名称: iwdg_feed
函数功能: 喂独立看门狗
输入参数: 无
返回值  : 无
备注	: 无
**************************************************************/
void iwdg_feed(void)
{   
 	IWDG_ReloadCounter();
}






















