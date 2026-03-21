/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c_slave.h"
#include "data.h"
#include "led.h"
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
void setBoot0(void) {
  // 檢查並設定 BOOT0 選項位元組
  FLASH_OBProgramInitTypeDef OBInit;

  // 讀取當前選項位元組
  HAL_FLASHEx_OBGetConfig(&OBInit);

  // 檢查是否需要修改 BOOT0 設定
  uint32_t currentUserConfig = OBInit.USERConfig;

  // 檢查 nSWBOOT0 和 nBOOT0 位元
  if ((currentUserConfig & FLASH_OPTR_nSWBOOT0) || !(currentUserConfig & FLASH_OPTR_nBOOT0))
  {
    // 需要設定：nSWBOOT0=0 (軟體控制) 和 nBOOT0=1 (BOOT0=0，從主快閃記憶體啟動)

    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();

    OBInit.OptionType = OPTIONBYTE_USER;
    OBInit.USERType = OB_USER_nSWBOOT0 | OB_USER_nBOOT0;

    // 清除 nSWBOOT0 (設為0) 並設定 nBOOT0 (設為1)
    OBInit.USERConfig = (currentUserConfig & ~FLASH_OPTR_nSWBOOT0) | FLASH_OPTR_nBOOT0;

    if (HAL_FLASHEx_OBProgram(&OBInit) == HAL_OK)
    {
      HAL_FLASH_OB_Lock();
      HAL_FLASH_Lock();

      // 重新啟動以應用新的選項位元組
      HAL_FLASH_OB_Launch();
    }
    else
    {
      HAL_FLASH_OB_Lock();
      HAL_FLASH_Lock();
      Error_Handler();
    }
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
  // setBoot0();  // 只在第一次燒錄時需要，之後可以註解掉
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM6_Init();
  MX_ADC2_Init();
  MX_ADC5_Init();
  /* USER CODE BEGIN 2 */

  // 初始化I2C（這個通常沒問題）
  I2C_Slave_Init(&hi2c1);

  // 簡化的ADC初始化
  // LedSet(LED_1, GPIO_PIN_SET);  // 啟動指示
  
  // 暫時禁用DMA中斷進行測試
  HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
  HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);
  HAL_NVIC_DisableIRQ(DMA1_Channel3_IRQn);
  // HAL_NVIC_DisableIRQ(ADC1_2_IRQn);
  // HAL_NVIC_DisableIRQ(ADC5_IRQn);

  // 初始化ADC
  adcInit();
  HAL_TIM_Base_Start(&htim6);  // 啟動定時器6

  // // 系統就緒
  LedSet(LED_4, GPIO_PIN_SET);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    // dataSave();
    // arrangeData();
    dataProcess();
    updateTxBuffer(voltBuffer, VOLT_BUFFER_SIZE);

    // 運行指示
    LedToggle(LED_5);
    HAL_Delay(10);
      
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
//   // 這個回調函數在ADC轉換完成時被呼叫
//   // 可以在這裡處理ADC數據或設定旗標
//   LedSet(LED_3, GPIO_PIN_SET);
// }
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
  while (1) {
    LedSet(LED_5, GPIO_PIN_SET);
    HAL_Delay(100);
    LedSet(LED_5, GPIO_PIN_RESET);
    HAL_Delay(100);
    LedSet(LED_5, GPIO_PIN_SET);
    HAL_Delay(100);
    LedSet(LED_5, GPIO_PIN_RESET);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
