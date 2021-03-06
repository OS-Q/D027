#include "xy_iic.h"

void delay_us_iic(unsigned int us)
{
	unsigned int i,j;
	
	for(i=0;i<us;i++)
	{
		for(j=0;j<8;j++)
			;
	}
}
 
//��ʼ��IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 
}

//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	SET_IIC_SDA;	  	  
	SET_IIC_SCL;
	delay_us_iic(4);
 	CLR_IIC_SDA;//START:when CLK is high,DATA change form high to low 
	delay_us_iic(4);
	CLR_IIC_SCL;//ǯסI2C���ߣ�׼�����ͻ�������� 
}

//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	CLR_IIC_SCL;
	CLR_IIC_SDA;//STOP:when CLK is high DATA change form low to high
 	delay_us_iic(4);
	SET_IIC_SCL; 
	SET_IIC_SDA;//����I2C���߽����ź�
	delay_us_iic(4);							   	
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
	SET_IIC_SDA;
	delay_us_iic(1);	   
	SET_IIC_SCL;
	delay_us_iic(1);	 
	while(GET_IIC_SDA==1)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	CLR_IIC_SCL;//ʱ�����0 	   
	return 0;  
} 

//����ACKӦ��
void IIC_Ack(void)
{
	CLR_IIC_SCL;
	SDA_OUT();
	CLR_IIC_SDA;
	delay_us_iic(2);
	SET_IIC_SCL;
	delay_us_iic(2);
	CLR_IIC_SCL;
}

//������ACKӦ��		    
void IIC_NAck(void)
{
	CLR_IIC_SCL;
	SDA_OUT();
	SET_IIC_SDA;
	delay_us_iic(2);
	SET_IIC_SCL;
	delay_us_iic(2);
	CLR_IIC_SCL;
}

//IIC����һ���ֽ�
//���شӻ�����Ӧ��
// 1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(uint8_t txd)
{                        
	uint8_t t;
	
	SDA_OUT(); 	    
	CLR_IIC_SCL;
	
	for(t=0;t<8;t++)
	{              
		if((txd&0x80)>>7)
		{
			SET_IIC_SDA;
		}
		else
		{
			CLR_IIC_SDA;
		}
		
		txd<<=1; 	  
		delay_us_iic(2);
		SET_IIC_SCL;
		delay_us_iic(2); 
		CLR_IIC_SCL;
		delay_us_iic(2);
	}	 
} 

//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	
	SDA_IN();//SDA����Ϊ����
	for(i=0;i<8;i++ )
	{
		CLR_IIC_SCL;
		delay_us_iic(2);
		SET_IIC_SCL;
		receive<<=1;
		if(GET_IIC_SDA==1)receive++;   
		delay_us_iic(1); 
	}
	
	if (!ack)
		IIC_NAck();//����nACK
	else
		IIC_Ack(); //����ACK   
		
	return receive;
}



























