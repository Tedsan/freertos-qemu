/*
 * gpio_interrupt.h
 *
 * DIからの割り込みによるタスク起動機能のヘッダファイル
 */

#ifndef GPIO_INTERRUPT_H
#define GPIO_INTERRUPT_H

#include "FreeRTOSConfig.h"
#include "../../Source/include/FreeRTOS.h"
#include "../../Source/include/task.h"

/* GPIOピン定義 */
#define GPIO_PIN_0      (1UL << 0)
#define GPIO_PIN_1      (1UL << 1)
#define GPIO_PIN_2      (1UL << 2)
#define GPIO_PIN_3      (1UL << 3)
#define GPIO_PIN_4      (1UL << 4)
#define GPIO_PIN_5      (1UL << 5)
#define GPIO_PIN_6      (1UL << 6)
#define GPIO_PIN_7      (1UL << 7)

/* 使用するGPIOピン */
#define GPIO_DI_PIN     GPIO_PIN_0

/* 関数プロトタイプ */
void vGPIOInit(void);
void vGPIOInterruptTask(void *pvParameters);
void vKeyInputTask(void *pvParameters);

/* 割り込みハンドラ */
void PORT0_ALL_IRQHandler(void);
void PORT0_0_IRQHandler(void);

#endif /* GPIO_INTERRUPT_H */
