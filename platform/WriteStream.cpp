/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "platform/WriteStream.h"
#include "mbed.h"
#include "mbed_format.h"


ssize_t WriteStream::writeall(const void *b, size_t size)
{
    const uint8_t *buffer = static_cast<const uint8_t*>(b);
    size_t written = 0;

    while (size > 0) {
        ssize_t r = write(buffer, size);

        if (r > 0) {
            buffer += r;
            size -= r;
            written += r;
        } else if (r < 0) {
            return r;
        } else {
            return written;
        }
    }

    return written;
}

int WriteStream::putc(int w) {
    uint8_t c = w;
    ssize_t r = writeall(&c, 1);
    if (r <= 0) {
        return r;
    } else {
        return c;
    }
}

ssize_t WriteStream::printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    ssize_t r = vprintf(format, args);
    va_end(args);
    return r;
}

ssize_t WriteStream::vprintf(const char *format, va_list args) {
    return vcbprintf(callback(this, (ssize_t (WriteStream::*)(const void *, size_t))&WriteStream::write), format, args);
}

ssize_t WriteStream::cat(ReadStream *stream, size_t size) {
    uint8_t *buffer = new uint8_t[size];
    while (true) {
        ssize_t r = stream->read(buffer, size);
        if (r <= 0) {
            return r;
        }

        r = this->writeall(buffer, r);
        if (r <= 0) {
            return r;
        }
    }
}
