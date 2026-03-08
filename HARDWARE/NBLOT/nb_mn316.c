#include "nb_mn316.h"
#include "lte_at.h"
#include <stdio.h>
#include <string.h>

static char resp[AT_DATA_LEN];
static char hex_buf[1024];

static void str_to_hex(const char *src, char *dest)
{
    while (*src)
    {
        sprintf(dest, "%02X", (unsigned char)*src);
        src++;
        dest += 2;
    }
    *dest = '\0';
}

int NB_MN316_Init(const char *dev_name, const char *prod_id, const char *token)
{
    int ret = -1;
    int cfg_try;
    char cmd[512];

    for (cfg_try = 1; cfg_try <= 3; cfg_try++)
    {
        at.cmd((int8_t *)"AT+MQTTDISC", 11, "OK", NULL, NULL);
        HAL_Delay(150);
        at.cmd((int8_t *)"AT+MQTTDISC=0", 13, "OK", NULL, NULL);
        HAL_Delay(150);
        at.cmd((int8_t *)"AT+MQTTDEL", 10, "OK", NULL, NULL);
        HAL_Delay(150);
        at.cmd((int8_t *)"AT+MQTTDEL=0", 12, "OK", NULL, NULL);
        HAL_Delay(300);

        snprintf(cmd, sizeof(cmd),
                 "AT+MQTTCFG=\"183.230.40.96\",1883,\"%s\",60,\"%s\",\"%s\",1,0",
                 dev_name, prod_id, token);

        ret = at.cmd((int8_t *)cmd, (int32_t)strlen(cmd), "OK", resp, NULL);
        if (ret == 0)
        {
            break;
        }

        printf("[MN316] Config retry %d/3\r\n", cfg_try);
        HAL_Delay(800);
    }

    if (ret != 0)
    {
        printf("[MN316] Config Fail (8002? Check Token/Module State)\r\n");
        return -1;
    }

    ret = at.cmd((int8_t *)"AT+MQTTOPEN=1,1,0,0,0,\"\",\"\"",
                 (int32_t)strlen("AT+MQTTOPEN=1,1,0,0,0,\"\",\"\""), "OK", resp, NULL);

    if (ret != 0)
    {
        printf("[MN316] MQTTOPEN cmd fail\r\n");
        return -2;
    }

    if (LTEAT_wait_urc("+MQTTOPEN:OK", 15000) != 0)
    {
        printf("[MN316] Open Timeout\r\n");
        return -2;
    }

    return 0;
}

int NB_MN316_Pub(const char *topic, const char *payload)
{
    int payload_len = (int)strlen(payload);
    static char pub_cmd[1200];

    if (payload_len * 2 >= (int)sizeof(hex_buf))
    {
        printf("[MN316] Payload too long\r\n");
        return -1;
    }

    str_to_hex(payload, hex_buf);

    snprintf(pub_cmd, sizeof(pub_cmd),
             "AT+MQTTPUB=\"%s\",0,0,0,%d,%s",
             topic, payload_len, hex_buf);

    return at.cmd((int8_t *)pub_cmd, (int32_t)strlen(pub_cmd), "OK", resp, NULL);
}
