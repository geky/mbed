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
#ifndef MBED_WRITE_STREAM_H
#define MBED_WRITE_STREAM_H

#include "platform/platform.h"
#include <cstdarg>

namespace mbed {
/** \addtogroup platform */
/** @{*/

class ReadStream;


/** WriteStream - Interface for writing to streams of bytes
 */
class WriteStream {
public:
    /** WriteStream lifetime
     */
    virtual ~WriteStream() {}

    /** Write data to the underlying stream
     *
     *  @param buffer   Buffer of data to write
     *  @param size     Size of buffer in bytes
     *  @return         Number of written bytes, negative value on failure
     */
    virtual ssize_t write(const void *buffer, size_t size) = 0;

    /** Write data to the underlying stream until all data has been written
     *
     *  @param buffer   Buffer of data to write
     *  @param size     Size of buffer in bytes
     *  @return         Number of written bytes, negative value on failure
     */
    ssize_t writeall(const void *buffer, size_t size);

    /** Write a character from the underlying stream
     *
     *  This is equivalent to write with size of 1
     *
     *  @param c        8-bit character to write
     *  @return         The written character on success, negative value on failure
     */
    int putc(int c);

    /** Write formatted output to the underlying stream
     *
     *  @param format   Format string to printf
     *  @param ...      Arguments to printf
     *  @return         Number of bytes written, negative value on failure
     *  @see printf
     */
    ssize_t printf(const char *format, ...);

    /** Write formatted output to the underlying stream
     *
     *  @param format   Format string to printf
     *  @param args     Arguments to printf
     *  @return         Number of bytes written, negative value on failure
     *  @see printf
     */
    ssize_t vprintf(const char *format, va_list args);

    /** Write a read stream to the underlying write stream
     *
     *  Continues to write to the underlying write stream until the
     *  read stream becomes empty
     *
     *  @param stream   Read stream to write to underlying stream
     *  @param buffer   Size of buffer to use in bytes
     *  @return         Number of written bytes, negative value on failure
     */
    ssize_t cat(ReadStream *stream, size_t buffer=32);
};


/** @}*/
} // namespace mbed

#endif
