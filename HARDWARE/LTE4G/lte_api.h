#ifndef __LTE_API_H__
#define __LTE_API_H__

#include "main.h"

typedef enum {
    MODULE_UNKNOWN = 0,
    MODULE_ML307,
    MODULE_MN316
} ModuleType_t;

extern ModuleType_t g_module_type;

int lte_comm_init(void);

#endif
