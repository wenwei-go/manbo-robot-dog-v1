#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "Move.h"
#include "Servo.h"
#include "RobotControl.h"

USART_TypeDef fake_usart1 = {1}, fake_usart3 = {3};
UART_HandleTypeDef huart1 = {USART1}, huart3 = {USART3};
TIM_HandleTypeDef htim2, htim3;
uint32_t fake_primask;
uint8_t Face_Mode;
static uint8_t *dma_buffer, *voice_buffer;
static uint16_t dma_size, dma_position;
static unsigned starts, stops, dma_starts, voice_starts, aborts;
static unsigned fail_dma_starts, fail_voice_starts, fail_aborts;
static MoveState_t last_state;
static uint8_t last_repeats;

void Move_Start_Continuous(MoveState_t state, uint8_t repeats)
{
    assert(fake_primask == 0U);
    starts++;
    last_state = state;
    last_repeats = repeats;
}

void Move_Stop(void)
{
    assert(fake_primask == 0U);
    stops++;
    last_state = MOVE_IDLE;
}

HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *timer, uint32_t channel)
{
    assert(timer->compare[channel / 4U] == 1500U);
    timer->started[channel / 4U] = 1U;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *uart)
{
    assert(uart == &huart1 || uart == &huart3);
    aborts++;
    if (fail_aborts) { fail_aborts--; return HAL_ERROR; }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *uart, uint8_t *data, uint16_t size)
{
    assert(uart == &huart3);
    dma_starts++;
    if (fail_dma_starts) { fail_dma_starts--; return HAL_ERROR; }
    dma_buffer = data;
    dma_size = size;
    dma_position = 0U;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *uart, uint8_t *data, uint16_t size)
{
    assert(uart == &huart1 && size == 1U);
    voice_starts++;
    if (fail_voice_starts) { fail_voice_starts--; return HAL_ERROR; }
    voice_buffer = data;
    return HAL_OK;
}

static void Reset(void)
{
    starts = stops = dma_starts = voice_starts = aborts = 0U;
    fail_dma_starts = fail_voice_starts = fail_aborts = 0U;
    fake_primask = 0U;
    last_state = MOVE_IDLE;
    RobotControl_Init();
    assert(dma_size == 256U && dma_starts == 1U && voice_starts == 1U);
}

/* Model the HAL HT/TC/IDLE positions, including TC followed by IDLE at the same position. */
static void Bluetooth_Bytes(const uint8_t *bytes, unsigned size)
{
    unsigned i;
    for (i = 0; i < size; ++i)
    {
        dma_buffer[dma_position++] = bytes[i];
        if (dma_position == dma_size / 2U || dma_position == dma_size)
        {
            HAL_UARTEx_RxEventCallback(&huart3, dma_position);
        }
        if (dma_position == dma_size) dma_position = 0U;
    }
    HAL_UARTEx_RxEventCallback(&huart3, dma_position ? dma_position : dma_size);
}

static void Repeat_Bluetooth(uint8_t byte, unsigned count)
{
    uint8_t bytes[256];
    assert(count <= sizeof(bytes));
    memset(bytes, byte, count);
    Bluetooth_Bytes(bytes, count);
}

static void Voice_Byte(uint8_t byte)
{
    *voice_buffer = byte;
    HAL_UART_RxCpltCallback(&huart1);
}

static void Test_Command_Contract(void)
{
    static const struct {
        uint8_t byte, face, repeats;
        MoveState_t state;
    } cases[] = {
        {0x01, 0, 1, MOVE_SLEEP}, {0x02, 1, 1, MOVE_DunXia},
        {0x03, 5, 1, MOVE_STAND}, {0x04, 1, 1, MOVE_PaXia},
        {0x05, 2, 2, MOVE_FORWARD}, {0x06, 2, 2, MOVE_BEHIND},
        {0x07, 2, 5, MOVE_LEFT}, {0x08, 2, 5, MOVE_RIGHT},
        {0x09, 2, 1, MOVE_SHAKE_QIANHOU}, {0x10, 4, 1, MOVE_FAST_SHAKE_QIANHOU},
        {0x11, 4, 1, MOVE_FAST_SHAKE_QIANHOU}, {0x12, 2, 1, MOVE_WAG_TAIL},
        {0x21, 4, 1, MOVE_FAST_SHAKE_QIANHOU}, {0x22, 0, 1, MOVE_SLEEP},
        {0x23, 2, 1, MOVE_WAG_TAIL}
    };
    unsigned i, byte;
    for (byte = 0U; byte <= 255U; ++byte)
    {
        uint8_t command = (uint8_t)byte;
        Reset();
        Bluetooth_Bytes(&command, 1U);
        assert(starts == 0U && stops == 0U); /* No motion mutation in the ISR. */
        RobotControl_Process();
        for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
        {
            if (cases[i].byte == command) break;
        }
        if (i < sizeof(cases) / sizeof(cases[0]))
        {
            assert(starts == 1U && stops == 0U);
            assert(last_state == cases[i].state && last_repeats == cases[i].repeats);
            assert(Face_Mode == cases[i].face);
        }
        else assert(starts == 0U && stops == 1U);
    }
}

static void Test_DMA_And_Queue(void)
{
    const uint8_t burst[] = {0x05, 0x00, 0x23};
    Reset();
    Bluetooth_Bytes(burst, sizeof(burst));
    RobotControl_Process();
    assert(starts == 2U && stops == 1U && last_state == MOVE_WAG_TAIL);
    assert(dma_starts == 1U); /* IDLE does not stop/rearm circular DMA. */

    Reset();
    Repeat_Bluetooth(0x03, 120U); RobotControl_Process();
    Repeat_Bluetooth(0x21, 20U); RobotControl_Process(); /* Cross HT boundary. */
    Repeat_Bluetooth(0x22, 116U); RobotControl_Process(); /* Cross TC boundary. */
    assert(starts == 256U);
    HAL_UARTEx_RxEventCallback(&huart3, 256U); RobotControl_Process();
    assert(starts == 256U);
    Repeat_Bluetooth(0x23, 10U); RobotControl_Process(); /* DMA wraps to zero. */
    assert(starts == 266U && last_state == MOVE_WAG_TAIL);
    HAL_UARTEx_RxEventCallback(&huart3, 0U);
    HAL_UARTEx_RxEventCallback(&huart3, 257U);
    HAL_UARTEx_RxEventCallback(&huart1, 10U);
    RobotControl_Process();
    assert(starts == 266U);

    Reset();
    Repeat_Bluetooth(0x03, 256U); /* Queue has 255 usable slots. */
    RobotControl_Process();
    assert(starts == 0U && stops == 1U);
    Repeat_Bluetooth(0x21, 1U); RobotControl_Process();
    assert(starts == 1U && last_state == MOVE_FAST_SHAKE_QIANHOU);

    Reset();
    Bluetooth_Bytes((const uint8_t *)"0x21\r\n", 6U); RobotControl_Process();
    assert(starts == 0U && stops == 6U);
}

static void Test_Voice_And_Recovery(void)
{
    unsigned old_starts, old_aborts;
    Reset();
    Voice_Byte(0x05);
    Repeat_Bluetooth(0x22, 1U);
    Voice_Byte(0x03);
    assert(starts == 0U);
    RobotControl_Process();
    assert(starts == 3U && last_state == MOVE_STAND && voice_starts == 3U);

    old_starts = dma_starts; old_aborts = aborts;
    HAL_UART_ErrorCallback(&huart3);
    assert(dma_starts == old_starts && aborts == old_aborts);
    fail_dma_starts = 1U;
    RobotControl_Process();
    assert(dma_starts == old_starts + 1U);
    RobotControl_Process();
    assert(dma_starts == old_starts + 2U);
    Repeat_Bluetooth(0x23, 1U); RobotControl_Process();
    assert(starts == 4U && last_state == MOVE_WAG_TAIL);

    old_starts = voice_starts;
    fail_voice_starts = 1U;
    Voice_Byte(0x01);
    RobotControl_Process();
    assert(voice_starts == old_starts + 2U && last_state == MOVE_SLEEP);
    HAL_UART_ErrorCallback(&huart1);
    fail_aborts = 1U;
    RobotControl_Process(); /* Failed abort is retried on the next loop. */
    RobotControl_Process();
    Voice_Byte(0x03); RobotControl_Process();
    assert(last_state == MOVE_STAND);
}

static void Test_Servo_Pins_And_Angles(void)
{
    unsigned i, angle;
    void (*setters[5])(float) = {Servo_SetAngle1, Servo_SetAngle2, Servo_SetAngle3, Servo_SetAngle4, Servo_SetAngle5};
    uint8_t (*getters[5])(void) = {Servo_GetAngle1, Servo_GetAngle2, Servo_GetAngle3, Servo_GetAngle4, Servo_GetAngle5};
    memset(&htim2, 0, sizeof(htim2)); memset(&htim3, 0, sizeof(htim3));
    Servo_Init();
    for (i = 0; i < 4U; ++i) assert(htim2.started[i] == 1U);
    assert(htim3.started[1] == 1U && htim3.started[0] == 0U && htim3.started[2] == 0U && htim3.started[3] == 0U);
    Servo_SetAngle1(0); assert(htim3.compare[1] == 2500U); /* LF PA7 */
    Servo_SetAngle2(0); assert(htim2.compare[3] == 500U);  /* LB PA3 */
    Servo_SetAngle3(0); assert(htim2.compare[0] == 2500U); /* RF PA0 */
    Servo_SetAngle4(0); assert(htim2.compare[1] == 500U);  /* RB PA1 */
    Servo_SetAngle5(0); assert(htim2.compare[2] == 500U);  /* M PA2 */
    for (i = 0; i < 5U; ++i)
    {
        for (angle = 0; angle <= 180U; ++angle)
        {
            unsigned actual;
            setters[i]((float)angle); actual = getters[i]();
            assert(actual + 1U >= angle && actual <= angle + 1U);
        }
        setters[i](-30.0f); assert(getters[i]() == 0U);
        setters[i](210.0f); assert(getters[i]() == 180U);
    }
}

int main(void)
{
    Test_Command_Contract();
    Test_DMA_And_Queue();
    Test_Voice_And_Recovery();
    Test_Servo_Pins_And_Angles();
    puts("PASS: 256 command values; DMA HT/TC/IDLE, wrap and deduplication; queue overflow; UART recovery; five servo mappings and 0..180 degree round trips.");
    return 0;
}
