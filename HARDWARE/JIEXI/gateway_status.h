// gateway_status.h
#ifndef __GATEWAY_STATUS_H
#define __GATEWAY_STATUS_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ST_UNKNOWN = 0,
    ST_TRUE,
    ST_FALSE
} tri_state_t;

/**
 * @brief  处理一帧节点数据（不含Addr的纯文本部分也行）
 * @param  data: 指向ASCII文本，比如 "FALL=True\r\n"
 * @param  len : 文本长度
 */
void GW_Status_ProcessText(const uint8_t* data, uint16_t len);

/**
 * @brief  获取三路状态（可选用）
 */
tri_state_t GW_Status_GetFall(void);
tri_state_t GW_Status_GetVcc(void);
tri_state_t GW_Status_GetPir(void);

/**
 * @brief  把状态转成字符串 "True/False/Unknown"
 */
const char* GW_Status_ToStr(tri_state_t st);

#ifdef __cplusplus
}
#endif

#endif
