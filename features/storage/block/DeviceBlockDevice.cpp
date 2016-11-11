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

#include "DeviceBlockDevice.h"

#if DEVICE_BLOCKDEVICE


DeviceBlockDevice::DeviceBlockDevice()
{
    _bd = bd_get_block_device();
}

DeviceBlockDevice::DeviceBlockDevice(block_device_t *bd)
{
    _bd = bd;
}

DeviceBlockDevice::~DeviceBlockDevice()
{
    if (get_status() != BD_STATUS_UNINITIALIZED) {
        deinit();
    }
}

bd_error_t DeviceBlockDevice::init()
{
    _lock.lock();
    bd_error_t err = bd_init(_bd);
    _lock.unlock();
    return err;
}

bd_error_t DeviceBlockDevice::deinit()
{
    _lock.lock();
    bd_error_t err = bd_deinit(_bd);
    _lock.unlock();
    return err;
}

bd_count_or_error_t DeviceBlockDevice::read(void *buffer, bd_block_t block, bd_count_t count)
{
    _lock.lock();
    bd_count_or_error_t ret = bd_read(_bd, buffer, block, count);
    _lock.unlock();
    return ret;
}

bd_count_or_error_t DeviceBlockDevice::write(const void *buffer, bd_block_t block, bd_count_t count)
{
    _lock.lock();
    bd_count_or_error_t ret = bd_write(_bd, buffer, block, count);
    _lock.unlock();
    return ret;
}

bd_error_t DeviceBlockDevice::sync()
{
    _lock.lock();
    bd_error_t err = bd_sync(_bd);
    _lock.unlock();
    return err;
}

bd_status_t DeviceBlockDevice::get_status()
{
    _lock.lock();
    bd_status_t status = bd_get_status(_bd);
    _lock.unlock();
    return status;
}

bd_size_t DeviceBlockDevice::get_block_size()
{
    _lock.lock();
    bd_size_t size = bd_get_status(_bd);
    _lock.unlock();
    return size;
}

bd_count_t DeviceBlockDevice::get_block_count()
{
    _lock.lock();
    bd_count_t count = bd_get_block_count(_bd);
    _lock.unlock();
    return count;
}


#endif
