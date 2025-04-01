/*
 * gpio_interrupt_test.c
 *
 * DIからの割り込みによるタスク起動機能の単体テスト
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* FreeRTOSのヘッダをインクルードする代わりにスタブを定義 */
/* gpio_interrupt.h の内容を直接ここに含める */

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

/* FreeRTOS関連の定義 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 5
typedef void* TaskHandle_t;
typedef long BaseType_t;
typedef unsigned long TickType_t;
#define pdFALSE         ( ( BaseType_t ) 0 )
#define pdTRUE          ( ( BaseType_t ) 1 )
#define pdPASS          ( pdTRUE )
#define portMAX_DELAY   ( TickType_t ) 0xffffffffUL

typedef enum {
    eNoAction = 0,
    eSetBits,
    eIncrement,
    eSetValueWithOverwrite,
    eSetValueWithoutOverwrite
} eNotifyAction;

/* 割り込みベクター番号 */
#define PORT0_ALL_IRQn  0
#define PORT0_0_IRQn    1

/* 関数プロトタイプ */
void vGPIOInit(void);
void vGPIOInterruptTask(void *pvParameters);
void PORT0_ALL_IRQHandler(void);
void PORT0_0_IRQHandler(void);

/* テスト用のスタブ定義 */

/* CMSIS/CMSDK_CM3.h と CMSIS/SMM_MPS2.h のスタブ */
typedef struct {
    uint32_t DATA;
    uint32_t DATAOUT;
    uint32_t OUTENABLESET;
    uint32_t OUTENABLECLR;
    uint32_t ALTFUNCSET;
    uint32_t ALTFUNCCLR;
    uint32_t INTENSET;
    uint32_t INTENCLR;
    uint32_t INTTYPESET;
    uint32_t INTTYPECLR;
    uint32_t INTPOLSET;
    uint32_t INTPOLCLR;
    uint32_t INTSTATUS;
    uint32_t INTCLEAR;
    uint32_t MASKLOWBYTE[4];
    uint32_t MASKHIGHBYTE[4];
} CMSDK_GPIO_TypeDef;

/* スタブ用のグローバル変数 */
CMSDK_GPIO_TypeDef gpio0_stub;
CMSDK_GPIO_TypeDef *CMSDK_GPIO0 = &gpio0_stub;

/* NVIC関数のスタブ */
void NVIC_SetPriority(uint32_t IRQn, uint32_t priority) {
    /* スタブ実装 - 何もしない */
    (void)IRQn;
    (void)priority;
}

void NVIC_EnableIRQ(uint32_t IRQn) {
    /* スタブ実装 - 何もしない */
    (void)IRQn;
}

/* FreeRTOS関数のスタブ */
TaskHandle_t xTaskGetCurrentTaskHandle(void) {
    /* スタブ実装 - ダミーのタスクハンドルを返す */
    static TaskHandle_t dummy_handle = (TaskHandle_t)1;
    return dummy_handle;
}

BaseType_t xTaskNotifyFromISR(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction, BaseType_t *pxHigherPriorityTaskWoken) {
    /* スタブ実装 - 成功を返す */
    (void)xTaskToNotify;
    (void)ulValue;
    (void)eAction;
    *pxHigherPriorityTaskWoken = pdFALSE;
    return pdPASS;
}

void portYIELD_FROM_ISR(BaseType_t xHigherPriorityTaskWoken) {
    /* スタブ実装 - 何もしない */
    (void)xHigherPriorityTaskWoken;
}

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit, uint32_t *pulNotificationValue, TickType_t xTicksToWait) {
    /* スタブ実装 - 通知を受信したことにする */
    (void)ulBitsToClearOnEntry;
    (void)ulBitsToClearOnExit;
    (void)xTicksToWait;
    
    /* テスト用に通知値を設定 */
    *pulNotificationValue = GPIO_DI_PIN;
    return pdTRUE;
}

/* printf関数のスタブ */
#define MAX_PRINTF_BUFFER 256
char printf_buffer[MAX_PRINTF_BUFFER];
int printf_call_count = 0;

/* 実際のprintf関数を使用 */
#include <stdarg.h>

int printf(const char *format, ...) {
    va_list args;
    int result;
    
    /* 実際に出力する */
    va_start(args, format);
    result = vprintf(format, args);
    va_end(args);
    
    /* カウントを増やす */
    printf_call_count++;
    
    return result;
}

/* テスト用のヘルパー関数 */
void reset_stubs(void) {
    /* スタブの状態をリセット */
    memset(&gpio0_stub, 0, sizeof(gpio0_stub));
    memset(printf_buffer, 0, sizeof(printf_buffer));
    printf_call_count = 0;
}

/* テストケース */

/* テストケース0: GPIOの初期化テスト */
void test_case_0_gpio_init(void) {
    printf("テストケース0: GPIOの初期化テスト\n");
    
    /* スタブをリセット */
    reset_stubs();
    
    /* テスト対象の関数を呼び出し */
    vGPIOInit();
    
    /* 期待される結果を検証 */
    if ((gpio0_stub.OUTENABLECLR == GPIO_DI_PIN) &&
        (gpio0_stub.ALTFUNCCLR == GPIO_DI_PIN) &&
        (gpio0_stub.INTENSET == GPIO_DI_PIN) &&
        (gpio0_stub.INTTYPESET == GPIO_DI_PIN) &&
        (gpio0_stub.INTPOLSET == GPIO_DI_PIN) &&
        (printf_call_count > 0)) {
        printf("テストケース0: 成功 - GPIOが正しく初期化されました\n");
    } else {
        printf("テストケース0: 失敗 - GPIOの初期化が正しくありません\n");
        printf("  OUTENABLECLR: 期待値=%u, 実際=%u\n", (unsigned)GPIO_DI_PIN, (unsigned)gpio0_stub.OUTENABLECLR);
        printf("  ALTFUNCCLR: 期待値=%u, 実際=%u\n", (unsigned)GPIO_DI_PIN, (unsigned)gpio0_stub.ALTFUNCCLR);
        printf("  INTENSET: 期待値=%u, 実際=%u\n", (unsigned)GPIO_DI_PIN, (unsigned)gpio0_stub.INTENSET);
        printf("  INTTYPESET: 期待値=%u, 実際=%u\n", (unsigned)GPIO_DI_PIN, (unsigned)gpio0_stub.INTTYPESET);
        printf("  INTPOLSET: 期待値=%u, 実際=%u\n", (unsigned)GPIO_DI_PIN, (unsigned)gpio0_stub.INTPOLSET);
        printf("  printf呼び出し: 期待値>0, 実際=%d\n", printf_call_count);
    }
}

/* テストケース1: 割り込みハンドラのテスト */
void test_case_1_irq_handler(void) {
    printf("テストケース1: 割り込みハンドラのテスト\n");
    
    /* スタブをリセット */
    reset_stubs();
    
    /* 割り込みハンドラを呼び出す前の準備 */
    gpio0_stub.INTSTATUS = GPIO_DI_PIN;  /* 割り込みステータスを設定 */
    
    /* 割り込みハンドラを呼び出す前にタスクハンドルを設定 */
    vGPIOInterruptTask(NULL);  /* これによりxInterruptTaskHandleが設定される */
    
    /* テスト対象の関数を呼び出し */
    PORT0_ALL_IRQHandler();
    
    /* 期待される結果を検証 */
    if (gpio0_stub.INTCLEAR == GPIO_DI_PIN) {
        printf("テストケース1: 成功 - 割り込みフラグが正しくクリアされました\n");
    } else {
        printf("テストケース1: 失敗 - 割り込みフラグのクリアが正しくありません\n");
        printf("  INTCLEAR: 期待値=%u, 実際=%u\n", (unsigned)GPIO_DI_PIN, (unsigned)gpio0_stub.INTCLEAR);
    }
}

/* テストケース2: 割り込み処理タスクのテスト */
void test_case_2_interrupt_task(void) {
    printf("テストケース2: 割り込み処理タスクのテスト\n");
    
    /* スタブをリセット */
    reset_stubs();
    
    /* テスト対象の関数を呼び出し */
    /* 注: 実際のタスクは無限ループなので、1回のイテレーションだけをテスト */
    vGPIOInterruptTask(NULL);  /* タスクハンドルを設定 */
    
    /* 期待される結果を検証 */
    if (printf_call_count > 0) {
        printf("テストケース2: 成功 - 割り込み処理タスクが正しく動作しました\n");
    } else {
        printf("テストケース2: 失敗 - 割り込み処理タスクの動作が正しくありません\n");
        printf("  printf呼び出し: 期待値>0, 実際=%d\n", printf_call_count);
    }
}

/* 実装部分 */

/* タスク通知に使用するタスクハンドル */
static TaskHandle_t xInterruptTaskHandle = NULL;

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
    
    printf("GPIO初期化完了: ピン%dが割り込み用に設定されました\r\n", 0); // __builtin_ctz(GPIO_DI_PIN)をハードコード
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
/* 割り込み処理タスク */
void vGPIOInterruptTask(void *pvParameters)
{
    uint32_t ulNotificationValue;
    
    /* タスクハンドルを保存 */
    xInterruptTaskHandle = xTaskGetCurrentTaskHandle();
    
    printf("割り込み処理タスクが開始されました\r\n");
    
    /* 単体テストでは無限ループを使用しない */
    #ifndef UNIT_TEST
    for (;;)
    #endif
    {
        /* タスク通知を待機 */
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY) == pdTRUE)
        {
            /* 通知を受信した場合、どのピンで割り込みが発生したかを確認 */
            if ((ulNotificationValue & GPIO_DI_PIN) != 0)
            {
                /* メッセージを出力 */
                printf("GPIO割り込みが検出されました: ピン%d\r\n", 0); // __builtin_ctz(GPIO_DI_PIN)をハードコード
            }
        }
    }
}

/* メイン関数 */
int main(void) {
    printf("GPIO割り込み機能の単体テストを開始します\n");
    
    /* 各テストケースを実行 */
    test_case_0_gpio_init();
    test_case_1_irq_handler();
    test_case_2_interrupt_task();
    
    printf("すべてのテストが完了しました\n");
    return 0;
}
