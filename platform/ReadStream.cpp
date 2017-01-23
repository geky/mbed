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

#include "platform/ReadStream.h"
#include "mbed.h"
#include "mbed_format.h"


ssize_t ReadStream::readall(void *b, size_t size)
{
    uint8_t *buffer = static_cast<uint8_t*>(b);
    size_t readed = 0;

    while (size > 0) {
        ssize_t r = read(buffer, size);

        if (r > 0) {
            buffer += r;
            size -= r;
            readed += r;
        } else if (r < 0) {
            return r;
        } else {
            return readed;
        }
    }

    return readed;
}

int ReadStream::getc() {
    uint8_t c;
    ssize_t r = readall(&c, 1);
    if (r <= 0) {
        return r;
    } else {
        return c;
    }
}

ssize_t ReadStream::scanf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    ssize_t r = vscanf(format, args);
    va_end(args);
    return r;
}

ssize_t ReadStream::vscanf(const char *format, va_list args) {
    return vcbscanf(callback(this, &ReadStream::read), format, args);
}

ssize_t ReadStream::cat(WriteStream *stream, size_t buffer) {
    return stream->cat(this, buffer);
}
