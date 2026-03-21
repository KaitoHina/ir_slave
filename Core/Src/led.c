#include "led.h"

static GPIO_TypeDef* const ledPorts[LED_COUNT] = {
  LED1_GPIO_Port, LED2_GPIO_Port, LED3_GPIO_Port,
  LED4_GPIO_Port, LED5_GPIO_Port, LED6_GPIO_Port, LED7_GPIO_Port
};

static const uint16_t ledPins[LED_COUNT] = {
  LED1_Pin, LED2_Pin, LED3_Pin,
  LED4_Pin, LED5_Pin, LED6_Pin, LED7_Pin
};

static int isValidLedIndex(LedIndex index) {
  return (index >= 0) && (index < LED_COUNT);
}

void AllOn(void) {
  for (int i = 0; i < LED_COUNT; i++) {
    HAL_GPIO_WritePin(ledPorts[i], ledPins[i], GPIO_PIN_SET);
  }
}

void AllOff(void) {
  for (int i = 0; i < LED_COUNT; i++) {
    HAL_GPIO_WritePin(ledPorts[i], ledPins[i], GPIO_PIN_RESET);
  }
}

void LedFlash(LedIndex index, int times, int delay) {
  if (!isValidLedIndex(index)) {
    return; // 防止越界
  }
  for (int i = 0; i < times; i++) {
    HAL_GPIO_WritePin(ledPorts[index], ledPins[index], GPIO_PIN_SET);
    HAL_Delay(delay);
    HAL_GPIO_WritePin(ledPorts[index], ledPins[index], GPIO_PIN_RESET);
    HAL_Delay(delay);
  }
}

void LedSet(LedIndex index, GPIO_PinState state) {
  if (!isValidLedIndex(index)) {
    return; // 防止越界
  }
  HAL_GPIO_WritePin(ledPorts[index], ledPins[index], state);
}

void LedToggle(LedIndex index) {
  if (!isValidLedIndex(index)) {
    return; // 防止越界
  }
  HAL_GPIO_TogglePin(ledPorts[index], ledPins[index]);
}