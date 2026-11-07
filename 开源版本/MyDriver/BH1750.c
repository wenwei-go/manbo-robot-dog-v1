#include "bh1750.h"

/*==========================================================================
 * 软件I2C底层实现
 *==========================================================================*/

/*
*************************************************
功能：延时函数，调整I2C速度
*************************************************
*/
static void I2C_Delay(void)
{
    uint8_t i = I2C_DELAY_TIME;
    while (i--);
}

/*
*************************************************
功能：设置SDA为输出模式
*************************************************
*/
static void SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_SDA_PORT, &GPIO_InitStruct);
}

/*
*************************************************
功能：设置SDA为输入模式
*************************************************
*/
static void SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_SDA_PORT, &GPIO_InitStruct);
}

/*
*************************************************
功能：初始化软件I2C的GPIO
*************************************************
*/
static void I2C_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能GPIO时钟 */
    I2C_GPIO_CLK_ENABLE();
    
    /* 配置SCL引脚 */
    GPIO_InitStruct.Pin = I2C_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_SCL_PORT, &GPIO_InitStruct);
    
    /* 配置SDA引脚 */
    SDA_OUT();
    
    /* 初始化总线为空闲状态 */
    SCL_H();
    SDA_H();
}

/*
*************************************************
功能：产生I2C起始信号
说明：SCL为高电平时，SDA由高变低
*************************************************
*/
static void I2C_Start(void)
{
    SDA_OUT();
    SDA_H();
    SCL_H();
    I2C_Delay();
    SDA_L();
    I2C_Delay();
    SCL_L();
    I2C_Delay();
}

/*
*************************************************
功能：产生I2C停止信号
说明：SCL为高电平时，SDA由低变高
*************************************************
*/
static void I2C_Stop(void)
{
    SDA_OUT();
    SCL_L();
    SDA_L();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SDA_H();
    I2C_Delay();
}

/*
*************************************************
功能：等待应答信号
返回：0-接收到应答，1-未接收到应答
*************************************************
*/
static uint8_t I2C_WaitAck(void)
{
    uint8_t timeout = 0;
    
    SDA_IN();
    SDA_H();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    
    while (SDA_READ()) {
        timeout++;
        if (timeout > 250) {
            I2C_Stop();
            return 1;
        }
    }
    
    SCL_L();
    I2C_Delay();
    return 0;
}

/*
*************************************************
功能：产生ACK应答
*************************************************
*/
static void I2C_Ack(void)
{
    SDA_OUT();
    SCL_L();
    SDA_L();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SCL_L();
    I2C_Delay();
}

/*
*************************************************
功能：产生NACK应答
*************************************************
*/
static void I2C_NAck(void)
{
    SDA_OUT();
    SCL_L();
    SDA_H();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SCL_L();
    I2C_Delay();
}

/*
*************************************************
功能：发送一个字节
参数：byte - 要发送的字节
返回：0-接收到应答，1-未接收到应答
*************************************************
*/
static uint8_t I2C_SendByte(uint8_t byte)
{
    uint8_t i;
    
    SDA_OUT();
    SCL_L();
    
    for (i = 0; i < 8; i++) {
        if (byte & 0x80) {
            SDA_H();
        } else {
            SDA_L();
        }
        byte <<= 1;
        I2C_Delay();
        SCL_H();
        I2C_Delay();
        SCL_L();
        I2C_Delay();
    }
    
    return I2C_WaitAck();
}

/*
*************************************************
功能：接收一个字节
返回：接收到的字节数据
*************************************************
*/
static uint8_t I2C_ReceiveByte(void)
{
    uint8_t i, byte = 0;
    
    SDA_IN();
    
    for (i = 0; i < 8; i++) {
        SCL_L();
        I2C_Delay();
        SCL_H();
        I2C_Delay();
        byte <<= 1;
        if (SDA_READ()) {
            byte |= 0x01;
        }
    }
    
    SCL_L();
    I2C_Delay();
    
    return byte;
}

/*==========================================================================
 * BH1750驱动实现
 *==========================================================================*/

/*
*************************************************
功能：初始化BH1750传感器
*************************************************
*/
HAL_StatusTypeDef BH1750_Init(void) 
{
    uint8_t opecode;
    HAL_StatusTypeDef status;
    
    /* 初始化I2C GPIO */
    I2C_GPIO_Init();
    
    /* 上电 */
    opecode = BH1750_POW_ON;
    status = BH1750_WriteOpecode(&opecode, 1);
    if (status != HAL_OK) return status;
    
    HAL_Delay(10);  // 等待上电稳定
    
    /* 设置为连续高分辨率模式 */
    opecode = BH1750_CONT_HI_RSLT_1;
    return BH1750_WriteOpecode(&opecode, 1);
}

/*
*************************************************
功能：写BH1750操作码到芯片，控制模式
*************************************************
*/
HAL_StatusTypeDef BH1750_WriteOpecode(uint8_t* pData, uint16_t size) 
{
    uint8_t i;
    
    /* 发送起始信号 */
    I2C_Start();
    
    /* 发送器件地址（写模式） */
    if (I2C_SendByte(BH1750_ADDRESS) != 0) {
        I2C_Stop();
        return HAL_ERROR;
    }
    
    /* 发送数据 */
    for (i = 0; i < size; i++) {
        if (I2C_SendByte(pData[i]) != 0) {
            I2C_Stop();
            return HAL_ERROR;
        }
    }
    
    /* 发送停止信号 */
    I2C_Stop();
    
    return HAL_OK;
}

/*
*************************************************
功能：读取BH1750的数据，存放到pData中
*************************************************
*/
HAL_StatusTypeDef BH1750_ReadData(uint8_t* pData, uint16_t size) 
{
    uint8_t i;
    
    /* 发送起始信号 */
    I2C_Start();
    
    /* 发送器件地址（读模式） */
    if (I2C_SendByte(BH1750_ADDRESS | 0x01) != 0) {
        I2C_Stop();
        return HAL_ERROR;
    }
    
    /* 读取数据 */
    for (i = 0; i < size; i++) {
        pData[i] = I2C_ReceiveByte();
        
        /* 最后一个字节发送NACK，其他发送ACK */
        if (i == size - 1) {
            I2C_NAck();
        } else {
            I2C_Ack();
        }
    }
    
    /* 发送停止信号 */
    I2C_Stop();
    
    return HAL_OK;
}

/*
*************************************************
功能：读取BH1750光照强度值
返回：光照强度（LUX）
说明：
光照数据计算（LUX）
LUX = 读取数据/1.2
例如：
读取值为0x56,0x78
LUX = 0X5678 / 1.2 = 22136/1.2 = 18447
*************************************************
*/
uint16_t BH1750_Read(void) 
{
    uint8_t pData[2];
    float ftmp = 0.0;
    
    /* 获取照度数据 */
    if (BH1750_ReadData(pData, 2) != HAL_OK) {
        return 0;  // 读取失败返回0
    }
    
    /* 转换照度数据 */
    ftmp = ((pData[0] << 8) | pData[1]) / 1.2f + 0.5f;
    
    return (uint16_t)ftmp;
}
