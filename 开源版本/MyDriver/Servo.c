#include "servo.h"                  // Device header
#include "tim.h"

/**
  * 函    数：PWM设置CCR
  * 参    数：Compare 要写入的CCR的值，范围：0~100
  * 返 回 值：无
  * 注意事项：CCR和ARR共同决定占空比，此函数仅设置CCR的值，并不直接是占空比
  *           占空比Duty = CCR / (ARR + 1)
  */
void PWM_SetCompare1(uint16_t Compare)//右前脚
{
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, Compare);
}
void PWM_SetCompare2(uint16_t Compare)//右后脚
{
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Compare);
}
void PWM_SetCompare3(uint16_t Compare)//左前脚
{
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, Compare);
}
void PWM_SetCompare4(uint16_t Compare)//左后脚
{
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, Compare);
}
void PWM_SetCompare5(uint16_t Compare)//尾巴 
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, Compare);
}

/**
  * 函    数：舵机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Servo_Init(void)
{
	//初始化舵机的底层PWM
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

/**
  * 函    数：舵机设置角度
  * 参    数：Angle 要设置的舵机角度，范围：0~180
  * 返 回 值：无
  */
void Servo_SetAngle1(float Angle)//左上
{
	Angle = 180 - Angle;
	PWM_SetCompare1(Angle / 180 * 2000 + 500);	//设置占空比
}
void Servo_SetAngle2(float Angle)//右上
{
	PWM_SetCompare2(Angle / 180 * 2000 + 500);	//设置占空比
}
void Servo_SetAngle3(float Angle)//左下
{
	Angle = 180 - Angle;
	PWM_SetCompare3(Angle / 180 * 2000 + 500);	//设置占空比
}
void Servo_SetAngle4(float Angle)//右下
{
	PWM_SetCompare4(Angle / 180 * 2000 + 500);	//设置占空比
}
void Servo_SetAngle5(float Angle)//尾巴
{
	PWM_SetCompare5(Angle / 180 * 2000 + 500);	//设置占空比
}

/**
  * 函    数：舵机读取角度
  * 参    数：无
  * 返 回 值：舵机角度，范围：0~180
  */
uint8_t Servo_GetAngle1(void)
{
	return (__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_2) - 500) * 180 / 2000;//占空比的反变换
}
uint8_t Servo_GetAngle2(void)
{
	return (__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1) - 500) * 180 / 2000;//占空比的反变换
}
uint8_t Servo_GetAngle3(void)
{
	return (__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_3) - 500) * 180 / 2000;//占空比的反变换
}
uint8_t Servo_GetAngle4(void)
{
	return (__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_4) - 500) * 180 / 2000;//占空比的反变换
}
uint8_t Servo_GetAngle5(void)
{
	return (__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1) - 500) * 180 / 2000;//占空比的反变换
}

