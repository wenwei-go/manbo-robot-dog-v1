#ifndef TEST_MAIN_H
#define TEST_MAIN_H
#include <stdint.h>

typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
typedef struct { uint32_t compare[4]; uint8_t started[4]; } TIM_HandleTypeDef;
typedef struct { uint8_t number; } USART_TypeDef;
typedef struct { USART_TypeDef *Instance; } UART_HandleTypeDef;
extern USART_TypeDef fake_usart1, fake_usart3;
#define USART1 (&fake_usart1)
#define USART3 (&fake_usart3)
#define TIM_CHANNEL_1 0U
#define TIM_CHANNEL_2 4U
#define TIM_CHANNEL_3 8U
#define TIM_CHANNEL_4 12U
#define __HAL_TIM_SET_COMPARE(timer, channel, value) ((timer)->compare[(channel) / 4U] = (value))
#define __HAL_TIM_GET_COMPARE(timer, channel) ((timer)->compare[(channel) / 4U])
extern uint32_t fake_primask;
#define __get_PRIMASK() fake_primask
#define __disable_irq() (fake_primask = 1U)
#define __set_PRIMASK(value) (fake_primask = (value))

HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *timer, uint32_t channel);
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *uart);
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *uart, uint8_t *data, uint16_t size);
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *uart, uint8_t *data, uint16_t size);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *uart, uint16_t size);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart);
#endif
