/*
 * gpio_interrupt.c
 *
 * DIからの割り込みによるタスク起動機能の実装
 */

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "gpio_interrupt.h"
#include "CMSIS/CMSDK_CM3.h"
#include "CMSIS/SMM_MPS2.h"

/* タスク通知に使用するタスクハンドル */
static TaskHandle_t xInterruptTaskHandle = NULL;

/* キー入力処理タスク用のハンドル */
static TaskHandle_t xKeyInputTaskHandle = NULL;

/*-----------------------------------------------------------*/
/* GPIOの初期化と割り込み設定 */
void vGPIOInit(void)
{
    /* GPIO0をデジタル入力として設定 */
    CMSDK_GPIO0->OUTENABLECLR = GPIO_DI_PIN;  /* 出力を無効化（入力モード） */
    CMSDK_GPIO0->ALTFUNCCLR = GPIO_DI_PIN;    /* 代替機能を無効化 */
    
    /* 割り込み設定 */
    CMSDK_GPIO0->INTENSET = GPIO_DI_PIN;      /* 割り込みを有効化 */
    CMSDK_GPIO0->INTTYPESET = GPIO_DI_PIN;    /* エッジ検出を設定 */
    CMSDK_GPIO0->INTPOLSET = GPIO_DI_PIN;     /* 立ち上がりエッジで検出 */
    
    /* NVIC設定 */
    NVIC_SetPriority(PORT0_ALL_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PORT0_ALL_IRQn);
    
    printf("GPIO初期化完了: ピン%dが割り込み用に設定されました\r\n", __builtin_ctz(GPIO_DI_PIN));
}

/*-----------------------------------------------------------*/
/* 割り込みハンドラ - すべてのGPIO0ピンの割り込みを処理 */
void PORT0_ALL_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t ulInterruptStatus;
    
    /* 割り込みステータスを取得 */
    ulInterruptStatus = CMSDK_GPIO0->INTSTATUS;
    
    /* 対象のピンの割り込みかチェック */
    if ((ulInterruptStatus & GPIO_DI_PIN) != 0)
    {
        /* 割り込みフラグをクリア */
        CMSDK_GPIO0->INTCLEAR = GPIO_DI_PIN;
        
        /* タスク通知を送信 */
        if (xInterruptTaskHandle != NULL)
        {
            /* 通知値として割り込みが発生したピン番号を送信 */
            xTaskNotifyFromISR(xInterruptTaskHandle, 
                              GPIO_DI_PIN, 
                              eSetBits, 
                              &xHigherPriorityTaskWoken);
            
            /* コンテキストスイッチが必要な場合は要求 */
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/*-----------------------------------------------------------*/
/* 個別のGPIOピン0の割り込みハンドラ（使用する場合） */
void PORT0_0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* 割り込みフラグをクリア */
    CMSDK_GPIO0->INTCLEAR = GPIO_PIN_0;
    
    /* タスク通知を送信 */
    if (xInterruptTaskHandle != NULL)
    {
        /* 通知値として割り込みが発生したピン番号を送信 */
        xTaskNotifyFromISR(xInterruptTaskHandle, 
                          GPIO_PIN_0, 
                          eSetBits, 
                          &xHigherPriorityTaskWoken);
        
        /* コンテキストスイッチが必要な場合は要求 */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/*-----------------------------------------------------------*/
/* キー入力処理タスク - 'i'キーが押されたら割り込みをシミュレート */
void vKeyInputTask(void *pvParameters)
{
    int c;
    
    printf("キー入力処理タスクが開始されました（'i'キーで割り込みをシミュレート）\r\n");
    
    for (;;)
    {
        c = getchar();
        
        /* 'i'キーが押されたら割り込みをシミュレート */
        if (c == 'i' || c == 'I')
        {
            printf("キー入力による割り込みシミュレーション\r\n");
            
            /* 割り込みをシミュレート */
            printf("キー入力による割り込みシミュレーション\r\n");
            
            /* 割り込みハンドラを直接呼び出し */
            /* 割り込みステータスを設定（実際のハードウェアでは自動的に設定される） */
            /* 注: INTSTATUSは読み取り専用なので、テスト用に直接ハンドラを呼び出す */
            
            /* タスク通知を直接送信 */
            if (xInterruptTaskHandle != NULL)
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                
                /* 通知値として割り込みが発生したピン番号を送信 */
                xTaskNotify(xInterruptTaskHandle, 
                           GPIO_DI_PIN, 
                           eSetBits);
            }
        }
        
        /* 少し待機 */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*-----------------------------------------------------------*/
/* 割り込み処理タスク */
void vGPIOInterruptTask(void *pvParameters)
{
    uint32_t ulNotificationValue;
    
    /* タスクハンドルを保存 */
    xInterruptTaskHandle = xTaskGetCurrentTaskHandle();
    
    printf("割り込み処理タスクが開始されました\r\n");
    
    for (;;)
    {
        /* タスク通知を待機 */
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY) == pdTRUE)
        {
            /* 通知を受信した場合、どのピンで割り込みが発生したかを確認 */
            if ((ulNotificationValue & GPIO_DI_PIN) != 0)
            {
                /* メッセージを出力 */
                printf("GPIO割り込みが検出されました: ピン%d\r\n", __builtin_ctz(GPIO_DI_PIN));
            }
        }
    }
}
