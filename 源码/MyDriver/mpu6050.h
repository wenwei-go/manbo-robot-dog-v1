/**
	******************************************************************************
	* @file    mpu6050.h
	* @brief   MPU6050陀螺仪加速度计驱动 - 软件I2C实现
	* @note    包含角度计算（互补滤波）
	******************************************************************************
	*/

#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"
#include <math.h>

/*==========================================================================
 * 软件I2C配置区域
 *==========================================================================*/
#define MPU_SCL_PORT        GPIOB
#define MPU_SCL_PIN         GPIO_PIN_4

#define MPU_SDA_PORT        GPIOB
#define MPU_SDA_PIN         GPIO_PIN_3

#define MPU_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
#define MPU_I2C_DELAY_TIME      5

/* GPIO操作宏 */
#define MPU_SCL_H()     HAL_GPIO_WritePin(MPU_SCL_PORT, MPU_SCL_PIN, GPIO_PIN_SET)
#define MPU_SCL_L()     HAL_GPIO_WritePin(MPU_SCL_PORT, MPU_SCL_PIN, GPIO_PIN_RESET)
#define MPU_SDA_H()     HAL_GPIO_WritePin(MPU_SDA_PORT, MPU_SDA_PIN, GPIO_PIN_SET)
#define MPU_SDA_L()     HAL_GPIO_WritePin(MPU_SDA_PORT, MPU_SDA_PIN, GPIO_PIN_RESET)
#define MPU_SDA_READ()  HAL_GPIO_ReadPin(MPU_SDA_PORT, MPU_SDA_PIN)

/*==========================================================================
 * MPU6050寄存器定义
 *==========================================================================*/
#define MPU6050_ADDR            0xD0    // MPU6050的I2C地址（AD0接GND）
// #define MPU6050_ADDR         0xD2    // 如果AD0接VCC，使用这个地址

#define MPU6050_SMPLRT_DIV      0x19    // 采样率分频器
#define MPU6050_CONFIG          0x1A    // 配置寄存器
#define MPU6050_GYRO_CONFIG     0x1B    // 陀螺仪配置
#define MPU6050_ACCEL_CONFIG    0x1C    // 加速度计配置

#define MPU6050_ACCEL_XOUT_H    0x3B    // 加速度X轴高字节
#define MPU6050_ACCEL_XOUT_L    0x3C
#define MPU6050_ACCEL_YOUT_H    0x3D
#define MPU6050_ACCEL_YOUT_L    0x3E
#define MPU6050_ACCEL_ZOUT_H    0x3F
#define MPU6050_ACCEL_ZOUT_L    0x40

#define MPU6050_TEMP_OUT_H      0x41    // 温度高字节
#define MPU6050_TEMP_OUT_L      0x42

#define MPU6050_GYRO_XOUT_H     0x43    // 陀螺仪X轴高字节
#define MPU6050_GYRO_XOUT_L     0x44
#define MPU6050_GYRO_YOUT_H     0x45
#define MPU6050_GYRO_YOUT_L     0x46
#define MPU6050_GYRO_ZOUT_H     0x47
#define MPU6050_GYRO_ZOUT_L     0x48

#define MPU6050_PWR_MGMT_1      0x6B    // 电源管理1
#define MPU6050_PWR_MGMT_2      0x6C    // 电源管理2
#define MPU6050_WHO_AM_I        0x75    // 器件ID

/*==========================================================================
 * 数据结构定义
 *==========================================================================*/
/* 原始数据结构 */
typedef struct {
	int16_t Accel_X;    // 加速度X轴原始数据
	int16_t Accel_Y;    // 加速度Y轴原始数据
	int16_t Accel_Z;    // 加速度Z轴原始数据
	int16_t Gyro_X;     // 陀螺仪X轴原始数据
	int16_t Gyro_Y;     // 陀螺仪Y轴原始数据
	int16_t Gyro_Z;     // 陀螺仪Z轴原始数据
	int16_t Temperature; // 温度原始数据
} MPU6050_RawData_t;

/* 实际物理量数据结构 */
typedef struct {
	float Accel_X;      // 加速度X轴 (g)
	float Accel_Y;      // 加速度Y轴 (g)
	float Accel_Z;      // 加速度Z轴 (g)
	float Gyro_X;       // 陀螺仪X轴 (°/s)
	float Gyro_Y;       // 陀螺仪Y轴 (°/s)
	float Gyro_Z;       // 陀螺仪Z轴 (°/s)
	float Temperature;  // 温度 (℃)
} MPU6050_Data_t;

/* 姿态角结构 */
typedef struct {
	float Roll;         // 翻滚角 (绕X轴)
	float Pitch;        // 俯仰角 (绕Y轴)
	float Yaw;          // 偏航角 (绕Z轴)
} MPU6050_Angle_t;

/*==========================================================================
 * 函数声明
 *==========================================================================*/
/* 初始化和基础读写 */
uint8_t MPU6050_Init(void);
uint8_t MPU6050_TestConnection(void);
uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t data);
uint8_t MPU6050_ReadReg(uint8_t reg, uint8_t *data);
uint8_t MPU6050_ReadRegs(uint8_t reg, uint8_t *data, uint8_t len);

/* 数据读取 */
void MPU6050_ReadRawData(MPU6050_RawData_t *raw);
void MPU6050_ReadData(MPU6050_Data_t *data);
float MPU6050_GetTemperature(void);

/* 角度计算 */
void MPU6050_CalcAngle(MPU6050_Data_t *data, MPU6050_Angle_t *angle, float dt);
void MPU6050_GetAngle(MPU6050_Angle_t *angle);

/* 定时器处理函数 */
void MPU_Proc(void);

/* 获取内部数据的函数（新增） */
void MPU6050_GetCurrentData(MPU6050_Data_t *data);
void MPU6050_GetCurrentAngle(MPU6050_Angle_t *angle);

#endif /* __MPU6050_H */
