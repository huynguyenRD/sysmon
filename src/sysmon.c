#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "hal_ui.h"

static long long rdll(const char* p) {
    FILE *fp = fopen(p, "r");
    if (!fp) {
        perror("Failed to open file");
        return -1;
    }

    long long v = -1;
    if (fscanf(fp, "%lld", &v) != 1) {
        perror("Failed to read long long value");
        v = -1;
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return v;
}

static int read_thermal_mC(void) {
    return (int)rdll("/sys/class/thermal/thermal_zone0/temp");
}

static int read_mem_kB(long *total, long *available) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Failed to open /proc/meminfo");
        return -1;
    }

    char k[32];
    long v;
    char u[8];
    long _mt = 0, _ma = 0;

    while (fscanf(fp, "%31s %ld %7s", k, &v, u) == 3) {
        if (strcmp(k, "MemTotal:") == 0) {
            _mt = v;
        } else if (strcmp(k, "MemAvailable:") == 0) {
            _ma = v;
        }
    }
    fclose(fp);

    if (total) {
        *total = _mt;
    }
    if (available) {
        *available = _ma;
    }

    return 0;
}

static int cpu_usage_pct(void) {
    static unsigned long long pu, pn, ps, pi;
    unsigned long long u, n, s, i;

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Failed to open /proc/stat");
        return -1;
    }
    
    if (fscanf(fp, "cpu %llu %llu %llu %llu", &u, &n, &s, &i) != 4) {
        perror("Failed to read CPU stats");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    
    unsigned long long du = u - pu;
    unsigned long long dn = n - pn;
    unsigned long long ds = s - ps;
    unsigned long long di = i - pi;

    pu = u; pn = n; ps = s; pi = i;

    unsigned long long busy = du + dn + ds;
    unsigned long long tot = busy + di;

    if (tot == 0) {
        return 0; // Avoid division by zero
    }
    
    int usage_pct = (int)((busy * 100) / tot);
    return usage_pct;
}

int main(void) {
    printf("Sysmon application started\n");
    const char* iface = getenv("IFACE") ? getenv("IFACE") : "eth0";
    int interval_ms = getenv("INTERVAL_MS") ? atoi(getenv("INTERVAL_MS")) : 500;
    int lcd_on = getenv("HAL_LCD") ? atoi(getenv("HAL_LCD")) : 1;
    const char* card = getenv("HAL_CARD"); if(!card) card="/dev/dri/card0";

    cpu_usage_pct();
    if (lcd_on) {
        hal_init();
        hal_lcd_init();
        hal_ui_init(card);
    }
    usleep(100*10000); // Allow time for CPU stats to stabilize

    char p1[256];
    char p2[256];
    unsigned long long rx0 = 0;
    unsigned long long tx0 = 0;
    unsigned long long rx1 = 0;
    unsigned long long tx1 = 0;
    snprintf(p1, sizeof(p1), "/sys/class/net/%s/statistics/rx_bytes", iface);
    snprintf(p2, sizeof(p2), "/sys/class/net/%s/statistics/tx_bytes", iface);
    FILE* fr = fopen(p1, "r");
    FILE* ft = fopen(p2, "r");
    if (fr && ft) {
        fscanf(fr, "%llu", &rx0);
        fscanf(ft, "%llu", &tx0);
    }
    if (fr) fclose(fr);
    if (ft) fclose(ft);

    for(;;) {
        int cpu = cpu_usage_pct();
        long mt, ma; read_mem_kB(&mt, &ma); int mem_used = (mt > 0) ? (int)((mt - ma) * 100 / mt) : -1;
        int t_mC = read_thermal_mC(); int temp_c = (t_mC >= 0) ? t_mC / 1000 : -1;

        fr = fopen(p1, "r");
        ft = fopen(p2, "r");
        if (fr && ft) {
            fscanf(fr, "%llu", &rx1);
            fscanf(ft, "%llu", &tx1);
        }
        if (fr) fclose(fr);
        if (ft) fclose(ft);
        double rx_rate = (rx1 >= rx0) ? (double)(rx1 - rx0)/1024.0/(interval_ms/1000.0) : 0.0;
        double tx_rate = (tx1 >= tx0) ? (double)(tx1 - tx0)/1024.0/(interval_ms/1000.0) : 0.0;
        rx0 = rx1;
        tx0 = tx1;

        printf("{\"cpu\": %d, \"mem_used\": %d, \"temp\": %d, \"rx_kBps\": %.1f, \"tx_kBps\": %.1f}\n",
             cpu, mem_used, temp_c, tx_rate, rx_rate);
        fflush(stdout);

        if (lcd_on) {
            hal_ui_bar3(cpu, mem_used, temp_c);
        }
        usleep(interval_ms * 1000);
    }

    return 0;
}