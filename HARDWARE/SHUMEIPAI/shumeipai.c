#include "shumeipai.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define SHUMEIPI_LINE_MAX  32  // enough for "65535,65535,65535\n"

static volatile uint16_t g_person = 0;
static volatile uint16_t g_half   = 0;
static volatile uint16_t g_work   = 0;
static volatile uint32_t g_last_tick = 0;
static volatile uint32_t g_bad_cnt = 0;

/* line buffer (filled in ISR) */
static uint8_t  s_line[SHUMEIPI_LINE_MAX];
static uint16_t s_len = 0;

static int parse_csv_counts(const char *line, uint16_t *p, uint16_t *h, uint16_t *w)
{
    // Accept formats like: "1,2,3", " 1, 2, 3\r"
    // We will copy and strip trailing \r \n.
    char buf[SHUMEIPI_LINE_MAX];
    size_t n = strlen(line);
    if (n == 0 || n >= sizeof(buf)) return 0;

    // copy
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // strip trailing spaces and \r \n
    for (int i = (int)strlen(buf) - 1; i >= 0; --i) {
        if (buf[i] == '\r' || buf[i] == '\n' || isspace((unsigned char)buf[i])) {
            buf[i] = '\0';
        } else {
            break;
        }
    }

    // split by commas
    char *s1 = buf;
    char *c1 = strchr(s1, ',');
    if (!c1) return 0;
    *c1 = '\0';

    char *s2 = c1 + 1;
    char *c2 = strchr(s2, ',');
    if (!c2) return 0;
    *c2 = '\0';

    char *s3 = c2 + 1;

    // trim leading spaces
    while (*s1 && isspace((unsigned char)*s1)) s1++;
    while (*s2 && isspace((unsigned char)*s2)) s2++;
    while (*s3 && isspace((unsigned char)*s3)) s3++;

    char *endp;

    long v1 = strtol(s1, &endp, 10);
    while (*endp && isspace((unsigned char)*endp)) endp++;
    if (*endp != '\0') return 0;

    long v2 = strtol(s2, &endp, 10);
    while (*endp && isspace((unsigned char)*endp)) endp++;
    if (*endp != '\0') return 0;

    long v3 = strtol(s3, &endp, 10);
    while (*endp && isspace((unsigned char)*endp)) endp++;
    if (*endp != '\0') return 0;

    if (v1 < 0 || v2 < 0 || v3 < 0) return 0;
    if (v1 > 65535 || v2 > 65535 || v3 > 65535) return 0;

    *p = (uint16_t)v1;
    *h = (uint16_t)v2;
    *w = (uint16_t)v3;
    return 1;
}

void ShumeiPi_Init(void)
{
    s_len = 0;
    memset((void*)s_line, 0, sizeof(s_line));

    g_person = 0;
    g_half = 0;
    g_work = 0;
    g_last_tick = 0;
    g_bad_cnt = 0;
}

void ShumeiPi_OnRxByte(uint8_t b)
{
    // line end: '\n' (Pi sends \n). Also accept '\r' as end.
    if (b == '\n' || b == '\r') {
        if (s_len == 0) {
            return; // ignore empty line
        }

        // null-terminate
        if (s_len >= SHUMEIPI_LINE_MAX) {
            s_len = SHUMEIPI_LINE_MAX - 1;
        }
        s_line[s_len] = '\0';

        uint16_t p, h, w;
        if (parse_csv_counts((char*)s_line, &p, &h, &w)) {
            g_person = p;
            g_half   = h;
            g_work   = w;
            g_last_tick = HAL_GetTick();
        } else {
            g_bad_cnt++;
        }

        // reset buffer for next line
        s_len = 0;
        return;
    }

    // normal byte
    if (s_len < (SHUMEIPI_LINE_MAX - 1)) {
        s_line[s_len++] = b;
    } else {
        // overflow: drop and reset until newline arrives
        g_bad_cnt++;
        s_len = 0;
    }
}

void ShumeiPi_GetCounts(ShumeiPi_Counts_t *out)
{
    if (!out) return;

    // atomic-ish snapshot: read twice and compare (simple method)
    uint16_t p1, h1, w1;
    uint32_t t1, b1;

    p1 = g_person; h1 = g_half; w1 = g_work; t1 = g_last_tick; b1 = g_bad_cnt;

    out->person = p1;
    out->half   = h1;
    out->work   = w1;
    out->last_update_tick = t1;
    out->bad_line_cnt = b1;
}
