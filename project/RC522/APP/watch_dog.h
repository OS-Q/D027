#ifndef  __WATCH_DOG_H__
#define  __WATCH_DOG_H__

/* �������ȼ� */
#define WDOG_TASK_PRIO 	  4

/* ��ջ��С��������žֲ��������Ĵ���ֵ���ж��еı����� */
#define WDOG_TASK_STK_SIZE 	    80

extern OS_TCB   WDOG_TASK_TCB;
extern CPU_STK  WDOG_TASK_stk[WDOG_TASK_STK_SIZE];

void WDOG_Task(void *p_arg);

#endif
