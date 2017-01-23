/*
 * Copyright (c) 2015-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed_format.h"
#include "mbed.h"

namespace mbed {

// callback based printf using FileHandle hooks
struct PrintFileHandle : public FileHandle {
    Callback<ssize_t(const void *, size_t)> _write;
    ssize_t err;
    char name[sizeof(":01234567")];

    PrintFileHandle(Callback<ssize_t(const void *, size_t)> write)
        : _write(write) {
    }

    virtual ssize_t write(const void *buffer, size_t length) {
        ssize_t r = _write(buffer, length);
        if (r < 0) {
            err = r;
        }
        return r;
    }

    virtual int close() {
        return 0;
    }

    virtual int isatty() {
        return 0;
    }

    virtual int fsync() {
        return 0;
    }

    virtual ssize_t read(void *buffer, size_t length) {
        MBED_UNREACHABLE;
    }

    virtual off_t lseek(off_t, int) {
        MBED_UNREACHABLE;
    }
};

ssize_t vcbprintf(mbed::Callback<ssize_t(const void *, size_t)> write, const char *fmt, va_list args)
{
    PrintFileHandle handle(write);

    char name[sizeof(":01234567")];
    sprintf(name, ":%p", &handle);
    FILE *f = fopen(name, "w");
    mbed_set_unbuffered_stream(f);

    ssize_t r = vfprintf(f, fmt, args);

    fflush(f);
    fclose(f);

    return (r >= 0) ? r : handle.err;
}

ssize_t cbprintf(mbed::Callback<ssize_t(const void *, size_t)> write, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ssize_t r = vcbprintf(write, fmt, args);
    va_end(args);
    return r;
}


// callback based scanf using FileHandle hooks
struct ScanFileHandle : public FileHandle {
    Callback<ssize_t(void *, size_t)> _read;
    ssize_t err;
    char name[sizeof(":01234567")];

    ScanFileHandle(Callback<ssize_t(void *, size_t)> read)
        : _read(read) {
        sprintf(name, ":%p", this);
    }

    virtual ssize_t read(void *buffer, size_t length) {
        ssize_t r = _read(buffer, length);
        if (r < 0) {
            err = r;
        }
        return r;
    }

    virtual int close() {
        return 0;
    }

    virtual int isatty() {
        return 0;
    }

    virtual int fsync() {
        return 0;
    }

    virtual ssize_t write(const void *buffer, size_t length) {
        MBED_ASSERT(false);
        MBED_UNREACHABLE;
    }

    virtual off_t lseek(off_t, int) {
        MBED_ASSERT(false);
        MBED_UNREACHABLE;
    }
};

ssize_t vcbscanf(mbed::Callback<ssize_t(void *, size_t)> read, const char *fmt, va_list args)
{
    ScanFileHandle handle(read);

    char name[sizeof(":01234567")];
    sprintf(name, ":%p", &handle);
    FILE *f = fopen(name, "r");
    mbed_set_unbuffered_stream(f);

    ssize_t r = vfscanf(f, fmt, args);

    fflush(f);
    fclose(f);

    return (r >= 0) ? r : handle.err;
}

ssize_t cbscanf(mbed::Callback<ssize_t(void *, size_t)> read, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ssize_t r = vcbscanf(read, fmt, args);
    va_end(args);
    return r;
}


}
