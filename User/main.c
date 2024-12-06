#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "AD.h"
#include "Key.h"
#include "Timer.h"
#include <stdio.h>
#include "filters.h"

extern uint8_t Mode;	//测量功能
extern uint8_t Range;	//量程
float TrueValue;		//真实值
char OLEDString[10];	//OLED显示字符串
uint16_t Num;			//计时,1毫秒+1
uint8_t Sleepflag = 0;	//停机标志

int main(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//开启PWR的时钟
															//停止模式和待机模式一定要记得开启
	/*模块初始化*/
	OLED_Init();				//OLED初始化
	AD_Init();					//AD初始化
	Key_Init();					//Key初始化
	Timer_Init();				//定时器初始化
	Kalman_Filter_Init();		//卡尔曼滤波器初始化
	
	/*开机显示*/
	OLED_ShowString(2, 5, "Running");
	Delay_ms(2000);
	OLED_ShowString(2, 5, "       ");
	Delay_ms(500);
	
	while (1)
	{
		OLED_ShowString(1, 1, "Range:");
		
		//直流电压档
		if (Mode == 0){			
			OLED_ShowString(2, 1, "DC:");		
			if (Range){//小量程
				OLED_ShowString(1, 7, "0.2V   ");
				TrueValue = DC_Filter(&AD_Value[0]) / 4095.0 * 3.3 / 16;//计算真实值
				if (TrueValue >= 0.204){TrueValue = 0;}//空接时置零
				else if (TrueValue >= 0.2){TrueValue = 0.19999;} //超过最大值
				sprintf(OLEDString,"%.5fV ",TrueValue);
				OLED_ShowString(3, 1, OLEDString);
			}
			else {//大量程
				OLED_ShowString(1, 7, "2V     ");
				TrueValue = DC_Filter(&AD_Value[1]) / 4095.0 * 3.3;//计算真实值
				if (TrueValue >= 3.2){TrueValue = 0;}//空接时置零
				else if (TrueValue >= 2){TrueValue = 1.9999;} //超过最大值
				sprintf(OLEDString,"%.4fV  ",TrueValue);
				OLED_ShowString(3, 1, OLEDString);
			}
		}
		//交流电压档
		if (Mode == 1){
			OLED_ShowString(2, 1, "AC:");
			TrueValue = AC_Cal_Filter();
			if (Range){//小量程
				OLED_ShowString(1, 7, "0.2V   ");
				if (TrueValue >= 0.204){TrueValue = 0;}//空接时置零
				else if (TrueValue >= 0.2){TrueValue = 0.1999;}//超过最大值
				sprintf(OLEDString,"%.4fV  ",TrueValue);
				OLED_ShowString(3, 1, OLEDString);
				
			}
			else {//大量程
				OLED_ShowString(1, 7, "2V     ");
				if (TrueValue >= 3.2){TrueValue = 0;}//空接时置零
				else if (TrueValue >= 2){TrueValue = 1.999;}//超过最大值
				sprintf(OLEDString,"%.3fV   ",TrueValue);
				OLED_ShowString(3, 1, OLEDString);
				
			}	
		}
		//欧姆档
		if (Mode == 2){
			OLED_ShowString(2, 1, "R :");	
			if (Range){//小量程
				OLED_ShowString(1, 7, "200ohm ");
				TrueValue = 99.7 * R_Filter(&AD_Value[3]) / (4095.0 - R_Filter(&AD_Value[3]));		  //计算真实值
				if (TrueValue > 400000){TrueValue = 0;}//空接时置零
				else if (TrueValue >= 200){TrueValue = 199.9;}				  //超过最大值
				sprintf(OLEDString,"%.1fohm ",TrueValue);
				OLED_ShowString(3, 1, OLEDString);
			}
			else {//大量程
				OLED_ShowString(1, 7, "200kohm");
				TrueValue = 100 * R_Filter(&AD_Value[4]) / (4095.0 - R_Filter(&AD_Value[4]));		  //计算真实值
				if (TrueValue > 400000){TrueValue = 0;}//空接时置零
				else if (TrueValue >= 200){TrueValue = 199.9;}				  //超过最大值
				sprintf(OLEDString,"%.1fkohm",TrueValue);
				OLED_ShowString(3, 1, OLEDString);
			}	
		}
		
		//计时1分钟
		if (Num >= 60000){
			Sleepflag = 1;//停机标志
			
			/*进入停止模式提示*/
			OLED_Clear();
			OLED_ShowString(2, 5, "Sleeping");
			Delay_ms(2000);
			OLED_ShowString(2, 5, "        ");
			
			PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);	//STM32进入停止模式，并等待中断唤醒
			SystemInit();											//唤醒后，要重新配置时钟
			
			/*唤醒提示*/
			OLED_ShowString(2, 5, "Running");
			Delay_ms(2000);
			OLED_ShowString(2, 5, "       ");
			Delay_ms(500);
			
			Sleepflag = 0;//清除标志
		}
	}
}

//定时器中断函数，1毫秒进入1次
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)//是定时器2的中断
	{
		Num++;	   //计1毫秒
		Key_Tick();//扫描按键
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清除定时器2的中断标志位
	}
}
