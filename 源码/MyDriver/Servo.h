#ifndef __SERVO_H
#define __SERVO_H
#include "main.h"  

void Servo_Init(void);//舵机初始化
void Servo_SetAngle1(float Angle);//左前脚
void Servo_SetAngle2(float Angle);//左后脚
void Servo_SetAngle3(float Angle);//右前脚
void Servo_SetAngle4(float Angle);//右后脚
void Servo_SetAngle5(float Angle);//尾巴
uint8_t Servo_GetAngle1(void);//舵机读取角度
uint8_t Servo_GetAngle2(void);
uint8_t Servo_GetAngle3(void);
uint8_t Servo_GetAngle4(void);
uint8_t Servo_GetAngle5(void);

#endif
