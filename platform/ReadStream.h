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
#ifndef MBED_READ_STREAM_H
#define MBED_READ_STREAM_H

#include "platform/platform.h"
#include <cstdarg>

namespace mbed {
/** \addtogroup platform */
/** @{*/

class WriteStream;


/** ReadStream - Interface for reading from streams of bytes
 */
class ReadStream {
public:
    /** ReadStream lifetime
     */
    virtual ~ReadStream() {}

    /** Read data from the underlying stream
     *
     *  @param buffer   Destination buffer for read data
     *  @param size     Size of buffer in bytes
     *  @return         Number of read bytes, negative value on failure
     */
    virtual ssize_t read(void *buffer, size_t size) = 0;

    /** Read data from the underlying stream until size has been read
     *
     *  @param buffer   Destination buffer for read data
     *  @param size     Size of buffer in bytes
     *  @return         Number of read bytes, negative value on failure
     */
    ssize_t readall(void *buffer, size_t size);

    /** Get a character from the underlying stream
     *
     *  This is equivalent to read with size of 1
     *
     *  @return         8-bit character read, negative value on failure
     */
    int getc();

    /** Read formatted input from the underlying stream
     *
     *  @param format   Format string to scanf
     *  @param ...      Arguments to scanf
     *  @return         Number of patterns matched, negative value on failure
     *  @see scanf
     */
    ssize_t scanf(const char *format, ...);

    /** Read formatted input from the underlying stream
     *
     *  @param format   Format string to scanf
     *  @param args     Arguments to scanf
     *  @return         Number of patterns matched, negative value on failure
     *  @see scanf
     */
    ssize_t vscanf(const char *format, va_list args);

    /** Write to a write stream from the underlying read stream
     *
     *  Continues to write to the write stream until the underlying
     *  read stream becomes empty
     *
     *  @param stream   Write stream to write to from underlying stream
     *  @param buffer   Size of buffer to use in bytes
     *  @return         Number of written bytes, negative value on failure
     */
    ssize_t cat(WriteStream *stream, size_t buffer=32);
};


/** @}*/
} // namespace mbed

#endif
