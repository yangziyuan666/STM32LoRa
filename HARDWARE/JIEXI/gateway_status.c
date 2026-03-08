// gateway_status.c
#include "gateway_status.h"
#include <string.h>

static volatile tri_state_t g_fall = ST_UNKNOWN;
static volatile tri_state_t g_vcc  = ST_UNKNOWN;
static volatile tri_state_t g_pir  = ST_UNKNOWN;

static tri_state_t parse_tf(const char* s)
{
    if (!s) return ST_UNKNOWN;
    if (strncmp(s, "True", 4) == 0)  return ST_TRUE;
    if (strncmp(s, "False", 5) == 0) return ST_FALSE;
    return ST_UNKNOWN;
}

static void update_status_from_line(const char* line)
{
    const char* eq = strchr(line, '=');
    if (!eq) return;

    if (strncmp(line, "FALL", 4) == 0) {
        g_fall = parse_tf(eq + 1);
    } else if (strncmp(line, "VCC", 3) == 0) {
        g_vcc = parse_tf(eq + 1);
    } else if (strncmp(line, "PIR", 3) == 0) {
        g_pir = parse_tf(eq + 1);
    }
}

void GW_Status_ProcessText(const uint8_t* data, uint16_t len)
{
    if (!data || len == 0) return;

    char line[80];
    uint16_t copyLen = len;
    if (copyLen >= sizeof(line)) copyLen = sizeof(line) - 1;

    memcpy(line, data, copyLen);
    line[copyLen] = '\0';

    // È¥µôÄ©Î² \r\n
    for (int k = (int)strlen(line) - 1; k >= 0; --k) {
        if (line[k] == '\r' || line[k] == '\n') line[k] = 0;
        else break;
    }

    update_status_from_line(line);
}

tri_state_t GW_Status_GetFall(void) { return g_fall; }
tri_state_t GW_Status_GetVcc(void)  { return g_vcc;  }
tri_state_t GW_Status_GetPir(void)  { return g_pir;  }

const char* GW_Status_ToStr(tri_state_t st)
{
    if (st == ST_TRUE)  return "True";
    if (st == ST_FALSE) return "False";
    return "Unknown";
}
