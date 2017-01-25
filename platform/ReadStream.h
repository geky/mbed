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
#include "platform/Pollable.h"
#include <cstdarg>

namespace mbed {
/** \addtogroup platform */
/** @{*/


/** ReadStream - Interface for reading from streams of bytes
 */
class ReadStream : public virtual Pollable {
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

    /** Check if data is available to be read
     *
     *  @return         Lower bound on the number of bytes that could be read
     *                  or a negative value on failure. May return a lower value
     *                  or 1 if number of bytes is unknown.
     */
    virtual ssize_t readable() = 0;

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
};


/** @}*/
} // namespace mbed

#endif
