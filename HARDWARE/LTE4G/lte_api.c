#include "lte_api.h"
#include "lte_ml307.h"
#include "lte_at.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

ModuleType_t g_module_type = MODULE_UNKNOWN;

static void DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

static int lte_uart5_reinit_baud(uint32_t baud)
{
    HAL_UART_DMAStop(&huart5);
    __HAL_UART_DISABLE_IT(&huart5, UART_IT_IDLE);

    huart5.Init.BaudRate = baud;
    if (HAL_UART_Init(&huart5) != HAL_OK)
    {
        return -1;
    }

    U4_RxFlag = 0;
    U4_RxLen = 0;
    HAL_UART_Receive_DMA(&huart5, U4_RxBuf, U4_RXMAX_SIZE);
    __HAL_UART_CLEAR_IDLEFLAG(&huart5);
    __HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);
    return 0;
}

int lte_comm_init(void)
{
    int ret;
    int probe_ok = 0;
    const uint32_t fixed_baud = 9600;
    char model_buf[AT_DATA_LEN] = {0};

    g_module_type = MODULE_UNKNOWN;

    printf("[COMM] Service init...\r\n");

    at_config cfg = {
        .name = "Common",
        .timeout = 5000,
    };
    at.init(&cfg);

    DelayMs(8000);

    if (lte_uart5_reinit_baud(fixed_baud) != 0)
    {
        printf("[ERROR] UART5 reinit failed, baud=%lu\r\n", (unsigned long)fixed_baud);
        return -1;
    }

    DelayMs(100);
    printf("[INFO] Probe AT at fixed baud=%lu\r\n", (unsigned long)fixed_baud);

    for (int i = 1; i <= 3; i++)
    {
        ret = LET_checkDevice();
        if (ret == 0)
        {
            probe_ok = 1;
            break;
        }
        printf("[WARN] AT retry %d/3 @%lu\r\n", i, (unsigned long)fixed_baud);
        DelayMs(300);
    }

    if (!probe_ok)
    {
        printf("[ERROR] AT no response\r\n");
        return -1;
    }

    printf("[INFO] UART5 baud fixed: %lu\r\n", (unsigned long)fixed_baud);

    if (at.cmd((int8_t *)"AT+CGMM", 7, "OK", model_buf, NULL) == 0)
    {
        printf("[INFO] Model: %s\r\n", model_buf);
    }

    if (strstr(model_buf, "MN316") != NULL)
    {
        g_module_type = MODULE_MN316;
        printf("[INFO] Detected module: NB-IoT (MN316)\r\n");
    }
    else
    {
        g_module_type = MODULE_ML307;
        printf("[INFO] Detected module: 4G Cat.1 (ML307/default)\r\n");
    }

    LET_setATI(0);
    LET_getCimi();
    LET_getCGSN();

    for (int t = 0; t < 20; t++)
    {
        LET_getCereg();
        DelayMs(500);
    }

    if (g_module_type == MODULE_ML307)
    {
        LET_setAPN("CMNET");
        LET_activePDP();
        DelayMs(1500);

        ret = -1;
        for (int i = 1; i <= 5; i++)
        {
            ret = LET_queryIp();
            if (ret == 0)
            {
                break;
            }
            printf("[WARN] CGPADDR retry %d/5\r\n", i);
            DelayMs(1000);
        }

        if (ret != 0)
        {
            printf("[ERROR] No IP\r\n");
            return -1;
        }
    }

    printf("[INFO] Comm ready!\r\n");
    return 0;
}
