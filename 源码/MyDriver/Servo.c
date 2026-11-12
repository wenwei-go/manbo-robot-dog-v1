#include "Servo.h"
#include "tim.h"

/* 20 ms period, 1 us timer ticks; GPIO mapping follows hardware/引脚连接.md. */
void PWM_SetCompare1(uint16_t compare) /* 左前 LF: PA7, TIM3_CH2 */
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, compare);
}

void PWM_SetCompare2(uint16_t compare) /* 左后 LB: PA3, TIM2_CH4 */
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, compare);
}

void PWM_SetCompare3(uint16_t compare) /* 右前 RF: PA0, TIM2_CH1 */
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, compare);
}

void PWM_SetCompare4(uint16_t compare) /* 右后 RB: PA1, TIM2_CH2 */
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, compare);
}

void PWM_SetCompare5(uint16_t compare) /* 尾巴 M: PA2, TIM2_CH3 */
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, compare);
}

void Servo_Init(void)
{
    /* Set neutral pulses before enabling outputs; avoid an initial zero pulse. */
    Servo_SetAngle1(90);
    Servo_SetAngle2(90);
    Servo_SetAngle3(90);
    Servo_SetAngle4(90);
    Servo_SetAngle5(90);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

static uint16_t Angle_To_Pulse(float angle)
{
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    return (uint16_t)(angle / 180.0f * 2000.0f + 500.0f);
}

static uint8_t Pulse_To_Angle(uint32_t pulse)
{
    if (pulse <= 500U) return 0U;
    if (pulse >= 2500U) return 180U;
    return (uint8_t)((pulse - 500U) * 180U / 2000U);
}

void Servo_SetAngle1(float angle) /* 左前，保留原逻辑反向 */
{
    PWM_SetCompare1(Angle_To_Pulse(180.0f - angle));
}

void Servo_SetAngle2(float angle) /* 左后 */
{
    PWM_SetCompare2(Angle_To_Pulse(angle));
}

void Servo_SetAngle3(float angle) /* 右前，保留原逻辑反向 */
{
    PWM_SetCompare3(Angle_To_Pulse(180.0f - angle));
}

void Servo_SetAngle4(float angle) /* 右后 */
{
    PWM_SetCompare4(Angle_To_Pulse(angle));
}

void Servo_SetAngle5(float angle) /* 尾巴 */
{
    PWM_SetCompare5(Angle_To_Pulse(angle));
}

uint8_t Servo_GetAngle1(void)
{
    return Pulse_To_Angle(3000U - __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2));
}

uint8_t Servo_GetAngle2(void)
{
    return Pulse_To_Angle(__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_4));
}

uint8_t Servo_GetAngle3(void)
{
    return Pulse_To_Angle(3000U - __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1));
}

uint8_t Servo_GetAngle4(void)
{
    return Pulse_To_Angle(__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_2));
}

uint8_t Servo_GetAngle5(void)
{
    return Pulse_To_Angle(__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_3));
}
