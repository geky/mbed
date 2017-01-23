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
#ifndef MBED_PIPE_H
#define MBED_PIPE_H

#include "platform/platform.h"
#include "platform/ReadStream.h"
#include "platform/WriteStream.h"
#include <cstdarg>

#include "rtos/Mutex.h"
#include "rtos/Semaphore.h"

namespace mbed {
/** \addtogroup platform */
/** @{*/



/** Pipe - A stream that connects writing to reading
 */
class Pipe : ReadStream, WriteStream{
public:
    /** Pipe lifetime
     *
     *  @param buffer   Size of internal buffer in bytes
     */
    Pipe(size_t buffer=512);
    virtual ~Pipe();

    /** Read data from the underlying stream
     *
     *  @param buffer   Destination buffer for read data
     *  @param size     Size of buffer in bytes
     *  @return         Number of read bytes, negative value on failure
     */
    virtual ssize_t read(void *buffer, size_t size);

    /** Write data to the underlying stream
     *
     *  @param buffer   Buffer of data to write
     *  @param size     Size of buffer in bytes
     *  @return         Number of written bytes, negative value on failure
     */
    virtual ssize_t write(const void *buffer, size_t size) = 0;

private:
    uint8_t *_buffer;
    size_t _size;
    unsigned _r;
    unsigned _w;
    rtos::Mutex _mutex;
    rtos::Semaphore _rsem;
    rtos::Semaphore _wsem;
};


/** @}*/
} // namespace mbed

#endif
