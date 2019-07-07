#ifndef __XY_IIC_H
#define __XY_IIC_H
#include "stm32f10x.h"

//IO��������
#define SDA_IN()  {GPIOB->CRL   &=0x0FFFFFFF;GPIOB->CRL  |=0x80000000;}
#define SDA_OUT() {GPIOB->CRL &=0x0FFFFFFF;GPIOB->CRL  |=0x30000000;}

#define SET_IIC_SCL		GPIO_SetBits(GPIOB,GPIO_Pin_6)
#define CLR_IIC_SCL		GPIO_ResetBits(GPIOB,GPIO_Pin_6)

#define SET_IIC_SDA		GPIO_SetBits(GPIOB,GPIO_Pin_7)
#define CLR_IIC_SDA		GPIO_ResetBits(GPIOB,GPIO_Pin_7)

#define GET_IIC_SDA		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)

//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	  
#endif
















