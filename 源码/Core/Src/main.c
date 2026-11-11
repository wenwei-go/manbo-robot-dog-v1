/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "OLED.h"
#include "servo.h"
#include "Move.h"
#include <time.h>
#include <stdlib.h>
uint16_t OLED_Slow_Down = 0;
char display_buf[32];
char buf[32];

uint8_t rx_data[256] = {0};// 串口接收缓冲区
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void OLED_Proc(void){
	if(OLED_Slow_Down <= 100) return;
	OLED_Slow_Down = 0;
	
	/*图案处理*/
	switch(Face_Mode)
	{
		case 0:
			OLED_Clear();
			OLED_ShowImage(0,0,128,64,Face_sleep);//睡觉
		break;
		
		case 1:
			OLED_Clear();
			OLED_ShowImage(0,0,128,64,Face_stare);//Ω
		break;
		
		case 2:
			OLED_Clear();
			OLED_ShowImage(0,0,128,64,Face_happy);//><
		break;
		
		case 3:
			OLED_Clear();
			OLED_ShowImage(0,0,128,64,Face_mania);//火火火
		break;
		
		case 4:
			OLED_Clear();
			OLED_ShowImage(0,0,128,64,Face_very_happy);//非常快乐
		break;
		
		case 5:
			OLED_Clear();
			OLED_ShowImage(0,0,128,64,Face_eyes);//眼睛
		break;
		
		case 6:
			OLED_Clear();
			OLED_ShowImage(0,0,128,64,Face_hello);//神奇的小表情
		break;
	}
	OLED_Update();
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if(htim->Instance == htim4.Instance){  // 1毫秒定时器
		OLED_Slow_Down++;
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART3)
    {
        if (Size > 0) {
            uint8_t cmd = rx_data[0];
            switch (cmd) {
                case 0x01: Face_Mode=0;		Move_Start_Continuous(MOVE_SLEEP, 1); 				break;   //{ID:1,keyword:"命令词",ASR:		"睡觉",				ASRTO:"晚安，玛卡巴卡"}
                case 0x02: Face_Mode=1; 	Move_Start_Continuous(MOVE_DunXia, 1); 				break;   //{ID:2,keyword:"命令词",ASR:		"蹲下",				ASRTO:"知道了"}
                case 0x03: Face_Mode=5;	 	Move_Start_Continuous(MOVE_STAND, 1); 				break;   //{ID:3,keyword:"命令词",ASR:		"立正",				ASRTO:"我立了"}
                case 0x04: Face_Mode=1; 	Move_Start_Continuous(MOVE_PaXia, 1); 				break;   //{ID:4,keyword:"命令词",ASR:		"趴下",				ASRTO:"你搁这训狗呢"}
                case 0x05: Face_Mode=2; 	Move_Start_Continuous(MOVE_FORWARD, 2); 			break;   //{ID:5,keyword:"命令词",ASR:		"前进",				ASRTO:"我他妈创死你"}
                case 0x06: Face_Mode=2; 	Move_Start_Continuous(MOVE_BEHIND, 2); 				break;   //{ID:6,keyword:"命令词",ASR:		"后退",				ASRTO:"请注意倒车，坦克是没有后视镜的"}
                case 0x07: Face_Mode=2; 	Move_Start_Continuous(MOVE_LEFT, 5);			 	break;   //{ID:7,keyword:"命令词",ASR:		"左转",				ASRTO:"曼波左转"}
                case 0x08: Face_Mode=2; 	Move_Start_Continuous(MOVE_RIGHT, 5); 				break;   //{ID:8,keyword:"命令词",ASR:		"右转",				ASRTO:"曼波右转"}
                case 0x09: Face_Mode=2;	 	Move_Start_Continuous(MOVE_SHAKE_QIANHOU, 1); 		break;   //{ID:9,keyword:"命令词",ASR:		"摇摆",				ASRTO:"曼波傻笑"}
                case 0x10: Face_Mode=4; 	Move_Start_Continuous(MOVE_FAST_SHAKE_QIANHOU, 1); 	break;   //{ID:10,keyword:"命令词",ASR:		"加速",				ASRTO:"wow"}
                case 0x11: Face_Mode=4; 	Move_Start_Continuous(MOVE_FAST_SHAKE_QIANHOU, 1); 	break;   //{ID:11,keyword:"命令词",ASR:		"摇摆加速",			ASRTO:"欧耶"}
                case 0x12: Face_Mode=2;		Move_Start_Continuous(MOVE_WAG_TAIL, 1);			break;   //{ID:12,keyword:"命令词",ASR:		"摇尾巴",			ASRTO:"哈基米"}
                // Hand gesture commands (sent from PC via Bluetooth, see CV/hand_identify.py)
                case 0x21: Face_Mode=4;	Move_Start_Continuous(MOVE_FAST_SHAKE_QIANHOU, 1);	break;   // gesture: open hand (5 fingers)
                case 0x22: Face_Mode=0;	Move_Start_Continuous(MOVE_SLEEP, 1);				break;   // gesture: fist
                case 0x23: Face_Mode=2;	Move_Start_Continuous(MOVE_WAG_TAIL, 1);				break;   // gesture: two fingers (V)
                default:   Move_Stop(); 														break;  // 未知命令，停止所有动作
            }
        }
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, rx_data, sizeof(rx_data));// 重新启动接收
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
    }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  
	HAL_Delay(500);//等待屏幕供电稳定
		
	OLED_Init();  // OLED屏幕初始化
	
	OLED_Clear();
	OLED_ShowImage(0, 0, 64, 64, PDX);
	OLED_ShowImage(64, 0, 64, 64, Liu);
	OLED_Update();
	
	HAL_Delay(3000);
	
	Servo_Init();// 初始化舵机
	Move_Init();
    Move_Start_Continuous(MOVE_SLEEP, 1);

	HAL_UARTEx_ReceiveToIdle_DMA(&huart3, rx_data, sizeof(rx_data));// 接收不定长数据
	__HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);// 关闭DMA传输过半中断
	
	HAL_TIM_Base_Start_IT(&htim4);  // 开启定时器4定时中断
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* 非阻塞式处理各个任务 */
	  OLED_Proc();         // OLED显示处理
	  Move_Process();      // 运动控制处理
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
