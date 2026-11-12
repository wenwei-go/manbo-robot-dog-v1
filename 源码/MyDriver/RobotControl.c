#include "RobotControl.h"
#include "Move.h"
#include "usart.h"

#define RX_BUFFER_SIZE 256U
#define COMMAND_QUEUE_SIZE 256U

static uint8_t bluetooth_rx[RX_BUFFER_SIZE];
static uint16_t bluetooth_position;
static uint8_t voice_rx;
/* Both UART IRQs and the DMA IRQ use preemption priority 0: one producer at a time. */
static volatile uint8_t commands[COMMAND_QUEUE_SIZE];
static volatile uint16_t command_head;
static volatile uint16_t command_tail;
static volatile uint8_t command_overflow;
static volatile uint8_t bluetooth_restart;
static volatile uint8_t voice_restart;

static void Queue_Command(uint8_t command)
{
    uint16_t next = (command_head + 1U) % COMMAND_QUEUE_SIZE;
    if (next == command_tail)
    {
        command_overflow = 1U;
        return;
    }
    commands[command_head] = command;
    command_head = next;
}

static void Start_Receivers(void)
{
    if (bluetooth_restart)
    {
        if (HAL_UART_AbortReceive(&huart3) == HAL_OK)
        {
            bluetooth_position = 0U;
            bluetooth_restart = 0U;
            /* Circular DMA: keep HT/TC enabled, receive continuously across IDLE events. */
            if (HAL_UARTEx_ReceiveToIdle_DMA(&huart3, bluetooth_rx, sizeof(bluetooth_rx)) != HAL_OK)
            {
                bluetooth_restart = 1U;
            }
        }
    }
    if (voice_restart)
    {
        if (HAL_UART_AbortReceive(&huart1) == HAL_OK)
        {
            voice_restart = 0U;
            if (HAL_UART_Receive_IT(&huart1, &voice_rx, 1U) != HAL_OK)
            {
                voice_restart = 1U;
            }
        }
    }
}

void RobotControl_Init(void)
{
    command_head = 0U;
    command_tail = 0U;
    command_overflow = 0U;
    bluetooth_restart = 1U;
    voice_restart = 1U;
    Start_Receivers();
}

static void Execute_Command(uint8_t command)
{
    switch (command)
    {
        case 0x01: Face_Mode = 0; Move_Start_Continuous(MOVE_SLEEP, 1); break;
        case 0x02: Face_Mode = 1; Move_Start_Continuous(MOVE_DunXia, 1); break;
        case 0x03: Face_Mode = 5; Move_Start_Continuous(MOVE_STAND, 1); break;
        case 0x04: Face_Mode = 1; Move_Start_Continuous(MOVE_PaXia, 1); break;
        case 0x05: Face_Mode = 2; Move_Start_Continuous(MOVE_FORWARD, 2); break;
        case 0x06: Face_Mode = 2; Move_Start_Continuous(MOVE_BEHIND, 2); break;
        case 0x07: Face_Mode = 2; Move_Start_Continuous(MOVE_LEFT, 5); break;
        case 0x08: Face_Mode = 2; Move_Start_Continuous(MOVE_RIGHT, 5); break;
        case 0x09: Face_Mode = 2; Move_Start_Continuous(MOVE_SHAKE_QIANHOU, 1); break;
        case 0x10:
        case 0x11:
        case 0x21: Face_Mode = 4; Move_Start_Continuous(MOVE_FAST_SHAKE_QIANHOU, 1); break;
        case 0x22: Face_Mode = 0; Move_Start_Continuous(MOVE_SLEEP, 1); break;
        case 0x12:
        case 0x23: Face_Mode = 2; Move_Start_Continuous(MOVE_WAG_TAIL, 1); break;
        default: Move_Stop(); break;
    }
}

void RobotControl_Process(void)
{
    Start_Receivers();
    for (;;)
    {
        uint8_t command;
        uint32_t primask = __get_PRIMASK();
        __disable_irq();
        if (command_overflow)
        {
            /* Discard an incomplete burst and stop instead of executing a stale prefix. */
            command_tail = command_head;
            command_overflow = 0U;
            __set_PRIMASK(primask);
            Move_Stop();
            return;
        }
        if (command_tail == command_head)
        {
            __set_PRIMASK(primask);
            return;
        }
        command = commands[command_tail];
        command_tail = (command_tail + 1U) % COMMAND_QUEUE_SIZE;
        __set_PRIMASK(primask);
        /* The foreground owns the motion state; ISR callbacks only enqueue bytes. */
        Execute_Command(command);
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    uint16_t position;
    if (huart->Instance != USART3 || Size == 0U || Size > sizeof(bluetooth_rx))
    {
        return;
    }
    position = bluetooth_position;
    if (Size < position)
    {
        while (position < sizeof(bluetooth_rx))
        {
            Queue_Command(bluetooth_rx[position++]);
        }
        position = 0U;
    }
    while (position < Size)
    {
        Queue_Command(bluetooth_rx[position++]);
    }
    /* Repeated IDLE/TC at the same position does not enqueue duplicate commands. */
    bluetooth_position = Size;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Queue_Command(voice_rx);
        if (HAL_UART_Receive_IT(&huart1, &voice_rx, 1U) != HAL_OK)
        {
            voice_restart = 1U;
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    /* Abort/restart in the foreground, including HAL's nonfatal RX error path. */
    if (huart->Instance == USART3)
    {
        bluetooth_restart = 1U;
    }
    else if (huart->Instance == USART1)
    {
        voice_restart = 1U;
    }
}
