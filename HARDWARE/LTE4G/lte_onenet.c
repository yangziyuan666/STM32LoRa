#include "lte_onenet.h"
#include "lte_ml307.h"
#include "nb_mn316.h"
#include "lte_api.h"
#include "lte_at.h"
#include <stdio.h>
#include <string.h>

#define ONENET_IP    "183.230.40.96"
#define ONENET_PORT  1883

static const char *PRODUCT_ID  = "x9q39zrzXf";
static const char *DEVICE_NAME = "GW001";
static const char *TOKEN =
"version=2018-10-31&res=products%2Fx9q39zrzXf%2Fdevices%2FGW001&et=2000000000&method=md5&sign=2udyiwmx%2BUL%2BEshyq8kcYg%3D%3D";

static const char *pubTopic = "$sys/x9q39zrzXf/GW001/thing/property/post";
static const char *setTopic = "$sys/x9q39zrzXf/GW001/thing/property/set";
static const char *replyTopic = "$sys/x9q39zrzXf/GW001/thing/property/post/reply";

static uint8_t g_mqtt_ready = 0;
static uint32_t g_msg_id = 1;

typedef struct {
    uint8_t  fall;
    uint8_t  pir;
    uint8_t  vcc;
    uint32_t person;
    uint32_t half;
    uint32_t work;
} onenet_data_t;

static volatile onenet_data_t g_data = {0};

void LTE_Onenet_UpdateData(uint8_t fall, uint8_t pir, uint8_t vcc,
                           uint32_t person, uint32_t half, uint32_t work)
{
    g_data.fall = fall;
    g_data.pir = pir;
    g_data.vcc = vcc;
    g_data.person = person;
    g_data.half = half;
    g_data.work = work;
}

void LTE_Onenet_Init(void)
{
    printf("[OneNet] PUB=%s\r\n", pubTopic);
    printf("[OneNet] SET=%s\r\n", setTopic);
    printf("[OneNet] RPY=%s\r\n", replyTopic);

    g_mqtt_ready = 0;

    if (g_module_type == MODULE_ML307)
    {
        printf("[OneNet] Mode: 4G (ML307)\r\n");

        if (LET_mqtt_cfg_basic() != 0)
        {
            printf("[OneNet-4G] MQTTCFG fail\r\n");
            return;
        }

        if (LET_mqtt_conn_onenet(ONENET_IP, ONENET_PORT,
                                 DEVICE_NAME, PRODUCT_ID, TOKEN) != 0)
        {
            printf("[OneNet-4G] MQTTCONN cmd fail\r\n");
            return;
        }

        if (LTEAT_wait_urc("+MQTTURC: \"conn\"", 20000) != 0)
        {
            printf("[OneNet-4G] wait conn timeout\r\n");
            return;
        }

        if (LET_mqtt_sub(setTopic) != 0)
        {
            printf("[OneNet-4G] sub set fail\r\n");
            return;
        }
        LTEAT_wait_urc("+MQTTURC: \"suback\"", 8000);

        if (LET_mqtt_sub(replyTopic) != 0)
        {
            printf("[OneNet-4G] sub reply fail\r\n");
            return;
        }
        LTEAT_wait_urc("+MQTTURC: \"suback\"", 8000);
    }
    else if (g_module_type == MODULE_MN316)
    {
        char sub_cmd[256];
        int nb_ret;

        printf("[OneNet] Mode: NB-IoT (MN316)\r\n");

        nb_ret = NB_MN316_Init(DEVICE_NAME, PRODUCT_ID, TOKEN);
        if (nb_ret != 0)
        {
            printf("[OneNet-NB] Init fail: %d\r\n", nb_ret);
            return;
        }

        snprintf(sub_cmd, sizeof(sub_cmd), "AT+MQTTSUB=\"%s\",0", setTopic);
        at.cmd((int8_t *)sub_cmd, (int32_t)strlen(sub_cmd), "OK", NULL, NULL);
        if (LTEAT_wait_urc("+MQTTSUBACK", 10000) != 0)
        {
            printf("[OneNet-NB] sub set timeout\r\n");
        }

        snprintf(sub_cmd, sizeof(sub_cmd), "AT+MQTTSUB=\"%s\",0", replyTopic);
        at.cmd((int8_t *)sub_cmd, (int32_t)strlen(sub_cmd), "OK", NULL, NULL);
        if (LTEAT_wait_urc("+MQTTSUBACK", 10000) != 0)
        {
            printf("[OneNet-NB] sub reply timeout\r\n");
        }
    }
    else
    {
        printf("[OneNet] Unknown module type\r\n");
        return;
    }

    g_mqtt_ready = 1;
    printf("[OneNet] MQTT READY\r\n");
}

void LTE_Onenet_Run(void)
{
    static uint32_t tick = 0;
    char payload[512];
    int n;
    int ret = -1;

    LTEAT_drain_print_once();

    if (!g_mqtt_ready)
    {
        return;
    }

    if (HAL_GetTick() - tick < 5000)
    {
        return;
    }
    tick = HAL_GetTick();

    n = snprintf(payload, sizeof(payload),
            "{\"id\":\"%lu\",\"version\":\"1.0\",\"params\":{"
                "\"PIR\":{\"value\":%s},"
                "\"VCC\":{\"value\":%s},"
                "\"FALL\":{\"value\":%s},"
                "\"person\":{\"value\":%lu},"
                "\"half\":{\"value\":%lu},"
                "\"work\":{\"value\":%lu}"
            "}}",
            (unsigned long)g_msg_id++,
            g_data.pir  ? "true" : "false",
            g_data.vcc  ? "true" : "false",
            g_data.fall ? "true" : "false",
            (unsigned long)g_data.person,
            (unsigned long)g_data.half,
            (unsigned long)g_data.work);

    if (n <= 0 || n >= (int)sizeof(payload))
    {
        printf("[OneNet] payload too long\r\n");
        return;
    }

    if (g_module_type == MODULE_ML307)
    {
        ret = LET_mqtt_pub_len(pubTopic, payload);
        if (ret == 0)
        {
            printf("[OneNet-4G] Pub OK\r\n");
        }
        else
        {
            printf("[OneNet-4G] Pub FAIL\r\n");
        }
    }
    else if (g_module_type == MODULE_MN316)
    {
        ret = NB_MN316_Pub(pubTopic, payload);
        if (ret == 0)
        {
            printf("[OneNet-NB] Pub OK\r\n");
        }
        else
        {
            printf("[OneNet-NB] Pub FAIL\r\n");
        }
    }
}
