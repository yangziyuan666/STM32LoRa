#ifndef __BOBAO_H
#define __BOBAO_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ============ 统一输入结构体 ============ */
typedef struct
{
    /* 节点板异常状态：true=正常 false=异常 */
    bool fall;
    bool vcc;
    bool pir;

    /* 节点状态是否有效（当 tri_state != ST_UNKNOWN 时置 true） */
    bool fall_valid;
    bool vcc_valid;
    bool pir_valid;

    /* 树莓派统计数据 */
    uint16_t person_cnt;
    uint16_t half_cnt;
    uint16_t work_cnt;   /* 目前不处理，但保留方便以后扩展 */

} Bobao_Input_t;

/**
 * @brief 初始化播报模块
 */
void Bobao_Init(void);

/**
 * @brief 统一更新入口：把所有事件交给 bobao.c 去处理
 * @param in  输入结构体（包含节点异常 + 树莓派统计）
 */
void Bobao_UpdateAll(const Bobao_Input_t *in);

#endif
