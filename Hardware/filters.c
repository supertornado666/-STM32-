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
�����ģ�����ƽ���˲������ֳƻ���ƽ���˲�����
������ ������ȡ�õ�N������ֵ����һ�����У����еĳ��ȹ̶�ΪN��
       ÿ�β�����һ�������ݷ����β�����ӵ�ԭ�����׵�һ�����ݣ��Ƚ��ȳ�ԭ�򣩣�
       �Ѷ����е�N�����ݽ�������ƽ�����㣬����µ��˲������
       Nֵ��ѡȡ��������N=12��ѹ����N=4��Һ�棬N=4-12���¶ȣ�N=1-4��
�ŵ㣺�������Ը��������õ��������ã�ƽ���ȸߣ�
      �����ڸ�Ƶ�񵴵�ϵͳ��
ȱ�㣺�����ȵͣ���żȻ���ֵ������Ը��ŵ��������ýϲ
      �������������������������Ĳ���ֵƫ�
      ��������������űȽ����صĳ��ϣ�
      �Ƚ��˷�RAM��
//////////////////////////////////////////////////////////////////////////*/

#define FILTER4_N 1000
uint16_t filter_buf[FILTER4_N + 1];

float filter4(uint16_t *pValue) {
  int filter_sum = 0;
  filter_buf[FILTER4_N] = *pValue;
  for (int i = 0; i < FILTER4_N; i++) {
    filter_buf[i] = filter_buf[i + 1]; // �����������ƣ���λ�Ե�
    filter_sum += filter_buf[i];
  }
  
  return (float)filter_sum / FILTER4_N;
}


/*//////////////////////////////////////////////////////////////////////////
�����壺��λֵƽ���˲������ֳƷ��������ƽ���˲�����
������ ��һ�����ȥ�����ֵ����Сֵ��ȡƽ��ֵ��     ��Nֵ��ѡȡ��3-14���� 
      �൱�ڡ���λֵ�˲�����+������ƽ���˲�������
      ��������N�����ݣ�ȥ��һ�����ֵ��һ����Сֵ��
      Ȼ�����N-2�����ݵ�����ƽ��ֵ��    
�ŵ㣺 �ں��ˡ���λֵ�˲�����+������ƽ���˲����������˲������ŵ㡣
       ����żȻ���ֵ������Ը��ţ�����������������Ĳ���ֵƫ�
       �����ڸ��������õ��������á�
       ƽ���ȸߣ����ڸ�Ƶ�񵴵�ϵͳ��
ȱ�㣺���ڲ����ٶȽ�����Ҫ�����ݼ����ٶȽϿ��ʵʱ���Ʋ����ã��Ƚ��˷�RAM��
//////////////////////////////////////////////////////////////////////////*/
#define N5 100

float filter5(uint16_t *pValue) {
  int filter_temp, filter_sum = 0;
  uint16_t filter_buf[N5];
  for(int i = 0; i < N5; i++) {
    filter_buf[i] = *pValue;	
  }
  // ����ֵ��С�������У�ð�ݷ���
  for(int j = 0; j < N5 - 1; j++) {
    for(int i = 0; i < N5 - 1 - j; i++) {
      if(filter_buf[i] > filter_buf[i + 1]) {
        filter_temp = filter_buf[i];
        filter_buf[i] = filter_buf[i + 1];
        filter_buf[i + 1] = filter_temp;
      }
    }
  }
  // ȥ�������С��ֵ����ƽ��
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
