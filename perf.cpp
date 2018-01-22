#include "perf.h"
#include "mbed.h"

const char *perf_names[PERF_COUNT] = {
    [PERF_FS]       = "fs",
    [PERF_AUTH]     = "auth",
    [PERF_ENC]      = "enc",
    [PERF_SOTP_GET] = "sotp-get",
    [PERF_SOTP_SET] = "sotp-set",
    [PERF_BD_READ]  = "bd-read",
    [PERF_BD_PROG]  = "bd-prog",
    [PERF_BD_ERASE] = "bd-erase",
};

uint32_t perf_starts[PERF_COUNT]; // start times
uint32_t perf_refs[PERF_COUNT];   // reference count to allow repeats
uint32_t perf_totals[PERF_COUNT]; // total time

void perf_reset() {
    for (int i = 0; i < PERF_COUNT; i++) {
        perf_refs[i] = 0;
        perf_totals[i] = 0;
    }
}

void perf_enter(enum perf_level level) {
    if (perf_refs[level] == 0) {
        perf_starts[level] = equeue_tick();
    }

    perf_refs[level] += 1;
}

void perf_exit(enum perf_level level) {
    perf_refs[level] -= 1;

    if (perf_refs[level] == 0) {
        perf_totals[level] += equeue_tick() - perf_starts[level];
    }
}

void perf_print() {
    printf("--- perf results ---\n");
    for (int i = 0; i < PERF_COUNT; i++) {
        printf("%s: %dms\n", perf_names[i], perf_totals[i]);
    }
}

