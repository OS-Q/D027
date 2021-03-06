#include "spi_flash.h"
#include "string.h"
#include "timer2.h"
#include "delay.h"
#include "app_cfg.h"
#include "flash_task.h"

uint16_t SPI_FLASH_TYPE;//�� flash оƬ�ͺţ�

void spi_gpio_write_byte(uint8_t dat)
{
	uint8_t i;
	
	CLR_SPI_FLASH_SCK;
	for (i=0; i<8; i++)
	{
		if ((dat&0x80)==0x80)	 
		{			
			SET_SPI_FLASH_MOSI;
		}
		else
		{
			CLR_SPI_FLASH_MOSI;
		}
		
		SET_SPI_FLASH_SCK;		 
		dat <<=1;
		CLR_SPI_FLASH_SCK;		 
	}
}

uint8_t spi_gpio_read_byte(void)
{
	uint8_t i, in=0;
	
	CLR_SPI_FLASH_SCK;
	for (i=0; i<8; i++)
	{
		in <<= 1;
		SET_SPI_FLASH_SCK;
		if (GET_SPI_FLASH_MISO==1)			 
			in |=1;		 
		CLR_SPI_FLASH_SCK;
	}
	
	return in;
}

uint8_t SPI_Flash_ReadSR(void)   
{  
	uint8_t byte=0;
	
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_ReadStatusReg);//���Ͷ�ȡ״̬�Ĵ�������    
	byte=spi_gpio_read_byte();  
	SET_SPI_FLASH_CS;
	
	return byte;   
} 

void SPI_Flash_Wait_Busy(void)   
{   
	while ((SPI_Flash_ReadSR()&0x01)==0x01);   // �ȴ�BUSYλ���
}

void SPI_Flash_PowerDown(void)   
{ 
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_PowerDown);//���͵�������  
	SET_SPI_FLASH_CS;	      
}   

void SPI_Flash_WAKEUP(void)   
{  
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_ReleasePowerDown);   
	SET_SPI_FLASH_CS; 
}

void SPI_FLASH_Write_Enable(void)   
{
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_WriteEnable);//����дʹ��  
	SET_SPI_FLASH_CS;

	while ((SPI_Flash_ReadSR()&0x02)!=0x02); 
} 
 
void SPI_FLASH_Write_Disable(void)   
{  
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_WriteDisable);//����д��ָֹ��    
	SET_SPI_FLASH_CS;

	while ((SPI_Flash_ReadSR()&0x02)==0x02); 
}

void SPI_FLASH_Write_SR(uint8_t sr)   
{
	SPI_FLASH_Write_Enable();
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_WriteStatusReg);//����дȡ״̬�Ĵ�������    
	spi_gpio_write_byte(sr);
	SET_SPI_FLASH_CS;

	SPI_Flash_Wait_Busy();
	SPI_FLASH_Write_Disable();
}  

uint16_t SPI_Flash_ReadID(void)
{
	uint16_t tmp = 0;
	
	CLR_SPI_FLASH_CS;				    
	spi_gpio_write_byte(W25X_ManufactDeviceID);//���Ͷ�ȡID����	    
	spi_gpio_write_byte(0x00); 	    
	spi_gpio_write_byte(0x00); 	    
	spi_gpio_write_byte(0x00); 	 			   
	tmp |=spi_gpio_read_byte()<<8;  
	tmp |=spi_gpio_read_byte();	 
	SET_SPI_FLASH_CS;
	
	return tmp;
}

uint8_t devUniqueID[8];
void SPI_Flash_Read_Unique_ID(void)
{
	uint8_t i;
	
	CLR_SPI_FLASH_CS;				    
	spi_gpio_write_byte(W25X_Unique_ID);	    
	spi_gpio_write_byte(0x00);
	spi_gpio_write_byte(0x00);
	spi_gpio_write_byte(0x00);
	spi_gpio_write_byte(0x00);
	
	for(i=0;i<8;i++)
		devUniqueID[i]=spi_gpio_read_byte();

	SET_SPI_FLASH_CS;
	DBG_INFO("devID=");
	for(i=0;i<8;i++)
		DBG_INFO("%02X",devUniqueID[i]);
	DBG_INFO("\n");
}

void spiflash_read(uint32_t dest,void *buf,uint16_t len)
{
	uint8_t *p=buf;
	uint16_t i=0;

	if(p == NULL)
		return;

	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_ReadData); //���Ͷ�ȡ����   
	spi_gpio_write_byte((dest>>16)&0xFF);  
	spi_gpio_write_byte((dest>>8)&0xFF);   
	spi_gpio_write_byte(dest&0xFF);   
	for(i=0;i<len;i++)
	{ 
		p[i]=spi_gpio_read_byte();   //ѭ����
	}
	SET_SPI_FLASH_CS;
}

void spiflash_write(uint32_t dest,void *buf, uint16_t len)
{
	uint8_t *p=buf;
	uint16_t i=0,page,j,lastlen,wrlen;
	uint32_t destL;

	if(p == NULL)
		return;
	if((SPI_FLASH_TYPE&0xFF00)==0XBF00)
	{
		for(i=0;i<len;)
		{
			SPI_FLASH_Write_Enable();
			CLR_SPI_FLASH_CS; 
			spi_gpio_write_byte(W25X_PageProgram_AAI); //W25X_PageProgram
			spi_gpio_write_byte((dest>>16)&0xFF);
			spi_gpio_write_byte((dest>>8)&0xFF);
			spi_gpio_write_byte(dest&0xFF);
			spi_gpio_write_byte(p[i++]);
			dest++;
			if(i<len)
			{
				spi_gpio_write_byte(p[i++]);
				dest++;
			}
			SET_SPI_FLASH_CS;

			SPI_Flash_Wait_Busy(); 
			SPI_FLASH_Write_Disable();
		}
	}
	else
	{
		page=len/256;
		lastlen=len%256;
		if(lastlen>0)
			page++;

		for(j=0;j<page;j++)
		{
			SPI_FLASH_Write_Enable();
			wrlen=256;
			if(j==(page-1))
			{
				if(lastlen>0)
					wrlen=lastlen;
			}
			destL=dest+j*256;
			
			CLR_SPI_FLASH_CS;
			spi_gpio_write_byte(W25X_PageProgram);      //����д����   
			spi_gpio_write_byte((destL>>16)&0xFF);  
			spi_gpio_write_byte((destL>>8)&0xFF);   
			spi_gpio_write_byte(destL&0xFF);   
			for(i=0;i<wrlen;i++)
				spi_gpio_write_byte(p[i+j*256]);//ѭ��д��  
			SET_SPI_FLASH_CS;
			SPI_Flash_Wait_Busy();
			SPI_FLASH_Write_Disable();
		}
	}
}

//����һ������
//Dst_Addr:������ַ 0~511 for w25x16
//����һ��ɽ��������ʱ��:150ms
void spiflash_sector4k_erase(uint32_t dest) 
{
	dest *= 4096;
  SPI_FLASH_Write_Enable();
	
	SPI_Flash_Wait_Busy();   
	CLR_SPI_FLASH_CS;  
	spi_gpio_write_byte(W25X_SectorErase);      //������������ָ�� 
	spi_gpio_write_byte((dest>>16)&0xFF);
	spi_gpio_write_byte((dest>>8)&0xFF);   
	spi_gpio_write_byte(dest&0xFF);  
	SET_SPI_FLASH_CS;     
	SPI_Flash_Wait_Busy();   				   //�ȴ��������
	
	SPI_FLASH_Write_Disable();
}

void spiflash_sector64k_erase(uint32_t dest)
{
	SPI_FLASH_Write_Enable();
	
	SPI_Flash_Wait_Busy();   
	CLR_SPI_FLASH_CS;  
	spi_gpio_write_byte(W25X_BlockErase);      //������������ָ�� 
	spi_gpio_write_byte((dest>>16)&0xFF);
	spi_gpio_write_byte((dest>>8)&0xFF);   
	spi_gpio_write_byte(dest&0xFF);  
	SET_SPI_FLASH_CS;     
	SPI_Flash_Wait_Busy();   				   //�ȴ��������
	
	SPI_FLASH_Write_Disable();
}

void spiflash_sector_all_erase(void)
{
	SPI_FLASH_Write_Enable();
	SPI_Flash_Wait_Busy();  
	
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_ChipErase);// ��������  
	SET_SPI_FLASH_CS;      
	SPI_Flash_Wait_Busy();   				   //�ȴ���������
	
	SPI_FLASH_Write_Disable();
}

//SPI��һҳ(0~65535)��д������256���ֽڵ�����,��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��;WriteAddr:��ʼд��ĵ�ַ(24bit);NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
  SPI_FLASH_Write_Enable();                  //SET WEL 
	CLR_SPI_FLASH_CS;                          //ʹ������   
  spi_gpio_write_byte(W25X_PageProgram);     //����дҳ����   
  spi_gpio_write_byte((u8)((WriteAddr)>>16)); //����24bit��ַ    
  spi_gpio_write_byte((u8)((WriteAddr)>>8));   
  spi_gpio_write_byte((u8)WriteAddr);   
  for(i=0;i<NumByteToWrite;i++)
    spi_gpio_write_byte(pBuffer[i]);//ѭ��д��  
	SET_SPI_FLASH_CS;                 //ȡ��Ƭѡ 
	SPI_Flash_Wait_Busy();					   //�ȴ�д�����
  SPI_FLASH_Write_Disable();
} 

//�޼���дSPI FLASH,��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��,WriteAddr:��ʼд��ĵ�ַ(24bit),NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite<=pageremain)
    pageremain=NumByteToWrite;//������256���ֽ�
	while(1)
	{	   
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)
      break;
	 	else
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			//��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)
        pageremain=256;               //һ�ο���д��256���ֽ�
			else 
        pageremain=NumByteToWrite; 	  //����256���ֽ���
		}
	}	    
} 

//��ȡSPI FLASH ,��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��,ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit),NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;    												    
	CLR_SPI_FLASH_CS;                           //ʹ������   
  spi_gpio_write_byte(W25X_ReadData);        //���Ͷ�ȡ����   
  spi_gpio_write_byte((u8)((ReadAddr)>>16)); //����24bit��ַ    
  spi_gpio_write_byte((u8)((ReadAddr)>>8));   
  spi_gpio_write_byte((u8)ReadAddr);   
  for(i=0;i<NumByteToRead;i++)
	{ 
    pBuffer[i]=spi_gpio_read_byte();   //ѭ������  
  }
	SET_SPI_FLASH_CS;                    //ȡ��Ƭѡ
}  
 
//��ָ����ַ��ʼд��ָ�����ȵ�����,�ú�������������!
//pBuffer:���ݴ洢��,WriteAddr:��ʼд��ĵ�ַ(24bit),NumByteToWrite:Ҫд����ֽ���(���65535)
u8 SPI_FLASH_BUF[4096];
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff,secremain,i;
	 
	secpos=WriteAddr/4096;//������ַ 0~511 for w25x16
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С   
	
	if(NumByteToWrite<=secremain)
    secremain=NumByteToWrite;//������4096���ֽ�
  
	while(1) 
	{
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//������������������
		 
		for(i=0;i<secremain;i++)//У������
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)
        break;//��Ҫ����  	  
		}
    
		if(i<secremain)//��Ҫ����
		{
			spiflash_sector4k_erase(secpos);//�����������
			for(i=0;i<secremain;i++)	   //����
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//д����������  
		}else{
      SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������.
    }
    
		if(NumByteToWrite==secremain)
    {
      //д�������
      break;
    } else {
      //д��δ����
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 
		  pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		  NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)
        secremain=4096;	//��һ����������д����
			else 
        secremain=NumByteToWrite;			//��һ����������д����
		}
	} 	 
}

void SPI_Flash_Write_Str(flash_save_container* pBuffer,u8 length)   
{ 
	u32 secpos;
	u16 i,j;
  
	secpos=pBuffer[0].loc/4096;//������ַ 0~511 for w25x16
	DBG_INFO("5 flash write data\n");
	SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//������������������
  spiflash_sector4k_erase(secpos);//�����������
  DBG_INFO("6 flash write data t\n");
  for(j = 0; j < length; j++)
  {
    DBG_INFO("5 flash write length = %d, loc = %d, data_len = %d\n",j,pBuffer[j].loc,pBuffer[j].data_len);
    for(i = 0; i < pBuffer[j].data_len; i++)	   //����
    {
      SPI_FLASH_BUF[i+pBuffer[j].loc]=pBuffer[j].data_point[i];
      //DBG_INFO("5 flash write  = %d,  = %d\n",i+pBuffer[j].loc,pBuffer[j].data_point[i]);
    }
  }
  DBG_INFO("7 flash write data\n");
  SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//д����������   
  DBG_INFO("8 flash write data\n");
}

int Flash_Check(void)
{
	uint32_t writeData = FLASH_MAGIC_FLAG,readData;
  uint32_t address=1023*FLASH_BLK_SIZE;
	OS_ERR err;
  SPI_Flash_Read((uint8_t *)&readData,address,4);	
	if(readData==FLASH_MAGIC_FLAG)
    return 0;		   
	else//�ų���һ�γ�ʼ�������
	{
		reset_flash();
		OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
		SPI_Flash_Write((uint8_t *)&writeData,address,4);
		OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
		SPI_Flash_Read((uint8_t *)&readData,address,4);  
		if(readData==FLASH_MAGIC_FLAG)
      return 0;
	}
	return -1;											  
}

int spiflash_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	CLR_SPI_FLASH_CS;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	SET_SPI_FLASH_CS;
	SET_SPI_FLASH_SCK;
	SET_SPI_FLASH_MOSI;
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	SPI_FLASH_Write_SR(0x00);
	
	SPI_FLASH_TYPE=SPI_Flash_ReadID();//��ȡFLASH ID. 
	if(((SPI_FLASH_TYPE&0xFF00)==0xFF00)||((SPI_FLASH_TYPE&0xFF00)==0x0000))
  {  
		DBG_ERROR("!!!!!!SPI_FLASH read err\n");
    return -1;
  }  
  
  DBG_INFO("Flash ID:%04X\n",SPI_FLASH_TYPE);
  return Flash_Check();
}

void flash_init_error(void)
{
  DBG_ERROR("!!!!!!flash read err\n");
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
} 
