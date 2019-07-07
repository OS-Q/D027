

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "includes.h"
#include "app_cfg.h"
#include "debug.h"
#include "delay.h"
#include "rc522.h"
#include "gpio_init.h"
#include "rc522_task.h"
#include "uart2_task.h"
#include "usart2.h"
#include "timer2.h"
#include "rtc_config.h"
#include "spi_flash.h"
#include "24cxx.h"
#include "open_result_task.h"
#include "led_task.h"
#include "flash_task.h"
#include "watch_dog.h"
#include <os_app_hooks.h>

uint32_t swVer=0x10;

static OS_TCB   StartUp_TCB;
static CPU_STK 	StartUp_stk[STARTUP_TASK_STK_SIZE];

extern void SoftReset(void);

/* �������������������������� */
static void StartUp_Task(void *p_arg)
{
	OS_ERR err;
	(void)p_arg;
	
	#if (OS_CFG_STAT_TASK_EN > 0) 
		OSStatTaskCPUUsageInit(&err); 		/* Determine CPU capacity. */ 
	#endif
	
	#if (OS_CFG_APP_HOOKS_EN > 0)
		App_OS_SetAllHooks();
	#endif
	
	if(spiflash_init() != 0)
  {
    flash_init_error();
    SoftReset();
  }
  
  if(AT24CXX_Init() != 0)
  {
    at24cxx_init_error();
    SoftReset();
  } 
	
	/* ��������	PB1��˸	*/
	OSTaskCreate((OS_TCB     *)&LED_TASK_TCB,       /* ������ƿ�ָ��	*/   
				 (CPU_CHAR   *)"LED����",									/* �������ƣ��������д������*/ 
				 (OS_TASK_PTR ) Led_Task,									/* �������ָ��	*/ 
				 (void       *) 0,											  /* ���ݸ�����Ĳ���parg	*/ 
				 (OS_PRIO     ) LED_TASK_PRIO,						/* �������ȼ�	*/ 
				 (CPU_STK    *)&LED_TASK_stk[0],					/* �����ջ����ַ	*/ 
				 (CPU_STK_SIZE) LED_TASK_STK_SIZE / 10,	  /* ��ջʣ�ྯ����	*/ 
				 (CPU_STK_SIZE) LED_TASK_STK_SIZE,				/* ��ջ��С	*/ 
				 (OS_MSG_QTY  ) 0u,											  /* �ɽ��յ������Ϣ������	 */ 
				 (OS_TICK     ) 0u,											  /* ʱ��Ƭ��תʱ��	*/ 
				 (void       *) 0,											  /* ������ƿ���չ��Ϣ	*/ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* ����ѡ�� */ 
				 (OS_ERR     *)&err);										  /* ����ֵ����/��*/ 

	/* ��������	����2�������ݴ��� */
	OSTaskCreate((OS_TCB     *)&UART2_TASK_TCB,     /* ������ƿ�ָ��*/   
				 (CPU_CHAR   *)"UART2����",								/* �������ƣ��������д������*/ 
				 (OS_TASK_PTR ) UART2_Task,							  /* �������ָ��				 */ 
				 (void       *) 0,											  /* ���ݸ�����Ĳ���parg		 */ 
				 (OS_PRIO     ) UART2_TASK_PRIO,					/* �������ȼ�				 */ 
				 (CPU_STK    *)&UART2_TASK_stk[0],				/* �����ջ����ַ			 */ 
				 (CPU_STK_SIZE) UART2_TASK_STK_SIZE / 10,			/* ��ջʣ�ྯ����			 */ 
				 (CPU_STK_SIZE) UART2_TASK_STK_SIZE,					/* ��ջ��С					 */ 
				 (OS_MSG_QTY  ) 0u,											  /* �ɽ��յ������Ϣ������	 */ 
				 (OS_TICK     ) 0u,											  /* ʱ��Ƭ��תʱ��			 */ 
				 (void       *) 0,											  /* ������ƿ���չ��Ϣ		 */ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* ����ѡ��*/ 
				 (OS_ERR     *)&err);										  /* ����ֵ����/��			 */ 

  	/* ��������	��������*/
	OSTaskCreate((OS_TCB     *)&RC522_TASK_TCB,     /* ������ƿ�ָ��*/   
				 (CPU_CHAR   *)"Read card Task",					/* �������ƣ��������д������*/ 
				 (OS_TASK_PTR ) Rc522_Task,							  /* �������ָ��				 */ 
				 (void       *) 0,											  /* ���ݸ�����Ĳ���parg		 */ 
				 (OS_PRIO     ) RC522_TASK_PRIO,					/* �������ȼ�				 */ 
				 (CPU_STK    *)&RC522_TASK_stk[0],				/* �����ջ����ַ			 */ 
				 (CPU_STK_SIZE) RC522_TASK_STK_SIZE / 10,			/* ��ջʣ�ྯ����			 */ 
				 (CPU_STK_SIZE) RC522_TASK_STK_SIZE,					/* ��ջ��С					 */ 
				 (OS_MSG_QTY  ) 0u,											  /* �ɽ��յ������Ϣ������	 */ 
				 (OS_TICK     ) 0u,											  /* ʱ��Ƭ��תʱ��			 */ 
				 (void       *) 0,											  /* ������ƿ���չ��Ϣ		 */ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* ����ѡ��*/ 
				 (OS_ERR     *)&err);										  /* ����ֵ����/��			 */ 
  
  /* �������� */
	OSTaskCreate((OS_TCB     *)&OPEN_RESULT_TASK_TCB,     /* ������ƿ�ָ��*/   
				 (CPU_CHAR   *)"Open Result Task",					/* �������ƣ��������д������*/ 
				 (OS_TASK_PTR ) OPEN_RESULT_Task,							  /* �������ָ��				 */ 
				 (void       *) 0,											  /* ���ݸ�����Ĳ���parg		 */ 
				 (OS_PRIO     ) OPEN_RESULT_TASK_PRIO,					/* �������ȼ�				 */ 
				 (CPU_STK    *)&OPEN_RESULT_TASK_stk[0],				/* �����ջ����ַ			 */ 
				 (CPU_STK_SIZE) OPEN_RESULT_TASK_STK_SIZE / 10,			/* ��ջʣ�ྯ����			 */ 
				 (CPU_STK_SIZE) OPEN_RESULT_TASK_STK_SIZE,					/* ��ջ��С					 */ 
				 (OS_MSG_QTY  ) 0u,											  /* �ɽ��յ������Ϣ������	 */ 
				 (OS_TICK     ) 0u,											  /* ʱ��Ƭ��תʱ��			 */ 
				 (void       *) 0,											  /* ������ƿ���չ��Ϣ		 */ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* ����ѡ��*/ 
				 (OS_ERR     *)&err);										  /* ����ֵ����/��			 */ 
         
           
	OSTaskCreate((OS_TCB     *)&FLASH_TASK_TCB,    
				 (CPU_CHAR   *)"Flash Task",					
				 (OS_TASK_PTR ) Flash_Task,							 
				 (void       *) 0,											  
				 (OS_PRIO     ) FLASH_TASK_PRIO,					
				 (CPU_STK    *)&FLASH_TASK_stk[0],				
				 (CPU_STK_SIZE) FLASH_TASK_STK_SIZE / 10,			
				 (CPU_STK_SIZE) FLASH_TASK_STK_SIZE,					
				 (OS_MSG_QTY  ) 0u,											  
				 (OS_TICK     ) 0u,											  
				 (void       *) 0,											  
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	
				 (OS_ERR     *)&err);

	OSTaskCreate((OS_TCB     *)&WDOG_TASK_TCB,    
				 (CPU_CHAR   *)"watchDog Task",					
				 (OS_TASK_PTR ) WDOG_Task,							 
				 (void       *) 0,											  
				 (OS_PRIO     ) WDOG_TASK_PRIO,					
				 (CPU_STK    *)&WDOG_TASK_stk[0],				
				 (CPU_STK_SIZE) WDOG_TASK_STK_SIZE / 10,			
				 (CPU_STK_SIZE) WDOG_TASK_STK_SIZE,					
				 (OS_MSG_QTY  ) 0u,											  
				 (OS_TICK     ) 0u,											  
				 (void       *) 0,											  
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	
				 (OS_ERR     *)&err);	
  
	/* ɾ����������	*/
	OSTaskDel(&StartUp_TCB,&err);
}

int main(void)
{
	OS_ERR err;
  
  /* gpio ��ʼ��*/
  gpio_init();
  
	/* LEDӲ����ʼ��*/
	LED_Init();
	
	/* ����1Ӳ����ʼ��*/
	USART1_Init(115200);
  USART2_Init(115200);
  DBG_ERROR("\nsoftware version=%x.%x\n",swVer>>4,swVer&0x0F);
  
  /* time2 */
  //TIM2_Int_Init(9999,7199);   ((1+7199 )/72M)*(1+9999)=1
  //TIM2_Int_Init(369,71); //2700HZ
	//TIM2_Int_Init(249,71);   //4000HZ
  TIM2_Int_Init(487,71);   //2048HZ
  beep_start(1250);
  
  /* rtc config */
  if(rtc_config_init() != 0)
  {
    rtc_init_error();
    goto out;
  }
  
  //RC522 ������ʼ��
  rc522_init();
    
	/* ����ʱ�ӳ�ʼ��*/
	OS_CPU_SysTickInit();
	
	/* ����ϵͳ��ʼ�����˴��Ѵ�����������*/
	OSInit(&err);
  
	/* �����ٽ��� */
	OS_CRITICAL_ENTER();
	
	/* ������ʼ����*/
	OSTaskCreate((OS_TCB     *)&StartUp_TCB,          /* ������ƿ�ָ��	*/   
				 (CPU_CHAR   *)"��ʼ����",									/* �������ƣ��������д������*/ 
				 (OS_TASK_PTR ) StartUp_Task,								/* �������ָ��	*/ 
				 (void       *) 0,											    /* ���ݸ�����Ĳ���parg	*/ 
				 (OS_PRIO     ) STARTUP_TASK_PRIO,					/* �������ȼ�	*/ 
				 (CPU_STK    *)&StartUp_stk[0],							/* �����ջ����ַ */ 
				 (CPU_STK_SIZE) STARTUP_TASK_STK_SIZE / 10,	/* ��ջʣ�ྯ����	*/ 
				 (CPU_STK_SIZE) STARTUP_TASK_STK_SIZE,			/* ��ջ��С	*/ 
				 (OS_MSG_QTY  ) 0u,											    /* �ɽ��յ������Ϣ������*/ 
				 (OS_TICK     ) 0u,											    /* ʱ��Ƭ��תʱ��	*/ 
				 (void       *) 0,											    /* ������ƿ���չ��Ϣ*/ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* ����ѡ��*/ 
				 (OS_ERR     *)&err);										    /* ����ֵ����/��	*/ 
		
	/* �˳��ٽ��� */
	OS_CRITICAL_EXIT();
				 
	/* ����ϵͳ��ʼ��ʼ���� */
	OSStart(&err);

out:
  SoftReset();
	return 0;
}
