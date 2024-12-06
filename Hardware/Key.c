#include "stm32f10x.h"                  // Device header

uint8_t Mode;			 //测量功能
uint8_t Range;			 //量程
extern uint16_t Num;	 //计时,1毫秒+1
extern uint8_t Sleepflag;//停机标志

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);		//开启AFIO的时钟，外部中断必须开启AFIO的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);							//将PB10和PB11引脚初始化为上拉输入
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource10);		//将外部中断的10号线映射到GPIOB，即选择PB10为外部中断引脚
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource11);		//将外部中断的11号线映射到GPIOB，即选择PB11为外部中断引脚
	
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line10 | EXTI_Line11;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//配置NVIC为分组2
														//即抢占优先级范围：0~3，相应优先级范围：0~3
														//此分组配置在整个工程中仅需调用一次
														//若有多个中断，可以把此代码放在main函数内，while循环之前
														//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	NVIC_InitTypeDef NVIC_InitStrcture;
	NVIC_InitStrcture.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStrcture.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStrcture.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStrcture.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStrcture);
}

//获取按键状态
uint8_t Key_GetState(void){
	if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) == 0){
		return 1;
	}
	if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0){
		return 2;
	}
	return 0;
}

//扫描按键
void Key_Tick(void){
	static uint8_t Count;
	static uint8_t CurrState,PrevState;
	
	Count++;
	if (Count >= 20){//消抖
		Count = 0;
		PrevState = CurrState;
		CurrState = Key_GetState();
		
		//判断按键变化状态
		if (CurrState == 0 && PrevState == 1 && !Sleepflag){//按键1抬起且不处于停机时
			Mode++;
			Mode %= 3;
		}
		else if (CurrState == 0 && PrevState == 2 && !Sleepflag){//按键2抬起且不处于停机时
			Range = !Range;
		}
	}
}

void EXTI15_10_IRQHandler(void){
		Num = 0;//重新计时
		EXTI_ClearITPendingBit(EXTI_Line10|EXTI_Line11);		//清除外部中断10|11号线的中断标志位
		
													//中断标志位必须清除
													//否则中断将连续不断地触发，导致主程序卡死
}
