#ifndef  __LED_TASK_H__
#define  __LED_TASK_H__

/* �������ȼ� */
#define LED_TASK_PRIO 	  4

/* ��ջ��С��������žֲ��������Ĵ���ֵ���ж��еı����� */
#define LED_TASK_STK_SIZE 	  80

extern OS_TCB   LED_TASK_TCB;
extern CPU_STK  LED_TASK_stk[LED_TASK_STK_SIZE];

/* Modes */
#define HAL_LED_MODE_OFF          0x00
#define HAL_LED_MODE_ON           0x01
#define HAL_LED_MODE_FLASH_SLOW   0x02
#define HAL_LED_MODE_FLASH_FAST   0x04

void LED_Init(void);
void Led_Task(void *p_arg);
void HalLedSet(uint8_t mode);

#endif
