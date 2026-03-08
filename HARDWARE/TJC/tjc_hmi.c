/* tjc_hmi.c */
#include "tjc_hmi.h"
#include "usart.h"      // 引用 huart6
#include <stdio.h>
#include <stdarg.h>     // 用于参数格式化
#include <string.h>

// 引用外部的串口句柄，确保 usart.h 中有 extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart6; 

// 串口屏固定的结束符: 0xFF 0xFF 0xFF
static uint8_t TJC_EndSeq[3] = {0xFF, 0xFF, 0xFF};

/**
 * @brief 内部函数：发送格式化指令到屏幕
 * @note  类似于 printf，会自动添加结束符
 */
static void TJC_Printf(const char* fmt, ...)
{
    char buf[128];  // 缓冲区，如果指令特别长请适当调大
    va_list args;
    
    // 1. 格式化字符串
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    // 2. 发送指令主体
    HAL_UART_Transmit(&huart6, (uint8_t*)buf, strlen(buf), 50);
    
    // 3. 发送结束符
    HAL_UART_Transmit(&huart6, TJC_EndSeq, 3, 50);
}

/* ---------------- 接口实现 ---------------- */

void TJC_SetText(const char* objName, const char* str)
{
    // 格式: objName.txt="str"
    TJC_Printf("%s.txt=\"%s\"", objName, str);
}

void TJC_SetNumber(const char* objName, int val)
{
    // 格式: objName.val=val
    TJC_Printf("%s.val=%d", objName, val);
}

void TJC_UpdateMainPage(const char* fall_str, const char* vcc_str, const char* pir_str, 
                        uint32_t person, uint32_t half, uint32_t work)
{
    // 1. 更新三个文本状态 (t0, t1, t2)
    TJC_SetText("t0", fall_str);
    TJC_SetText("t1", vcc_str);
    TJC_SetText("t2", pir_str);
    
    // 2. 更新三个统计数字 (n0, n1, n2)
    TJC_SetNumber("n0", (int)person);
    TJC_SetNumber("n1", (int)half);
    TJC_SetNumber("n2", (int)work);
}

