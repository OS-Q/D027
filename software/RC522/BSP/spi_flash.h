#ifndef SPI_FLASH_H
#define SPI_FLASH_H
#include "debug.h"
#include "stm32f10x.h"
#include "flash_task.h"

#define W25Q80 	0XEF13 // 1M BYTES FLASH
#define W25Q16 	0XEF14 // 2M BYTES FLASH
#define W25Q32 	0XEF15 // 4M BYTES FLASH
#define W25Q64 	0XEF16 // 8M BYTES FLASH
 // 4KbytesΪ1 sector
// 16 sectors Ϊ1 block
// ����W25Q64 ����Ϊ8M �ֽڣ���8*1024*1024/(4*1024)=2048 sectors����8*1024*1024/(16*4*1024)=128 blocks ,

//ָ���
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F
#define W25X_Unique_ID			0x4B
#define W25X_PageProgram_AAI	0xAD 

#define SET_SPI_FLASH_CS		GPIO_SetBits(GPIOA,GPIO_Pin_8)
#define CLR_SPI_FLASH_CS		GPIO_ResetBits(GPIOA,GPIO_Pin_8)

#define SET_SPI_FLASH_SCK		GPIO_SetBits(GPIOB,GPIO_Pin_13)
#define CLR_SPI_FLASH_SCK		GPIO_ResetBits(GPIOB,GPIO_Pin_13)

#define SET_SPI_FLASH_MOSI		GPIO_SetBits(GPIOB,GPIO_Pin_15)
#define CLR_SPI_FLASH_MOSI		GPIO_ResetBits(GPIOB,GPIO_Pin_15)

#define GET_SPI_FLASH_MISO		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)

int spiflash_init(void);
void spiflash_sector4k_erase(uint32_t dest);/*  4K ����;  dest < оƬ����ֽ��� ��������4*1024����������
����ʹ��8M �ֽ�оƬ  ��dest<(8*1024*1024) */
void spiflash_sector64k_erase(uint32_t dest);/* 64K����; dest < оƬ����ֽ��� ��������64*1024��������*/
void spiflash_sector_all_erase(void);          /*�������*/
void spiflash_read(uint32_t dest,void *buf,uint16_t len);  /*��FLASH ;  dest+len < оƬ����ֽ���*/
void spiflash_write(uint32_t dest,void *buf, uint16_t len); /*дFLASH  ; dest+len < оƬ����ֽ���*/
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);
void SPI_Flash_Write_Str(flash_save_container* pBuffer,u8 length);
void flash_init_error(void);

#endif 

