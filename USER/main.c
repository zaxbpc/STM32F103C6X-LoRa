//=============================================================================
//	Project			:	sht30
//	Version			:	V1.0
//	File				:	main.c
//	Author			:	eonegh
//	Date				:	2023-04-20
//	Controller	:	STM32F103C6T6
//	IDE					:	Keil uVision5 V5.36.0.0
//	Compiler		:	eonegh
//	Brief				:	主函数
//=============================================================================
#include "sht30.h"	//温湿度传感器sht30采集数据
#include "usart.h"	//串口debug（printf函数）API
#include "led.h"		//LED控制API，用于debug

//实验二LoRa的无线传输功能的实现主要在这两个文件里面
#include "usart2.h" //串口2通信配置,和LoRa模块连接，数据无线发送和接收函数
#include "set_mode.h"//LoRa工作模式设置，配置模式和传输模式

//根据节点不同类型，选择不同的定义，比如当前是感知节点，就注释掉其余两行代码
#define PERCEPTION 1//定义当前节点为感知节点
//#define ROUTER 1//定义当前节点为路由节点
//#define GATEWAY 1//定义当前节点为网关节点

#define SEND_ADDR 0x08//发送给下一个节点的地址，当前的下一个地址是00 0x08 0x17 可根据实际情况更改

#define NUM 4//发送数据包的长度，可根据实际情况更改
u8 DATA_BUF[NUM] = {0x00};
u8 REC_DATA_BUF[64] = {0x00};//路由节点接收到感知节点的数据存储到这个数组里面
u8 GPRS_BUF[64] = {0x00};//网关节点接收到感知节点的数据存储到这个数组里面
void buildDATA(void);

int main(void)
{ 
	u8 len = 0;//接收数据的长度
	u8 i = 0;
	
	delay_init();				//延时函数初始化
	
	//系统有多个中断响应时，要进行优先级分组设置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	
//串口1波特率为115200，用于使用printf函数进行debug
	USART1_PinConfig();
	USART1_Init(115200);
	
//串口2初始化及端口配置	
	drv_uart_init( 9600 ); //串口初始化  根据模块波特率变化
	SET_MODE_Init();//初始化MD0和MD1引脚
	Transmit_Mode();//设置为传输模式
	
	IIC_PinConfig();		//初始化IIC，用于MCU与SHT30通信，这里使用软件模拟方式实现
	LED_PinConfig();		//初始化LED，用于调试
  

	
	while(1)
	{
		//感知节点采集温湿度，并通过LoRa传输给路由节点
		#if PERCEPTION
		if(SHT30_ReadTempHum(SHT30_DEVADDR)){
			printf("Temperature = %5.2fC, Humidity = %5.2f%%RH \r\n",Temperature,Humidity);
			//采集到的温湿度转换后存到DATA_BUF里面，然后通过LoRa发送出去
			buildDATA();
			ASxx_tx_packet(0x00,SEND_ADDR,0x17,DATA_BUF,NUM);
		}
		else{
			printf("Temperature or Humidity illegal! \r\n");	//采集的温湿度数据非法，丢弃
		} 
		#endif
		
		//路由节点，接收感知节点的数据，并转发给网关节点
		#if ROUTER
			if(USART2_RX_STA&0x8000){//如果接收到正确的数据，USART2_RX_STA&0x8000会等于1
				len=USART2_RX_STA&0x3f; //得到此次接收到的数据长度
				//打印当前接收的数据
				for(i = 0; i < len; i++){
					printf("USART2_RX_BUF[%0d] == %0d\r\n",i,USART2_RX_BUF[i]);
					REC_DATA_BUF[i] = USART2_RX_BUF[i];//将接收到的数据存到REC_DATA_BUF里面
				}
				ASxx_tx_packet(00,SEND_ADDR,17,REC_DATA_BUF,len);//然后将数据转发给网关节点
				USART2_RX_STA = 0;
			}	
		#endif
		
		//网关节点，接收路由节点的数据，并上传给服务器
		#if GATEWAY
			if(USART2_RX_STA&0x8000){//如果接收到正确的数据，USART2_RX_STA&0x8000会等于1
				len=USART2_RX_STA&0x3f; //得到此次接收到的数据长度
				//打印当前接收的数据
				for(i = 0; i < len; i++){
					printf("USART2_RX_BUF[%0d] == %0d\r\n",i,USART2_RX_BUF[i]);
					GPRS_BUF[i] = USART2_RX_BUF[i];//将接收到的数据存到GPRS_BUF里面
				}
				
				//这里需要自己补充一点代码，就是将GPRS_BUF的数据传输到服务器，实验三的内容
				
				//
				
				USART2_RX_STA = 0;
			}	
			
		#endif
		LED0 = !LED0;		//LED每秒闪烁一次,用于debug
		
		delay_ms(1000);
	}
	
 return 0;
}

//将采集到的温湿度构建成一个完整的数据帧进行数据传输
void buildDATA(void){
	u16 Tem_4 = 0;
	u16 Hum_4 = 0;
	
	//Temperature = 28.40C, Humidity = 36.85%RH 
	//1、采集到的温湿度是小数，但是串口传输的是u8类型的整数，所以要先小数转整数
	Tem_4 = (int) (Temperature*100);
	Hum_4 = (int) (Humidity*100);
	//2、u8类型的数值大小范围为0-255，2840显然超出范围了，所以要进行拆分
	//这里的思路是用两个u8来存储，分别存储高八位和低八位
	//拆分成四个8位数据
	
	DATA_BUF[0] = (Tem_4 >> 8) & (0xFF);//温度高八位
	DATA_BUF[1] = (Tem_4 & 0xFF);//温度低八位
		
	DATA_BUF[2] = (Hum_4 >> 8) & (0xFF);//湿度高八位
	DATA_BUF[3] = (Hum_4 & 0xFF);//湿度低八位
	
	//如果要想添加其他数据，就在这后面继续添加就行
	//DATA_BUF[4] = xx;
	//
}


/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
