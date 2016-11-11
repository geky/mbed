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

#ifndef MBED_DEVICE_BLOCK_DEVICE_H
#define MBED_DEVICE_BLOCK_DEVICE_H

#include "block_api.h"
#include "BlockDevice.h"
#include "Mutex.h"

#ifdef DEVICE_BLOCKDEVICE


/** A hardware device capable of writing and reading blocks provided by a device
 */
class DeviceBlockDevice : public BlockDevice {
public:
    /** Lifetime of a block device
     */
    DeviceBlockDevice();
    DeviceBlockDevice(block_device_t *bd);
    virtual ~DeviceBlockDevice();

    /** Initialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual bd_error_t init();

    /** Deinitialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual bd_error_t deinit();

    /** Read blocks from a block device
     *
     *  @param buffer   Buffer to write block to
     *  @param block    Index of block to begin reading from
     *  @param count    Number of blocks to read (defaults to 1)
     *  @return         Either the number of blocks read or a negative error code on failure
     */
    virtual bd_count_or_error_t read(void *buffer, bd_block_t block, bd_count_t count = 1);

    /** Write blocks to a block device
     *
     *  @param buffer   Buffer of data to write to blocks
     *  @param block    Index of block to begin writing to
     *  @param count    Number of blocks to write (defaults to 1)
     *  @return         Either the number of blocks written or a negative error code on failure
     */
    virtual bd_count_or_error_t write(const void *buffer, bd_block_t block, bd_count_t count = 1);

    /** Sync an underlying block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual bd_error_t sync();

    /** Get the status of the underlying block device
     *
     *  @return         Status of the underlying device
     */
    virtual bd_status_t get_status();

    /** Get the block size of the underlying device
     *
     *  @return         The number of 8-bit bytes in a block
     */
    virtual bd_size_t get_block_size();

    /** Get the block count of the underlying device
     *
     *  @return         The number of blocks on the device
     */
    virtual bd_count_t get_block_count();

private:
    block_device_t *_bd;
    rtos::Mutex _lock;
};


#endif

#endif
