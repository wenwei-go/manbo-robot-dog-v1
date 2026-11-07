#ifndef __BH1750_H
#define __BH1750_H

#include "main.h"

/*==========================================================================
 * 软件I2C配置区域 - 根据你的硬件连接修改以下定义
 *==========================================================================*/

/* 定义I2C使用的GPIO端口和引脚 */
#define I2C_SCL_PORT        GPIOA
#define I2C_SCL_PIN         GPIO_PIN_4

#define I2C_SDA_PORT        GPIOA
#define I2C_SDA_PIN         GPIO_PIN_5

/* GPIO时钟使能 - 根据使用的GPIO端口修改 */
#define I2C_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()

/* I2C延时时间（调整I2C速度，值越大速度越慢） */
#define I2C_DELAY_TIME      10

/* GPIO操作宏定义 */
#define SCL_H()     HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, GPIO_PIN_SET)
#define SCL_L()     HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, GPIO_PIN_RESET)
#define SDA_H()     HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, GPIO_PIN_SET)
#define SDA_L()     HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, GPIO_PIN_RESET)
#define SDA_READ()  HAL_GPIO_ReadPin(I2C_SDA_PORT, I2C_SDA_PIN)

/*==========================================================================
 * BH1750配置
 *==========================================================================*/

/* BH1750 I2C地址 */
#define BH1750_ADDRESS          0x46  /* ADDRESS引脚接地时地址为0x46，接电源时地址为0xB8 */

/* BH1750 操作指令 */
#define BH1750_POW_OFF          0x00  /* 断电 */
#define BH1750_POW_ON           0x01  /* 上电 */
#define BH1750_POW_RST          0x07  /* 复位数据寄存器 */
#define BH1750_CONT_HI_RSLT_1   0x10  /* 连续测量，1lx精度，周期120ms */
#define BH1750_CONT_HI_RSLT_2   0x11  /* 连续测量，0.5lx精度，周期120ms */
#define BH1750_CONT_LOW_RSLT    0x13  /* 连续测量，4lx精度，周期16ms */
#define BH1750_ONE_HI_RSLT_1    0x20  /* 一次测量，1lx精度，周期120ms，测量完后自动进入POWER DOWN */
#define BH1750_ONE_HI_RSLT_2    0x21  /* 一次测量，0.5lx精度，周期120ms，测量完后自动进入POWER DOWN */
#define BH1750_ONE_LOW_RSLT     0x23  /* 一次测量，4lx精度，周期16ms，测量完后自动进入POWER DOWN */

/*==========================================================================
 * 函数声明
 *==========================================================================*/

HAL_StatusTypeDef BH1750_Init(void);
HAL_StatusTypeDef BH1750_WriteOpecode(uint8_t* pData, uint16_t size);
HAL_StatusTypeDef BH1750_ReadData(uint8_t* pData, uint16_t size);
uint16_t BH1750_Read(void);

#endif /* __BH1750_H */
