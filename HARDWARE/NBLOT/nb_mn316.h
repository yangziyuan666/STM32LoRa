#ifndef __NB_MN316_H__
#define __NB_MN316_H__

#include <stdint.h>

// 初始化 MN316 并连接 OneNET
int NB_MN316_Init(const char *dev_name, const char *prod_id, const char *token);

// MN316 发布消息 (内部会自动转 Hex)
int NB_MN316_Pub(const char *topic, const char *payload);

#endif
