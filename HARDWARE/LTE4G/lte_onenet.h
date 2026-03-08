#ifndef __LTE_ONENET_H__
#define __LTE_ONENET_H__

#include <stdint.h>

void LTE_Onenet_Init(void);
void LTE_Onenet_Run(void);

void LTE_Onenet_UpdateData(uint8_t full, uint8_t pir, uint8_t vcc,
                           uint32_t person, uint32_t half, uint32_t work);

#endif
