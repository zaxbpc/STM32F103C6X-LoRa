#include "sth_send.h"

void LoRa_SendData(u8 a)
{
	u16 len;	
	float* temhum;                                   //定义数组tmhum用于暂时储存温湿度数据
	float data_frame[5];                             //定义数组data_frame用于存储每次发送的数据值
	temhum = SHT30_read_result(SHT30_DEVADDR);       //温湿度传感器采集数据并将返回值赋给数组temhum
		data_frame[0] = 0x00;                          //第一个数据为组号
		data_frame[1] = 0x04;                          //第二个数据为地址
		data_frame[2] = 0x17;                          //第三个数据为校验位
		data_frame[3] = temhum[0];                     //第四个数据为温度
		data_frame[4] = temhum[1];                     //第五个数据为湿度
		len = sizeof(data_frame);                      //len为数据长度
		GPIO_SetBits(GPIOB,GPIO_Pin_6);
		GPIO_ResetBits(GPIOB,GPIO_Pin_5);
	  USART_Send(data_frame,len);
}