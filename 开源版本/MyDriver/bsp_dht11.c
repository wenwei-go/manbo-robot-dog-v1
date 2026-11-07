/**
	******************************************************************************
	* @file    bsp_dht11.c
	* @author  
	* @date    
	* @version v2.0
	* @note    DHT11 driver - 优化版，增加超时保护，防止程序卡死
	******************************************************************************
	*/
 
#include "tim.h"
#include "bsp_dht11.h"

/**
	* @brief 微秒级延时
	*/
void bsp_delay_us(uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	HAL_TIM_Base_Start(&htim3);
	while (__HAL_TIM_GET_COUNTER(&htim3) < us);
	HAL_TIM_Base_Stop(&htim3);
}

/**
	* @brief DHT11 输出模式
	*/
static void DHT11_Mode_OUT_PP(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = BSP_DHT11_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	
	HAL_GPIO_Init(BSP_DHT11_PORT, &GPIO_InitStruct);
}

/**
	* @brief DHT11 输入模式
	*/
static void DHT11_Mode_IN_NP(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = BSP_DHT11_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;  // 改为上拉输入，更稳定
	
	HAL_GPIO_Init(BSP_DHT11_PORT, &GPIO_InitStruct);
}

/**
	* @brief 等待引脚变为指定电平，带超时保护
	* @param level: 等待的电平状态 (0或1)
	* @param timeout_us: 超时时间(微秒)
	* @retval 0-成功, 1-超时
	*/
static uint8_t DHT11_WaitLevel(uint8_t level, uint16_t timeout_us)
{
	uint16_t time = 0;
	
	while (DHT11_IN != level) {
		bsp_delay_us(1);
		time++;
		if (time >= timeout_us) {
			return 1;  // 超时
		}
	}
	return 0;  // 成功
}

/**
	* @brief DHT11 读取一个字节
	* @retval 读取到的字节数据
	*/
static uint8_t DHT11_ReadByte(void)
{
	uint8_t i, temp = 0;
 
	for (i = 0; i < 8; i++) {
		// 等待低电平结束，超时100us
		if (DHT11_WaitLevel(1, 100) != 0) {
			return 0;  // 超时返回0
		}
		
		// 延时40us判断是0还是1
		bsp_delay_us(40);
		
		if (DHT11_IN == 1) {
			temp |= (uint8_t)(0x01 << (7 - i));  // 高位在前
			// 等待高电平结束，超时100us
			if (DHT11_WaitLevel(0, 100) != 0) {
				return 0;  // 超时返回0
			}
		} else {
			temp &= (uint8_t)~(0x01 << (7 - i));
		}
	}
	return temp;
}

/**
	* @brief DHT11 读取温湿度数据
	* @param Temp_Dat: 温度数据指针
	* @param RH_Dat: 湿度数据指针
	* @retval 0-读取失败, 1-读取成功
	*/
uint8_t DHT11_ReadData(float *Temp_Dat, unsigned int *RH_Dat)
{
	DHT11_Data_TypeDef DHT11_Data;
	uint8_t checksum;
	
	// 1. 主机发送起始信号
	DHT11_Mode_OUT_PP();
	DHT11_OUT_0;
	HAL_Delay(18);  // 拉低至少18ms
	
	DHT11_OUT_1;
	bsp_delay_us(30);  // 拉高20-40us
	
	// 2. 切换为输入模式，等待DHT11响应
	DHT11_Mode_IN_NP();
	
	// 3. 等待DHT11拉低（响应信号），超时100us
	if (DHT11_WaitLevel(0, 100) != 0) {
		DHT11_Mode_OUT_PP();
		DHT11_OUT_1;
		return 0;  // 未收到应答
	}
	
	// 4. 等待DHT11拉高，超时100us
	if (DHT11_WaitLevel(1, 100) != 0) {
		DHT11_Mode_OUT_PP();
		DHT11_OUT_1;
		return 0;  // 应答信号异常
	}
	
	// 5. 等待DHT11拉低（准备发送数据），超时100us
	if (DHT11_WaitLevel(0, 100) != 0) {
		DHT11_Mode_OUT_PP();
		DHT11_OUT_1;
		return 0;  // 应答信号异常
	}
	
	// 6. 开始接收40位数据
	DHT11_Data.humi_int  = DHT11_ReadByte();  // 湿度整数
	DHT11_Data.humi_deci = DHT11_ReadByte();  // 湿度小数
	DHT11_Data.temp_int  = DHT11_ReadByte();  // 温度整数
	DHT11_Data.temp_deci = DHT11_ReadByte();  // 温度小数
	DHT11_Data.check_sum = DHT11_ReadByte();  // 校验和
	
	// 7. 读取完成，恢复输出模式
	DHT11_Mode_OUT_PP();
	DHT11_OUT_1;
	
	// 8. 数据校验
	checksum = DHT11_Data.humi_int + DHT11_Data.humi_deci + 
	           DHT11_Data.temp_int + DHT11_Data.temp_deci;
	
	if (DHT11_Data.check_sum != checksum) {
		return 0;  // 校验失败
	}
	
	// 9. 数据合理性检查
	if (DHT11_Data.humi_int > 100 || DHT11_Data.temp_int > 50) {
		return 0;  // 数据异常
	}
	
	// 10. 输出数据
	*Temp_Dat = DHT11_Data.temp_int + DHT11_Data.temp_deci * 0.1f;
	*RH_Dat = DHT11_Data.humi_int;
	
	return 1;  // 读取成功
}

/**
	* @brief DHT11 多次读取，提高成功率
	* @param Temp_Dat: 温度数据指针
	* @param RH_Dat: 湿度数据指针
	* @param retry: 重试次数
	* @retval 0-读取失败, 1-读取成功
	*/
uint8_t DHT11_ReadData_Retry(float *Temp_Dat, unsigned int *RH_Dat, uint8_t retry)
{
	uint8_t i;
	
	for (i = 0; i < retry; i++) {
		if (DHT11_ReadData(Temp_Dat, RH_Dat) == 1) {
			return 1;  // 读取成功
		}
		HAL_Delay(100);  // 失败后延时100ms再重试
	}
	
	return 0;  // 重试失败
}
