#include "timer2.h"

//ͨ�ö�ʱ��2�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��2
static volatile uint16_t beep_times;

// ���㹫ʽ��
void TIM2_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;				//��ʱ�����ýṹ�嶨��
	NVIC_InitTypeDef NVIC_InitStructure;						      //�ж����ýṹ�嶨��

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  //ʱ��ʹ��
	
	//��ʱ��TIM2��ʼ��: ��ʱ��Ƶ��((1+TIM_Prescaler )/72M)*(1+TIM_Period )
	TIM_TimeBaseStructure.TIM_Period = arr;    //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;  //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;      //����ʱ�ӷָ�:TDTS = Tck_tim	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM2�ж�,��������ж�	TIM_IT_Update

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���

	//TIM_Cmd(TIM2, ENABLE);  //ʹ��TIM2			 
}

void beep_start(uint16_t times)
{
  beep_times = times;
  TIM_Cmd(TIM2, ENABLE);
}


//��ʱ��2�жϷ������
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���TIM2�����жϷ������
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );   //���TIM2�����жϱ�־ 
		GPIOB->ODR ^= GPIO_Pin_4;
	}
  
  if(beep_times-- <= 0)
  {
    TIM_Cmd(TIM2, DISABLE);
    GPIO_ResetBits(GPIOB,GPIO_Pin_4);
  }  
}












