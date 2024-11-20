/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TIM_HandleTypeDef htim1;
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma1;
UART_HandleTypeDef huart1;

volatile int a = 0;
osThreadId_t PrintTaskHandle;
uint32_t PrintTaskBuffer[ 512 ];
osStaticThreadDef_t PrintTaskControlBlock;
const osThreadAttr_t PrintTask_attributes = {
  .name = "PrintTask",
  .cb_mem = &PrintTaskControlBlock,
  .cb_size = sizeof(PrintTaskControlBlock),
  .stack_mem = &PrintTaskBuffer[0],
  .stack_size = sizeof(PrintTaskBuffer),
  .priority = (osPriority_t) osPriorityHigh,
};

//to do: declare RTOS object to commute between ISR and RTOS task

#define SAMPLE_BUFFER_SIZE 256
uint16_t sample_buffer[SAMPLE_BUFFER_SIZE];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  osStatus_t status;
  uint8_t mode;
  uint8_t start;
  uint8_t end;
  int i = 0;
  printf("kjghfajkdgfh\n");
  for(;;)
  {


	  if(a == 1){//print first half
		  start = 0;
		  end = 127;
		  for(i = start; i<=end; i++){
			  printf("%d\r\n", sample_buffer[i]);
		  }
	  }else if(a==2){
		  start = 128;
		  end = 255;
		  for(i = start; i<=end; i++)printf("%d\r\n", sample_buffer[i]);
	  }


    osDelay(10);
  }
  /* USER CODE END 5 */
}
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

void Error_Handler()
{
    __disable_irq();
    while (1)
    {

    }
}


static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_ADC_CLK_ENABLE();
    }

}


void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance==ADC1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_ADC_CLK_DISABLE();
    }

}

void ADC1_Init(void)
{
    ADC_MultiModeTypeDef multimode = {0};
    ADC_ChannelConfTypeDef sConfig = {0};

  /** Common config
  */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;//ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }

  /** Configure the ADC multi-mode
  */
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
    {
        Error_Handler();
    }

  /** Configure Regular Channel
  */
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;//ADC_SAMPLETIME_247CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{

	printf("1\r\n");
	a=1;

}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{

	printf("2\r\n");
	//for (int i=0;i<128;i++)printf("%d, ", sample_buffer[i]);
    //to do: inform RTOS task to print out sensor data
    a = 2;
}

void DMA1_Channel1_IRQHandler(void)
{
    SET_BIT(hadc1.Instance->CFGR, ADC_CFGR_DMACFG);
    HAL_DMA_IRQHandler(&hdma1);
}

void ADC1_DMA1CH1_init()
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma1.Instance = DMA1_Channel1;
    hdma1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma1.Init.MemInc = DMA_MINC_ENABLE;
    hdma1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma1.Init.Mode = DMA_CIRCULAR;
    hdma1.Init.Priority = DMA_PRIORITY_HIGH;

    if (HAL_DMA_Init(&hdma1) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma1);

    ADC1_Init();

    NVIC_SetVector(DMA1_Channel1_IRQn, (uint32_t)&DMA1_Channel1_IRQHandler);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}



void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    if(htim_base->Instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
    if(htim_base->Instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_DISABLE();
    }
}


static void TIM1_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 40 - 1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 1000 - 1;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
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

  MX_USART1_UART_Init();


    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    //MX_GPIO_Init();



  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
   printf("sajkhfkjdhk\r\n");
   osKernelInitialize();
   PrintTaskHandle = osThreadNew(StartDefaultTask, NULL, &PrintTask_attributes);
   if(PrintTaskHandle == NULL)printf("error\n");
   else printf("%d", PrintTaskHandle);
   TIM1_Init();
   ADC1_DMA1CH1_init();
  /* USER CODE END 2 */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sample_buffer[0], SAMPLE_BUFFER_SIZE);
    HAL_TIM_Base_Start_IT(&htim1);
      printf("123\n");
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

      osKernelStart();
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
