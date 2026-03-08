#include "bobao.h"
#include "usart.h"
#include <string.h>

/* 你用的是串口3 TX发送给天问 */
extern UART_HandleTypeDef huart3;

/* ===================== 内部状态缓存 ===================== */

/* 节点状态：true=正常 false=异常 */
static bool last_fall = true;
static bool last_vcc  = true;
static bool last_pir  = true;

/* 是否已经收到过有效数据（UNKNOWN过滤关键点） */
static bool last_fall_valid = false;
static bool last_vcc_valid  = false;
static bool last_pir_valid  = false;

/* P/H 上一次计数 */
static uint16_t last_person_cnt = 0;
static uint16_t last_half_cnt   = 0;

/* ===================== 内部发送函数 ===================== */
static void Bobao_Send(const char *cmd)
{
    if (cmd == NULL) return;
    uint16_t len = strlen(cmd);
    if (len == 0) return;
	  u1_printf("[BOBAO] send to Tianwen: %s\r\n", cmd);

    /* 纯字符串发送，不加\r\n */
    HAL_UART_Transmit(&huart3, (uint8_t*)cmd, len, 100);
}

/* ===================== API ===================== */

void Bobao_Init(void)
{
    last_fall = true;
    last_vcc  = true;
    last_pir  = true;

    last_fall_valid = false;
    last_vcc_valid  = false;
    last_pir_valid  = false;

    last_person_cnt = 0;
    last_half_cnt   = 0;
}

void Bobao_UpdateAll(const Bobao_Input_t *in)
{
    if (in == NULL) return;

    /* ========== 1) 节点板事件处理（UNKNOWN过滤） ========== */

    /* FALL */
    if (in->fall_valid)
    {
        if (last_fall_valid)
        {
            /* true->false 才播报 */
            if ((last_fall == true) && (in->fall == false))
                Bobao_Send("diaolan");
        }
        else
        {
            /* 第一次收到有效数据：如果直接就是异常(false)，也要播报一次 */
            if (in->fall == false)
                Bobao_Send("diaolan");
        }

        last_fall = in->fall;
        last_fall_valid = true;
    }

    /* VCC */
    if (in->vcc_valid)
    {
        if (last_vcc_valid)
        {
            if ((last_vcc == true) && (in->vcc == false))
                Bobao_Send("diansuo");
        }
        else
        {
            if (in->vcc == false)
                Bobao_Send("diansuo");
        }

        last_vcc = in->vcc;
        last_vcc_valid = true;
    }

    /* PIR */
    if (in->pir_valid)
    {
        if (last_pir_valid)
        {
            if ((last_pir == true) && (in->pir == false))
                Bobao_Send("reshidian");
        }
        else
        {
            if (in->pir == false)
                Bobao_Send("reshidian");
        }

        last_pir = in->pir;
        last_pir_valid = true;
    }

    /* ========== 2) 树莓派 P/H 事件处理 ========== */

    if ((last_person_cnt == 0) && (in->person_cnt != 0))
        Bobao_Send("person");

    if ((last_half_cnt == 0) && (in->half_cnt != 0))
        Bobao_Send("half");

    last_person_cnt = in->person_cnt;
    last_half_cnt   = in->half_cnt;
}


