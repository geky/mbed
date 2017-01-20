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

#include "SlicingBlockDevice.h"


SlicingBlockDevice::SlicingBlockDevice(BlockDevice *bd, bd_addr_t start)
    : _bd(bd)
    , _start_from_end(false), _start(start)
    , _stop_from_end(true), _stop(0)
{
    if ((int64_t)_start < 0) {
        _start_from_end = true;
        _start = -_start;
    }
}

SlicingBlockDevice::SlicingBlockDevice(BlockDevice *bd, bd_addr_t start, bd_addr_t stop)
    : _bd(bd)
    , _start_from_end(false), _start(start)
    , _stop_from_end(false), _stop(stop)
{
    if ((int64_t)_start < 0) {
        _start_from_end = true;
        _start = -_start;
    }

    if ((int64_t)_stop < 0) {
        _stop_from_end = true;
        _stop = -_stop;
    }
}

bd_error_t SlicingBlockDevice::init()
{
    bd_error_t err = _bd->init();
    if (err) {
        return err;
    }

    bd_size_t size = _bd->size();

    // Calculate from_end values
    if (_start_from_end) {
        _start_from_end = false;
        _start = size - _start;
    }

    if (_stop_from_end) {
        _stop_from_end = false;
        _stop = size - _stop;
    }

    // Check that block addresses are valid
    if (!is_valid_erase(_start, _stop - _start)) {
        return BD_ERROR_PARAMETER;
    }

    return 0;
}

bd_error_t SlicingBlockDevice::deinit()
{
    return _bd->deinit();
}

bd_error_t SlicingBlockDevice::read(void *b, bd_addr_t addr, bd_size_t size)
{
    if (!is_valid_read(addr, size)) {
        return BD_ERROR_PARAMETER;
    }

    return _bd->read(b, addr - _start, size);
}

bd_error_t SlicingBlockDevice::program(const void *b, bd_addr_t addr, bd_size_t size)
{
    if (!is_valid_program(addr, size)) {
        return BD_ERROR_PARAMETER;
    }

    return _bd->program(b, addr - _start, size);
}

bd_error_t SlicingBlockDevice::erase(bd_addr_t addr, bd_size_t size)
{
    if (!is_valid_erase(addr, size)) {
        return BD_ERROR_PARAMETER;
    }

    return _bd->erase(addr - _start, size);
}

bd_error_t SlicingBlockDevice::sync()
{
    return _bd->sync();
}

bd_size_t SlicingBlockDevice::read_size()
{
    return _bd->read_size();
}

bd_size_t SlicingBlockDevice::program_size()
{
    return _bd->program_size();
}

bd_size_t SlicingBlockDevice::erase_size()
{
    return _bd->erase_size();
}
