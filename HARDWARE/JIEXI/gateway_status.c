// gateway_status.c
#include "gateway_status.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static volatile tri_state_t g_fall = ST_UNKNOWN;
static volatile tri_state_t g_vcc  = ST_UNKNOWN;
static volatile tri_state_t g_pir  = ST_UNKNOWN;
static volatile uint16_t g_hr = 0;
static volatile uint16_t g_spo2 = 0;
static volatile uint8_t g_hr_valid = 0;
static volatile uint8_t g_spo2_valid = 0;

static int starts_with_nocase(const char* s, const char* prefix)
{
    while (*prefix) {
        if (tolower((unsigned char)*s) != tolower((unsigned char)*prefix)) {
            return 0;
        }
        ++s;
        ++prefix;
    }
    return 1;
}

static tri_state_t parse_tf(const char* s)
{
    if (!s) return ST_UNKNOWN;
    if (starts_with_nocase(s, "True"))  return ST_TRUE;
    if (starts_with_nocase(s, "False")) return ST_FALSE;
    if (starts_with_nocase(s, "OK"))    return ST_TRUE;
    return ST_UNKNOWN;
}

static void trim_range(const char** begin, const char** end)
{
    while (*begin < *end && isspace((unsigned char)**begin)) {
        (*begin)++;
    }
    while (*end > *begin && isspace((unsigned char)*(*end - 1))) {
        (*end)--;
    }
}

static void apply_kv(const char* key_begin, const char* key_end,
                     const char* val_begin, const char* val_end)
{
    char key[16];
    char val[20];
    size_t klen;
    size_t vlen;
    long num;

    trim_range(&key_begin, &key_end);
    trim_range(&val_begin, &val_end);

    klen = (size_t)(key_end - key_begin);
    vlen = (size_t)(val_end - val_begin);

    if (klen == 0 || vlen == 0) {
        return;
    }

    if (klen >= sizeof(key)) klen = sizeof(key) - 1;
    if (vlen >= sizeof(val)) vlen = sizeof(val) - 1;

    memcpy(key, key_begin, klen);
    key[klen] = '\0';
    memcpy(val, val_begin, vlen);
    val[vlen] = '\0';

    if (strcmp(key, "FALL") == 0) {
        g_fall = parse_tf(val);
    } else if (strcmp(key, "VCC") == 0) {
        g_vcc = parse_tf(val);
    } else if (strcmp(key, "PIR") == 0) {
        g_pir = parse_tf(val);
    } else if (strcmp(key, "HR") == 0) {
        num = strtol(val, NULL, 10);
        if (num >= 0 && num <= 65535) {
            g_hr = (uint16_t)num;
            g_hr_valid = 1;
        }
    } else if (strcmp(key, "SPO2") == 0) {
        num = strtol(val, NULL, 10);
        if (num >= 0 && num <= 65535) {
            g_spo2 = (uint16_t)num;
            g_spo2_valid = 1;
        }
    } else if (strcmp(key, "VALID") == 0) {
        num = strtol(val, NULL, 10);
        if (num == 0) {
            g_hr_valid = 0;
            g_spo2_valid = 0;
        }
    }
}

static void update_status_from_line(const char* line)
{
    const char* p = line;

    while (*p) {
        const char* key_begin;
        const char* key_end;
        const char* val_begin;
        const char* val_end;

        while (*p == ',' || *p == '\r' || *p == '\n' || isspace((unsigned char)*p)) {
            ++p;
        }
        if (*p == '\0') {
            break;
        }

        key_begin = p;
        while (*p && *p != '=' && *p != ',') {
            ++p;
        }
        key_end = p;

        if (*p != '=') {
            while (*p && *p != ',') {
                ++p;
            }
            continue;
        }

        ++p;
        val_begin = p;
        while (*p && *p != ',') {
            ++p;
        }
        val_end = p;

        apply_kv(key_begin, key_end, val_begin, val_end);

        if (*p == ',') {
            ++p;
        }
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

    for (int k = (int)strlen(line) - 1; k >= 0; --k) {
        if (line[k] == '\r' || line[k] == '\n') line[k] = 0;
        else break;
    }

    update_status_from_line(line);
}

tri_state_t GW_Status_GetFall(void) { return g_fall; }
tri_state_t GW_Status_GetVcc(void)  { return g_vcc;  }
tri_state_t GW_Status_GetPir(void)  { return g_pir;  }
uint16_t GW_Status_GetHr(void)      { return g_hr; }
uint16_t GW_Status_GetSpo2(void)    { return g_spo2; }
uint8_t GW_Status_HrValid(void)     { return g_hr_valid; }
uint8_t GW_Status_Spo2Valid(void)   { return g_spo2_valid; }

const char* GW_Status_ToStr(tri_state_t st)
{
    if (st == ST_TRUE)  return "True";
    if (st == ST_FALSE) return "False";
    return "Unknown";
}

