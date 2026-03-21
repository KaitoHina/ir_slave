#include "data.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_adc.h"
#include "led.h"

uint8_t voltBuffer[VOLT_BUFFER_SIZE] = {0};

uint16_t adc1_value[4] = {0};  // ADC1: Vref, ch2, ch1, ch5
uint16_t adc2_value[3] = {0};  // ADC2: ch3, ch4, ch15
uint16_t adc5_value[1] = {0};  // ADC5: ch1

void adcInit() {
  // 校準所有ADC
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc5, ADC_SINGLE_ENDED);
  
  // ADC1 DMA啟動
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1_value, 4);
  // LedSet(LED_4, GPIO_PIN_SET);

  // ADC2 DMA啟動
  HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc2_value, 3);
  // LedSet(LED_5, GPIO_PIN_SET);
  
  // ADC5 DMA啟動
  HAL_ADC_Start_DMA(&hadc5, (uint32_t*)adc5_value, 1);
  // LedSet(LED_6, GPIO_PIN_SET);
}

void dataSave() {
  // __disable_irq();

  // 簡單的DMA數據複製 (8通道完整系統)
  memcpy(&voltBuffer[0], adc1_value, 8);   // ADC1: channels 0-3
  memcpy(&voltBuffer[8], adc2_value, 6);   // ADC2: channels 4-6
  memcpy(&voltBuffer[14], adc5_value, 2);  // ADC5: channel 7

  // __enable_irq();
}

// Process data to remove ambient light effect (max - min algorithm)
void dataProcess() {
  // 移除環境光影響的最大值-最小值算法
  voltBuffer[0] = (uint8_t)(adc1_value[0] & 0xFF);       // Vref LSB
  voltBuffer[1] = (uint8_t)((adc1_value[0] >> 8) & 0xFF); // Vref MSB
  static const uint16_t *address[] = {
    NULL,
    adc1_value + 1,
    adc1_value + 2,
    adc1_value + 3,
    adc2_value + 0,
    adc2_value + 1,
    adc2_value + 2,
    adc5_value + 0
  };
  for (int i = 1; i < EYE_NUM + 1; i++) {
    uint16_t maxVal = 0;
    uint16_t minVal = 0xFFFF;

    __HAL_TIM_SET_COUNTER(&htim6, 0);
    const uint16_t start = __HAL_TIM_GET_COUNTER(&htim6);
    while (__HAL_TIM_GET_COUNTER(&htim6) - start < 833) {
      // 等待833微秒週期結束
      // const uint16_t sample = (uint16_t)(voltBuffer[i * 2] | (voltBuffer[i * 2 + 1] << 8));
      const uint16_t sample = (*address[i]);
      if (sample > maxVal) {
        maxVal = sample;
      }
      if (sample < minVal) {
        minVal = sample;
      }
    }
    const uint16_t processedValue = maxVal - minVal;
    voltBuffer[i * 2]     = (uint8_t)(processedValue & 0xFF);       // LSB
    voltBuffer[i * 2 + 1] = (uint8_t)((processedValue >> 8) & 0xFF); // MSB
    __HAL_TIM_SET_COUNTER(&htim6, 0); // 重置計數器
  }
  arrangeData();
}

/*
eye: 1, 2, 3, 4, 5, 6, 7
buf: 1, 2, 4, 5, 3, 6, 7
*/
void arrangeData() {
  uint8_t arrangedBuf[VOLT_BUFFER_SIZE] = {0};

  // Copy Vref (bytes 0-1)
  arrangedBuf[0] = voltBuffer[0];
  arrangedBuf[1] = voltBuffer[1];

  // Rearranging according to the specified order
  for (int i = 0; i < EYE_NUM; i++) {
    int srcIndex;
    switch (i) {
      case 0: srcIndex = 1; break; // eye 1
      case 1: srcIndex = 2; break; // eye 2
      case 2: srcIndex = 4; break; // eye 3
      case 3: srcIndex = 5; break; // eye 4
      case 4: srcIndex = 3; break; // eye 5
      case 5: srcIndex = 6; break; // eye 6
      case 6: srcIndex = 7; break; // eye 7
      default: srcIndex = i + 1; break;
    }
    // Each sensor data is 2 bytes
    arrangedBuf[(i + 1) * 2]     = voltBuffer[srcIndex * 2];     // LSB
    arrangedBuf[(i + 1) * 2 + 1] = voltBuffer[srcIndex * 2 + 1]; // MSB
  }

  // Copy back to ProcessBuffer
  memcpy(voltBuffer, arrangedBuf, VOLT_BUFFER_SIZE);
}