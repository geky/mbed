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

#ifndef MBED_FORMAT_H
#define MBED_FORMAT_H

#include <cstdarg>
#include <Callback.h>
#include <mbed.h>

namespace mbed {
/** \addtogroup platform */
/** @{*/


/** Write formatted output to a callback
 *
 *  The cbprintf functions are an extension to the standard printf family of functions that
 *  uses a callback to describe its output stream. The callback is expected to behave like
 *  the standard C write function and should consume a provided buffer of data. Any negative
 *  error codes from the write callback are propagated as the return value of cbprintf.
 *
 *  @param write    Write callback that behaves like the standard C write function
 *  @param fmt      Format string to printf
 *  @param ...      Arguments to printf
 *  @return         Number of bytes written, negative value on failure
 *  @see write
 *  @see printf
 */
ssize_t cbprintf(mbed::Callback<ssize_t(const void *, size_t)> write, const char *fmt, ...);

/** Write formatted output to a callback
 *
 *  The cbprintf functions are an extension to the standard printf family of functions that
 *  uses a callback to describe its output stream. The callback is expected to behave like
 *  the standard C write function and should consume a provided buffer of data. Any negative
 *  error codes from the write callback are propagated as the return value of cbprintf.
 *
 *  @param write    Write callback that behaves like the standard C write function
 *  @param fmt      Format string to printf
 *  @param args     Arguments to printf
 *  @return         Number of bytes written, negative value on failure
 *  @see write
 *  @see printf
 */
ssize_t vcbprintf(mbed::Callback<ssize_t(const void *, size_t)> write, const char *fmt, va_list args);

/** Read formatted input from a callback
 *
 *  The cbscanf functions are an extension to the standard scanf family of functions that
 *  uses a callback to describe its input stream. The callback is expected to behave like
 *  the standard C read function and should populate a provided buffer with available
 *  data. Any negative error codes from the read callback are propagated as the return
 *  value of cbscanf.
 *
 *  @param read     Read callback that behaves like the standard C read function
 *  @param fmt      Format string to scanf
 *  @param ...      Arguments to scanf
 *  @return         Number of patterns matched, negative value on failure
 *  @see read
 *  @see scanf
 */
ssize_t cbscanf(mbed::Callback<ssize_t(void *, size_t)> read, const char *fmt, ...);

/** Read formatted input from a callback
 *
 *  The cbscanf functions are an extension to the standard scanf family of functions that
 *  uses a callback to describe its input stream. The callback is expected to behave like
 *  the standard C read function and should populate a provided buffer with available
 *  data. Any negative error codes from the read callback are propagated as the return
 *  value of cbscanf.
 *
 *  @param read     Read callback that behaves like the standard C read function
 *  @param fmt      Format string to scanf
 *  @param args     Arguments to scanf
 *  @return         Number of patterns matched, negative value on failure
 *  @see read
 *  @see scanf
 */
ssize_t vcbscanf(mbed::Callback<ssize_t(void *, size_t)> read, const char *fmt, va_list args);


/** @}*/
}

#endif
