#ifndef PERF_H
#define PERF_H

enum perf_level {
    PERF_FS,
    PERF_ENC,
    PERF_BD_READ,
    PERF_BD_PROG,
    PERF_BD_ERASE,
    PERF_COUNT, // for size
};

void perf_enter(enum perf_level level);
void perf_exit(enum perf_level level);

void perf_reset();
void perf_print();

#endif
