#include "lte_ml307.h"
#include <stdio.h>
#include <string.h>

static char resp[AT_DATA_LEN];

int LET_checkDevice(void)
{
    return at.cmd((int8_t*)"AT", 2, "OK", resp, NULL);
}

int LET_pwrdown(void)
{
    return at.cmd((int8_t*)"AT+MREBOOT", 10, "OK", resp, NULL);
}

int LET_setATI(int on)
{
    char cmd[16];
    snprintf(cmd, sizeof(cmd), "ATE%d", on);
    return at.cmd((int8_t*)cmd, (int32_t)strlen(cmd), "OK", resp, NULL);
}

int LET_getCimi(void)
{
    int ret = at.cmd((int8_t*)"AT+CIMI", 7, "OK", resp, NULL);
    printf("[INFO]CIMI:\r\n%s\r\n", resp);
    return ret;
}

int LET_getCGSN(void)
{
    int ret = at.cmd((int8_t*)"AT+CGSN=1", 9, "OK", resp, NULL);
    printf("[INFO]CGSN:\r\n%s\r\n", resp);
    return ret;
}

int LET_getCereg(void)
{
    int ret = at.cmd((int8_t*)"AT+CEREG?", 9, "OK", resp, NULL);
    printf("[INFO]cereg:\r\n%s\r\n", resp);
    return ret;
}

int LET_queryIp(void)
{
    int ret = at.cmd((int8_t*)"AT+CGPADDR=1", 12, "OK", resp, NULL);
    if (ret != 0)
    {
        printf("[INFO]CGPADDR:no response\r\n");
        return -1;
    }
    if (!strstr(resp, "+CGPADDR:"))
    {
        printf("[ERROR] CGPADDR no IP: %s\r\n", resp);
        return -1;
    }
    printf("[INFO]CGPADDR:\r\n%s\r\n", resp);
    return 0;
}

int LET_setAPN(const char *apn)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CGDCONT=1,\"IP\",\"%s\"", apn);
    return at.cmd((int8_t*)cmd, (int32_t)strlen(cmd), "OK", resp, NULL);
}

int LET_activePDP(void)
{
    return at.cmd((int8_t*)"AT+CGACT=1,1", 12, "OK", resp, NULL);
}

int LET_mqtt_cfg_basic(void)
{
    int ret = at.cmd((int8_t*)"AT+MQTTCFG=\"pingresp\",0,1",
                     (int32_t)strlen("AT+MQTTCFG=\"pingresp\",0,1"),
                     "OK", resp, NULL);
    if (ret != 0) return ret;

    ret = at.cmd((int8_t*)"AT+MQTTCFG=\"clean\",0,1",
                 (int32_t)strlen("AT+MQTTCFG=\"clean\",0,1"),
                 "OK", resp, NULL);
    return ret;
}

int LET_mqtt_conn_onenet(const char *ip, int port,
                         const char *clientid,
                         const char *username,
                         const char *password)
{
    char cmd[600];
    int n = snprintf(cmd, sizeof(cmd),
                     "AT+MQTTCONN=0,\"%s\",%d,\"%s\",\"%s\",\"%s\"",
                     ip, port, clientid, username, password);
    if (n <= 0 || n >= (int)sizeof(cmd))
    {
        printf("[ML307] MQTTCONN too long\r\n");
        return 2;
    }
    return at.cmd((int8_t*)cmd, (int32_t)strlen(cmd), "OK", resp, NULL);
}

int LET_mqtt_sub(const char *topic)
{
    char cmd[300];
    int n = snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",0", topic);
    if (n <= 0 || n >= (int)sizeof(cmd))
    {
        printf("[ML307] SUB too long\r\n");
        return 2;
    }
    return at.cmd((int8_t*)cmd, (int32_t)strlen(cmd), "OK", resp, NULL);
}

static char s_cmd_pub[1200];   // 全局静态，不会被栈覆盖

int LET_mqtt_pub_len(const char *topic, const char *payload)
{
    int len = (int)strlen(payload);

    int n = snprintf(s_cmd_pub, sizeof(s_cmd_pub),
                     "AT+MQTTPUB=0,\"%s\",0,0,0,%d,\"%s\"",
                     topic, len, payload);

    if (n <= 0 || n >= (int)sizeof(s_cmd_pub))
    {
        printf("[ML307] PUB too long\r\n");
        return 2;
    }

    printf("[DBG] PUB_CMD=%s\r\n", s_cmd_pub);  // 输出一次看看是否完整
    return at.cmd((int8_t*)s_cmd_pub, n, "OK", resp, NULL);
}

