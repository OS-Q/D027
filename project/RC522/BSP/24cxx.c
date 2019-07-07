#include "24cxx.h"
#include "delay.h"
#include "app_cfg.h"
#include "debug.h"
#include "timer2.h"
#include "open_result_task.h"

//��ʼ��IIC�ӿ�
int AT24CXX_Init(void)
{
	IIC_Init();
  return AT24CXX_Check();
}
//��AT24CXXָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ  
//����ֵ  :����������
uint8_t AT24CXX_ReadOneByte(uint16_t ReadAddr)
{				  
	uint8_t temp=0;		  	    																 
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //����д����
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//���͸ߵ�ַ
		IIC_Wait_Ack();		 
	}else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //����������ַ0XA0,д���� 	 

	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr%256);   //���͵͵�ַ
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //�������ģʽ			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();//����һ��ֹͣ����	    
	return temp;
}
//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//DataToWrite:Ҫд�������
void AT24CXX_WriteOneByte(uint16_t WriteAddr,uint8_t DataToWrite)
{				   	  	    																 
	IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //����д����
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//���͸ߵ�ַ
	}
	else
	{
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //����������ַ0XA0,д���� 
	}
	
	IIC_Wait_Ack();	   
	IIC_Send_Byte(WriteAddr%256);   //���͵͵�ַ
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //�����ֽ�							   
	IIC_Wait_Ack();  		    	   
	IIC_Stop();//����һ��ֹͣ���� 
	
	delay_ms(10);// ����ֵ��Ҫ�Լ�����
}
//��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������
//�ú�������д��16bit����32bit������.
//WriteAddr  :��ʼд��ĵ�ַ  
//DataToWrite:���������׵�ַ
//Len        :Ҫд�����ݵĳ���2,4
void AT24CXX_WriteLenByte(uint16_t WriteAddr,uint32_t DataToWrite,uint8_t Len)
{  	
	uint8_t t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
}

//��AT24CXX�����ָ����ַ��ʼ��������ΪLen������
//�ú������ڶ���16bit����32bit������.
//ReadAddr   :��ʼ�����ĵ�ַ 
//����ֵ     :����
//Len        :Ҫ�������ݵĳ���2,4
uint32_t AT24CXX_ReadLenByte(uint16_t ReadAddr,uint8_t Len)
{  	
	uint8_t t;
	uint32_t temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 	 				   
	}
	return temp;												    
}
//���AT24CXX�Ƿ�����
//��������24XX�����һ����ַ(255)���洢��־��.
//���������24Cϵ��,�����ַҪ�޸�
//����1:���ʧ��
//����0:���ɹ�
int AT24CXX_Check(void)
{
	uint8_t temp;
	OS_ERR err;
	temp=AT24CXX_ReadOneByte(254);//����ÿ�ο�����дAT24CXX			   
	if(temp==EEPROM_MAGIC_FLAG)
    return 0;		   
	else//�ų���һ�γ�ʼ�������
	{
		reset_eeprom();
		OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
		AT24CXX_WriteOneByte(254,EEPROM_MAGIC_FLAG);
	  temp=AT24CXX_ReadOneByte(254);	  
		if(temp==EEPROM_MAGIC_FLAG)
      return 0;
	}
	return -1;											  
}

//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void AT24CXX_Read(uint16_t ReadAddr,uint8_t *pBuffer,uint16_t NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}  
//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
void AT24CXX_Write(uint16_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite)
{
	uint8_t readback,i;
  while(NumToWrite--)
	{
		for(i = 0; i < 5; i++)
    {
      AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
      readback = AT24CXX_ReadOneByte(WriteAddr);
      if(readback != *pBuffer)
      {
        //DBG_ERROR("write to AT24CXX error %d times,addr=%d,w_data=0x%x,r_data=0x%x\n",i,WriteAddr,*pBuffer,readback);
        delay_ms(1);
      } else {
        //DBG_INFO("write to AT24CXX ok,addr=%d,w_data=0x%x,r_data=0x%x\n",WriteAddr,*pBuffer,readback);
        break;
      }
      if(i >= 5)
      {
        DBG_ERROR("write to AT24CXX error try %d times,addr=%d,w_data=0x%x,r_data=0x%x\n",i,WriteAddr,*pBuffer,readback);
      }  
    }  
		WriteAddr++;
		pBuffer++;
	}
}

void eeprom_debug_all_data(void)
{
  uint8_t i,j;
  uint8_t readData[255];
  AT24CXX_Read(0,readData,255);
  for(j = 0; j < 4; j++)
  {  
    DBG_INFO("%02x %02x %02x",readData[j*63],readData[j*63 + 1],readData[j*63 + 2]);
    DBG_INFO("\n");
    for(i = 3; i < 63; i++)
    {
      DBG_INFO("%02x ",readData[j*63 + i]);
      if((i - 2)%12 == 0 && i != 0)
        DBG_INFO("\n");
    }
   }  
   DBG_INFO("\n");
}

void at24cxx_init_error(void)
{
  DBG_ERROR("!!!!!!eeprom read err\n");
  hw_delay_ms(2000);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
} 










