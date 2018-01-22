#ifndef PERF_H
#define PERF_H

// variables under test
// (avoids mbed_lib recompile)
#define PERF_BLOCK_SIZE 1024

// perf stuff
enum perf_level {
    PERF_FS,
    PERF_AUTH,
    PERF_ENC,
    PERF_SOTP_GET,
    PERF_SOTP_SET,
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
