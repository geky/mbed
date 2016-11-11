/* mbed Microcontroller Library
 * Copyright (c) 2016 ARM Limited
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

#ifndef MBED_BLOCK_API_H
#define MBED_BLOCK_API_H

#include <stdint.h>


/** Opaque type for block device
 */
typedef void *block_device_t;

/** Enum of standard error codes
 *
 *  @enum bd_error_t
 */
typedef enum bd_error {
    BD_ERROR_OK             = 0,     /*!< no error */
    BD_ERROR_WOULD_BLOCK    = -4001, /*!< operation would block */
    BD_ERROR_UNSUPPORTED    = -4002, /*!< unsupported operation */
    BD_ERROR_PARAMETER      = -4003, /*!< invalid parameter */
    BD_ERROR_NO_DEVICE      = -4004, /*!< device is missing or not connected */
    BD_ERROR_DEVICE_ERROR   = -4005, /*!< device specific error */
} bd_error_t;

/** Enum of status codes
 */
typedef enum bd_status {
    BD_STATUS_OK            = 0,    /*!< device is ready for read/writes */
    BD_STATUS_UNINITIALIZED = 4001, /*!< device is currently uninitialized */
} bd_status_t;

/** Type representing the address of a specific block
 */
typedef uint32_t bd_block_t;

/** Type representing a quantity of 8-bit bytes
 */
typedef uint32_t bd_size_t;

/** Type representing a count of blocks
 */
typedef uint32_t bd_count_t;

/** Type representing either a count of blocks or a negative error code
 */
typedef int32_t bd_count_or_error_t;


/** Get the default block device
 *
 *  @return         Default block device
 */
block_device_t *bd_get_block_device(void);

/** Initialize a block device
 *
 *  @param bd       Block device to initialize
 *  @return         0 on success or a negative error code on failure
 */
bd_error_t bd_init(block_device_t *bd);

/** Deinitialize a block device
 *
 *  @param bd       Block device to deinitialize
 *  @return         0 on success or a negative error code on failure
 */
bd_error_t bd_deinit(block_device_t *bd);

/** Read blocks from a block device
 *
 *  @param bd       Block device to read from
 *  @param buffer   Buffer to write block to
 *  @param block    Index of block to begin reading from
 *  @param count    Number of blocks to read
 *  @return         Either the number of blocks read or a negative error code on failure
 */
bd_count_or_error_t bd_read(block_device_t *bd,
        void *buffer, bd_block_t block, bd_count_t count);

/** Write blocks to a block device
 *
 *  @param bd       Block device to write to
 *  @param buffer   Buffer of data to write to blocks
 *  @param block    Index of block to begin writing to
 *  @param count    Number of blocks to write
 *  @return         Either the number of blocks written or a negative error code on failure
 */
bd_count_or_error_t bd_write(block_device_t *bd,
        const void *buffer, bd_block_t block, bd_count_t count);

/** Sync an underlying block device
 *
 *  @param bd       Block device to flush any pending writes
 *  @return         0 on success or a negative error code on failure
 */
bd_error_t bd_sync(block_device_t *bd);

/** Get the status of the underlying block device
 *
 *  @param bd       Block device to query status of
 *  @return         Status of the underlying device
 */
bd_status_t bd_get_status(block_device_t *bd);

/** Get the block size of the underlying device
 *
 *  @param bd       Block device to query the block size of
 *  @return         The number of 8-bit bytes in a block
 */
bd_size_t bd_get_block_size(block_device_t *bd);

/** Get the block count of the underlying device
 *
 *  @param bd       Block device to query to block count of
 *  @return         The number of blocks on the device
 */
bd_count_t bd_get_block_count(block_device_t *bd);


#endif
