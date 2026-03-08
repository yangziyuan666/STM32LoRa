#ifndef __SHUMEIPAI_H
#define __SHUMEIPAI_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t person;
    uint16_t half;
    uint16_t work;
    uint32_t last_update_tick;   // HAL_GetTick() when last valid line parsed
    uint32_t bad_line_cnt;       // count of parse failures (debug)
} ShumeiPi_Counts_t;

/**
 * Init parser module.
 * Call once in main after MX_USART3_UART_Init().
 */
void ShumeiPi_Init(void);

/**
 * Feed one received byte into parser.
 * Call this in USART3 RX interrupt callback.
 */
void ShumeiPi_OnRxByte(uint8_t b);

/**
 * Get last parsed counts (copy-out).
 * Thread-safe enough for ISR+main usage (uses atomic copy pattern).
 */
void ShumeiPi_GetCounts(ShumeiPi_Counts_t *out);

#ifdef __cplusplus
}
#endif

#endif
