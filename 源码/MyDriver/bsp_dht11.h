/**
	******************************************************************************
	* @file    bsp_dht11.h
	* @author  
	* @date    
	* @version v2.0
	* @note    DHT11 driver header - 优化版
	******************************************************************************
	*/

#ifndef __BSP_DHT11_H
#define __BSP_DHT11_H
 
#include "tim.h"
#include "stm32f1xx.h"

/* DHT11 GPIO配置 */
#define BSP_DHT11_PORT    DHT11_GPIO_Port
#define BSP_DHT11_PIN     DHT11_Pin

/* DHT11 IO操作宏定义 */
#define DHT11_OUT_1       HAL_GPIO_WritePin(BSP_DHT11_PORT, BSP_DHT11_PIN, GPIO_PIN_SET)
#define DHT11_OUT_0       HAL_GPIO_WritePin(BSP_DHT11_PORT, BSP_DHT11_PIN, GPIO_PIN_RESET)
#define DHT11_IN          HAL_GPIO_ReadPin(BSP_DHT11_PORT, BSP_DHT11_PIN)

/* DHT11 数据结构 */
typedef struct
{
	uint8_t humi_int;     // 湿度的整数部分
	uint8_t humi_deci;    // 湿度的小数部分
	uint8_t temp_int;     // 温度的整数部分
	uint8_t temp_deci;    // 温度的小数部分
	uint8_t check_sum;    // 校验和
} DHT11_Data_TypeDef;

/* 函数声明 */
void bsp_delay_us(uint16_t us);
uint8_t DHT11_ReadData(float *Temp_Dat, unsigned int *RH_Dat);
uint8_t DHT11_ReadData_Retry(float *Temp_Dat, unsigned int *RH_Dat, uint8_t retry);

#endif /* __BSP_DHT11_H */
