#ifndef __LTE_ML307_H__
#define __LTE_ML307_H__

#include "lte_def.h"
#include "lte_at.h"

int LET_checkDevice(void);
int LET_pwrdown(void);
int LET_setATI(int on);
int LET_getCimi(void);
int LET_getCGSN(void);
int LET_getCereg(void);
int LET_queryIp(void);

int LET_setAPN(const char *apn);
int LET_activePDP(void);

/* MQTT */
int LET_mqtt_cfg_basic(void);
int LET_mqtt_conn_onenet(const char *ip, int port,
                         const char *clientid,
                         const char *username,
                         const char *password);
int LET_mqtt_sub(const char *topic);
int LET_mqtt_pub_len(const char *topic, const char *payload);

#endif
