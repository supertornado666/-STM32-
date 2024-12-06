#include "filters.h"
#include "AD.h"
#include "math.h"
#include "Delay.h"
float Filt_Value,m;
extern uint8_t Range;
float  u1=0,u2=0,ur=0;
int n=0;
/////////////////////////////////////////////////////////////////////
///*Filter0:Kalman Filter*/
//
/////////////////////////////////////////////////////////////////////
struct 
{
    float LastP;    //??????? ?????0.02
    float Now_P;    //??????? ?????0
    float out;      //???????? ?????0
    float Kg;       //????? ?????0
    float Q;        //??????? ?????0.001
    float R;        //??????? ?????0.543
}KalmanInfo;        //Kalman Filter parameter

void Kalman_Filter_Init(void)
{
    KalmanInfo.LastP = 1;
    KalmanInfo.Now_P = 0.0;
    KalmanInfo.out = 0.0;
    KalmanInfo.Kg = 0.0;
    KalmanInfo.Q = 0.003;
    KalmanInfo.R = 0.03;
    return;
}

float Kalman_Filter_Fun(uint16_t *pValue){
	//???????:k????????? = k-1???????? + ???????
    KalmanInfo.Now_P = KalmanInfo.LastP + KalmanInfo.Q;
    //???????:????? = k????????? / (k????????? + ???????)
    KalmanInfo.Kg = KalmanInfo.Now_P / (KalmanInfo.Now_P + KalmanInfo.R);
    //???????:k?????????? = ???????? + ????? * (??? - ????????)
    KalmanInfo.out = KalmanInfo.out + KalmanInfo.Kg * (*pValue - KalmanInfo.out);//??????????????????
    //???????: ?????????? info->LastP ?????????
    KalmanInfo.LastP = (1 - KalmanInfo.Kg) * KalmanInfo.Now_P;
    return KalmanInfo.out;
}


/*//////////////////////////////////////////////////////////////////////////
方法四：递推平均滤波法（又称滑动平均滤波法）
方法： 把连续取得的N个采样值看成一个队列，队列的长度固定为N，
       每次采样到一个新数据放入队尾，并扔掉原来队首的一次数据（先进先出原则），
       把队列中的N个数据进行算术平均运算，获得新的滤波结果。
       N值的选取：流量，N=12；压力，N=4；液面，N=4-12；温度，N=1-4。
优点：对周期性干扰有良好的抑制作用，平滑度高；
      适用于高频振荡的系统。
缺点：灵敏度低，对偶然出现的脉冲性干扰的抑制作用较差；
      不易消除由于脉冲干扰所引起的采样值偏差；
      不适用于脉冲干扰比较严重的场合；
      比较浪费RAM。
//////////////////////////////////////////////////////////////////////////*/

#define FILTER4_N 1000
uint16_t filter_buf[FILTER4_N + 1];

float filter4(uint16_t *pValue) {
  int filter_sum = 0;
  filter_buf[FILTER4_N] = *pValue;
  for (int i = 0; i < FILTER4_N; i++) {
    filter_buf[i] = filter_buf[i + 1]; // 所有数据左移，低位仍掉
    filter_sum += filter_buf[i];
  }
  
  return (float)filter_sum / FILTER4_N;
}


/*//////////////////////////////////////////////////////////////////////////
方法五：中位值平均滤波法（又称防脉冲干扰平均滤波法）
方法： 采一组队列去掉最大值和最小值后取平均值，     （N值的选取：3-14）。 
      相当于“中位值滤波法”+“算术平均滤波法”。
      连续采样N个数据，去掉一个最大值和一个最小值，
      然后计算N-2个数据的算术平均值。    
优点： 融合了“中位值滤波法”+“算术平均滤波法”两种滤波法的优点。
       对于偶然出现的脉冲性干扰，可消除由其所引起的采样值偏差。
       对周期干扰有良好的抑制作用。
       平滑度高，适于高频振荡的系统。
缺点：对于测量速度较慢或要求数据计算速度较快的实时控制不适用，比较浪费RAM。
//////////////////////////////////////////////////////////////////////////*/
#define N5 100

float filter5(uint16_t *pValue) {
  int filter_temp, filter_sum = 0;
  uint16_t filter_buf[N5];
  for(int i = 0; i < N5; i++) {
    filter_buf[i] = *pValue;	
  }
  // 采样值从小到大排列（冒泡法）
  for(int j = 0; j < N5 - 1; j++) {
    for(int i = 0; i < N5 - 1 - j; i++) {
      if(filter_buf[i] > filter_buf[i + 1]) {
        filter_temp = filter_buf[i];
        filter_buf[i] = filter_buf[i + 1];
        filter_buf[i + 1] = filter_temp;
      }
    }
  }
  // 去除最大最小极值后求平均
  for(int i = 1; i < N5 - 1; i++) filter_sum += filter_buf[i];
  
  return (float)filter_sum / (N5 - 2);
}



float DC_Filter(uint16_t *pValue){
//	Filt_Value = 0;
//	for (int i = 0; i < 10000; i++){
//		Filt_Value += Kalman_Filter_Fun(pValue);
//	}
//	Filt_Value /= 10000;

	
//	Filt_Value = 0;
//	for (int i = 0; i < 500; i++){
//		Filt_Value += filter4(pValue);
//	}
//	Filt_Value /= 500;


	Filt_Value = 0;
	for (int i = 0; i < 200; i++){
		Filt_Value += filter5(pValue);
	}
	Filt_Value /= 200;
	
	return Filt_Value;
}

float AC_Cal_Filter(void){
    Filt_Value=0;
	if (Range)
{
		for(int i=0;i<10;i++)
	{
		for(int j=0;j<15;j++)
		{
			u1=(AD_Value[2]*1.3156)/4095-0.509;
			u2+=u1*u1;
			Delay_ms(2);
	    }
		ur=sqrt(u2/15);
		u2=0;
		Filt_Value+=ur;
		
	}
} 
	else
{
for(int i=0;i<10;i++)
	{
		for(int j=0;j<15;j++)
		{
			u1=(AD_Value[2]*13.156)/4095-5.09;
			u2+=(u1*u1);
         Delay_ms(2);			
	    }
	ur=sqrt(u2/15);
		u2=0;	
		Filt_Value+=ur;
		
	}		
}
	Filt_Value/=10;

	return Filt_Value;

}
float R_Filter(uint16_t *pValue){
//	Filt_Value = 0;
//	for (int i = 0; i < 5000; i++){
//		Filt_Value += Kalman_Filter_Fun(pValue);
//	}
//	Filt_Value /= 10000;

	
//	Filt_Value = 0;
//	for (int i = 0; i < 250; i++){
//		Filt_Value += filter4(pValue);
//	}
//	Filt_Value /= 250;


	Filt_Value = 0;
	for (int i = 0; i < 100; i++){
		Filt_Value += filter5(pValue);
	}
	Filt_Value /= 100;
	
	return Filt_Value;
}
