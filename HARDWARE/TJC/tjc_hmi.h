/* tjc_hmi.h */
#ifndef __TJC_HMI_H__
#define __TJC_HMI_H__

#include "main.h"

/* * 陶晶驰串口屏驱动封装
 * 依赖: USART6 (在 usart.c 中初始化)
 */

// ---------------- 基础驱动函数 ----------------

/**
 * @brief 修改文本控件内容 (例如: t0.txt="Hello")
 * @param objName 控件名称 (如 "t0")
 * @param str     显示的字符串
 */
void TJC_SetText(const char* objName, const char* str);

/**
 * @brief 修改数字控件数值 (例如: n0.val=123)
 * @param objName 控件名称 (如 "n0")
 * @param val     显示的数值
 */
void TJC_SetNumber(const char* objName, int val);


// ---------------- 业务逻辑函数 ----------------

/**
 * @brief 一键更新主界面所有数据
 * @param fall_str  跌倒状态字符串
 * @param vcc_str   电源状态字符串
 * @param pir_str   红外状态字符串
 * @param person    总人数
 * @param half      半身数
 * @param work      工作数
 */
void TJC_UpdateMainPage(const char* fall_str, const char* vcc_str, const char* pir_str, 
                        uint32_t person, uint32_t half, uint32_t work);

#endif /* __TJC_HMI_H__ */

