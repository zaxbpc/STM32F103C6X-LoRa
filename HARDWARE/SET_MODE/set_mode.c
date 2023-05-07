/**
  ***************************************************************************************************
  * @author  lilin
  * @version V1.0
  * @date    2021.10.16
  * @brief   通过IO口电平的变化，来改变MD0和MD1的值，从而改变LoRa的工作模式
  ***************************************************************************************************
  * @attention
  *一、功能说明
	*	MD0==1,MD1==0----------------------该模式为发送模式
	*   MD0==0,MD1==0----------------------该模式为配置模式
	*   通过它的不断闪烁来判断程序是否正常运行。
  ***************************************************************************************************
  */
#include "set_mode.h"

void SET_MODE_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB端口时钟
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //MOD0-->PA5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.5
	GPIO_ResetBits(GPIOA,GPIO_Pin_5);						 //PA.5 输出低
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //MOD1-->PA4 端口配置
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.4
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);						 //PA.4 输出低
}
void Config_Mode(){
	MD0 = 0;
	MD1 = 0;
}
void Transmit_Mode(){
	MD0 = 1;
	MD1 = 0;
}
