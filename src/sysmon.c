#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "hal_ui.h"

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
    int interval_ms = getenv("INTERVAL_MS") ? atoi(getenv("INTERVAL_MS")) : 500;

    for(;;) {
        int cpu = cpu_usage_pct();

        printf("\"cpu\": %d}\n",
             cpu);

        usleep(interval_ms * 1000);
    }

    return 0;
}