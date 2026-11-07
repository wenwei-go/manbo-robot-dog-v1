/**
	******************************************************************************
	* @file    mpu6050.c
	* @brief   MPU6050陀螺仪加速度计驱动实现
	******************************************************************************
	*/

#include "mpu6050.h"

/* 全局变量 */
static MPU6050_Data_t mpu_data = {0};
static MPU6050_Angle_t mpu_angle = {0};
static uint32_t last_update_time = 0;

/* 陀螺仪零点校准值 */
static float gyro_offset_x = 0;
static float gyro_offset_y = 0;
static float gyro_offset_z = 0;

/* 加速度计零点校准值 */
static float accel_offset_x = 0;
static float accel_offset_y = 0;
static float accel_offset_z = 0;

/*==========================================================================
 * 软件I2C底层实现
 *==========================================================================*/
static void MPU_I2C_Delay(void)
{
	uint8_t i = MPU_I2C_DELAY_TIME;
	while (i--);
}

static void MPU_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = MPU_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(MPU_SDA_PORT, &GPIO_InitStruct);
}

static void MPU_SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = MPU_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(MPU_SDA_PORT, &GPIO_InitStruct);
}

static void MPU_I2C_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	MPU_GPIO_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = MPU_SCL_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(MPU_SCL_PORT, &GPIO_InitStruct);
	
	MPU_SDA_OUT();
	
	MPU_SCL_H();
	MPU_SDA_H();
}

static void MPU_I2C_Start(void)
{
	MPU_SDA_OUT();
	MPU_SDA_H();
	MPU_SCL_H();
	MPU_I2C_Delay();
	MPU_SDA_L();
	MPU_I2C_Delay();
	MPU_SCL_L();
	MPU_I2C_Delay();
}

static void MPU_I2C_Stop(void)
{
	MPU_SDA_OUT();
	MPU_SCL_L();
	MPU_SDA_L();
	MPU_I2C_Delay();
	MPU_SCL_H();
	MPU_I2C_Delay();
	MPU_SDA_H();
	MPU_I2C_Delay();
}

static uint8_t MPU_I2C_WaitAck(void)
{
	uint8_t timeout = 0;
	
	MPU_SDA_IN();
	MPU_SDA_H();
	MPU_I2C_Delay();
	MPU_SCL_H();
	MPU_I2C_Delay();
	
	while (MPU_SDA_READ()) {
		timeout++;
		if (timeout > 250) {
			MPU_I2C_Stop();
			return 1;
		}
	}
	
	MPU_SCL_L();
	MPU_I2C_Delay();
	return 0;
}

static void MPU_I2C_Ack(void)
{
	MPU_SDA_OUT();
	MPU_SCL_L();
	MPU_SDA_L();
	MPU_I2C_Delay();
	MPU_SCL_H();
	MPU_I2C_Delay();
	MPU_SCL_L();
	MPU_I2C_Delay();
}

static void MPU_I2C_NAck(void)
{
	MPU_SDA_OUT();
	MPU_SCL_L();
	MPU_SDA_H();
	MPU_I2C_Delay();
	MPU_SCL_H();
	MPU_I2C_Delay();
	MPU_SCL_L();
	MPU_I2C_Delay();
}

static uint8_t MPU_I2C_SendByte(uint8_t byte)
{
	uint8_t i;
	
	MPU_SDA_OUT();
	MPU_SCL_L();
	
	for (i = 0; i < 8; i++) {
		if (byte & 0x80) {
			MPU_SDA_H();
		} else {
			MPU_SDA_L();
		}
		byte <<= 1;
		MPU_I2C_Delay();
		MPU_SCL_H();
		MPU_I2C_Delay();
		MPU_SCL_L();
		MPU_I2C_Delay();
	}
	
	return MPU_I2C_WaitAck();
}

static uint8_t MPU_I2C_ReceiveByte(void)
{
	uint8_t i, byte = 0;
	
	MPU_SDA_IN();
	
	for (i = 0; i < 8; i++) {
		MPU_SCL_L();
		MPU_I2C_Delay();
		MPU_SCL_H();
		MPU_I2C_Delay();
		byte <<= 1;
		if (MPU_SDA_READ()) {
			byte |= 0x01;
		}
	}
	
	MPU_SCL_L();
	MPU_I2C_Delay();
	
	return byte;
}

/*==========================================================================
 * MPU6050寄存器读写
 *==========================================================================*/
uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
	MPU_I2C_Start();
	
	if (MPU_I2C_SendByte(MPU6050_ADDR) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	if (MPU_I2C_SendByte(reg) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	if (MPU_I2C_SendByte(data) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	MPU_I2C_Stop();
	return 0;
}

uint8_t MPU6050_ReadReg(uint8_t reg, uint8_t *data)
{
	MPU_I2C_Start();
	
	if (MPU_I2C_SendByte(MPU6050_ADDR) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	if (MPU_I2C_SendByte(reg) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	MPU_I2C_Start();
	
	if (MPU_I2C_SendByte(MPU6050_ADDR | 0x01) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	*data = MPU_I2C_ReceiveByte();
	MPU_I2C_NAck();
	MPU_I2C_Stop();
	
	return 0;
}

uint8_t MPU6050_ReadRegs(uint8_t reg, uint8_t *data, uint8_t len)
{
	uint8_t i;
	
	MPU_I2C_Start();
	
	if (MPU_I2C_SendByte(MPU6050_ADDR) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	if (MPU_I2C_SendByte(reg) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	MPU_I2C_Start();
	
	if (MPU_I2C_SendByte(MPU6050_ADDR | 0x01) != 0) {
		MPU_I2C_Stop();
		return 1;
	}
	
	for (i = 0; i < len; i++) {
		data[i] = MPU_I2C_ReceiveByte();
		if (i == len - 1) {
			MPU_I2C_NAck();
		} else {
			MPU_I2C_Ack();
		}
	}
	
	MPU_I2C_Stop();
	return 0;
}

/*==========================================================================
 * MPU6050初始化和配置
 *==========================================================================*/
uint8_t MPU6050_TestConnection(void)
{
	uint8_t who_am_i;
	
	if (MPU6050_ReadReg(MPU6050_WHO_AM_I, &who_am_i) != 0) {
		return 1;
	}
	
	if (who_am_i == 0x68) {
		return 0;  // 连接成功
	}
	
	return 1;  // 连接失败
}

uint8_t MPU6050_Init(void)
{
	MPU_I2C_GPIO_Init();
	
	HAL_Delay(100);
	
	// 退出睡眠模式
	if (MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x00) != 0) return 1;
	HAL_Delay(10);
	
	// 设置采样率为1kHz
	if (MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x07) != 0) return 1;
	
	// 配置低通滤波器 (98Hz)
	if (MPU6050_WriteReg(MPU6050_CONFIG, 0x02) != 0) return 1;
	
	// 陀螺仪量程 ±2000°/s
	if (MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18) != 0) return 1;
	
	// 加速度计量程 ±2g
	if (MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x00) != 0) return 1;
	
	HAL_Delay(10);
	
	// 测试连接
	if (MPU6050_TestConnection() != 0) return 1;
	
	// 陀螺仪和加速度计零点校准（采集100次求平均）
	gyro_offset_x = 0;
	gyro_offset_y = 0;
	gyro_offset_z = 0;
	accel_offset_x = 0;
	accel_offset_y = 0;
	accel_offset_z = 0;
	
	for(uint8_t i = 0; i < 100; i++) {
		MPU6050_Data_t temp_data;
		MPU6050_ReadData(&temp_data);
		gyro_offset_x += temp_data.Gyro_X;
		gyro_offset_y += temp_data.Gyro_Y;
		gyro_offset_z += temp_data.Gyro_Z;
		accel_offset_x += temp_data.Accel_X;
		accel_offset_y += temp_data.Accel_Y;
		accel_offset_z += temp_data.Accel_Z;
		HAL_Delay(5);
	}
	
	gyro_offset_x /= 100.0f;
	gyro_offset_y /= 100.0f;
	gyro_offset_z /= 100.0f;
	accel_offset_x /= 100.0f;
	accel_offset_y /= 100.0f;
	accel_offset_z /= 100.0f;
	
	// 加速度计Z轴应该是1g（重力），所以offset要减去1g
	accel_offset_z -= 1.0f;
	
	last_update_time = HAL_GetTick();
	
	return 0;
}

/*==========================================================================
 * 数据读取
 *==========================================================================*/
void MPU6050_ReadRawData(MPU6050_RawData_t *raw)
{
	uint8_t data[14];
	
	MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H, data, 14);
	
	raw->Accel_X = (int16_t)((data[0] << 8) | data[1]);
	raw->Accel_Y = (int16_t)((data[2] << 8) | data[3]);
	raw->Accel_Z = (int16_t)((data[4] << 8) | data[5]);
	raw->Temperature = (int16_t)((data[6] << 8) | data[7]);
	raw->Gyro_X = (int16_t)((data[8] << 8) | data[9]);
	raw->Gyro_Y = (int16_t)((data[10] << 8) | data[11]);
	raw->Gyro_Z = (int16_t)((data[12] << 8) | data[13]);
}

void MPU6050_ReadData(MPU6050_Data_t *data)
{
	MPU6050_RawData_t raw;
	
	MPU6050_ReadRawData(&raw);
	
	// 加速度计：±2g量程，灵敏度16384 LSB/g
	data->Accel_X = raw.Accel_X / 16384.0f;
	data->Accel_Y = raw.Accel_Y / 16384.0f;
	data->Accel_Z = raw.Accel_Z / 16384.0f;
	
	// 陀螺仪：±2000°/s量程，灵敏度16.4 LSB/(°/s)
	data->Gyro_X = raw.Gyro_X / 16.4f;
	data->Gyro_Y = raw.Gyro_Y / 16.4f;
	data->Gyro_Z = raw.Gyro_Z / 16.4f;
	
	// 温度：公式 Temperature = (TEMP_OUT / 340) + 36.53
	data->Temperature = raw.Temperature / 340.0f + 36.53f;
}

float MPU6050_GetTemperature(void)
{
	uint8_t data[2];
	int16_t raw_temp;
	
	MPU6050_ReadRegs(MPU6050_TEMP_OUT_H, data, 2);
	raw_temp = (int16_t)((data[0] << 8) | data[1]);
	
	return raw_temp / 340.0f + 36.53f;
}

/*==========================================================================
 * 角度计算（互补滤波）
 *==========================================================================*/
void MPU6050_CalcAngle(MPU6050_Data_t *data, MPU6050_Angle_t *angle, float dt)
{
	float accel_roll, accel_pitch;
	float alpha = 0.96f;  // 互补滤波系数
	
	// 陀螺仪数据减去零点偏移
	float gyro_x = data->Gyro_X - gyro_offset_x;
	float gyro_y = data->Gyro_Y - gyro_offset_y;
	float gyro_z = data->Gyro_Z - gyro_offset_z;
	
	// 加速度计数据减去零点偏移
	float accel_x = data->Accel_X - accel_offset_x;
	float accel_y = data->Accel_Y - accel_offset_y;
	float accel_z = data->Accel_Z - accel_offset_z;
	
	// 死区处理：小于阈值的认为是0（减少漂移）
	const float GYRO_DEADZONE = 0.5f;  // 死区阈值 °/s
	if(fabs(gyro_x) < GYRO_DEADZONE) gyro_x = 0;
	if(fabs(gyro_y) < GYRO_DEADZONE) gyro_y = 0;
	if(fabs(gyro_z) < GYRO_DEADZONE) gyro_z = 0;
	
	// 由加速度计计算角度（单位：度）- 使用校准后的数据
	accel_roll = atan2(accel_y, accel_z) * 57.2957795f;
	accel_pitch = atan2(-accel_x, sqrt(accel_y * accel_y + 
	                    accel_z * accel_z)) * 57.2957795f;
	
	// 互补滤波：融合陀螺仪和加速度计数据
	angle->Roll = alpha * (angle->Roll + gyro_x * dt) + 
	              (1.0f - alpha) * accel_roll;
	angle->Pitch = alpha * (angle->Pitch + gyro_y * dt) + 
	               (1.0f - alpha) * accel_pitch;
	
	// 偏航角只能由陀螺仪积分得到（已优化）
	angle->Yaw += gyro_z * dt;
	
	// Yaw角限制在-180到180度之间
	if(angle->Yaw > 180.0f) angle->Yaw -= 360.0f;
	if(angle->Yaw < -180.0f) angle->Yaw += 360.0f;
}

void MPU6050_GetAngle(MPU6050_Angle_t *angle)
{
	angle->Roll = mpu_angle.Roll;
	angle->Pitch = mpu_angle.Pitch;
	angle->Yaw = mpu_angle.Yaw;
}

/*==========================================================================
 * 获取内部数据的函数（新增）
 *==========================================================================*/
void MPU6050_GetCurrentData(MPU6050_Data_t *data)
{
	data->Accel_X = mpu_data.Accel_X;
	data->Accel_Y = mpu_data.Accel_Y;
	data->Accel_Z = mpu_data.Accel_Z;
	data->Gyro_X = mpu_data.Gyro_X;
	data->Gyro_Y = mpu_data.Gyro_Y;
	data->Gyro_Z = mpu_data.Gyro_Z;
	data->Temperature = mpu_data.Temperature;
}

void MPU6050_GetCurrentAngle(MPU6050_Angle_t *angle)
{
	angle->Roll = mpu_angle.Roll;
	angle->Pitch = mpu_angle.Pitch;
	angle->Yaw = mpu_angle.Yaw;
}

/*==========================================================================
 * Yaw角重置函数（新增）
 *==========================================================================*/
void MPU6050_ResetYaw(void)
{
	mpu_angle.Yaw = 0;
}

/*==========================================================================
 * 定时器处理函数（5ms调用一次）
 *==========================================================================*/
void MPU_Proc(void)
{
	uint32_t current_time = HAL_GetTick();
	float dt = (current_time - last_update_time) / 1000.0f;  // 转换为秒
	
	last_update_time = current_time;
	
	// 读取数据
	MPU6050_ReadData(&mpu_data);
	
	// 计算角度
	MPU6050_CalcAngle(&mpu_data, &mpu_angle, dt);
}
