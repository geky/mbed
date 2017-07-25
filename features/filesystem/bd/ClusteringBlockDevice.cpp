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

#include "ClusteringBlockDevice.h"


ClusteringBlockDevice::ClusteringBlockDevice(BlockDevice *bd, bd_size_t erase_size)
    : _bd(bd)
    , _read_size(0)
    , _program_size(0)
    , _erase_size(erase_size)
{
}

ClusteringBlockDevice::ClusteringBlockDevice(BlockDevice *bd,
        bd_size_t read_size,
        bd_size_t program_size,
        bd_size_t erase_size)
    : _bd(bd)
    , _read_size(read_size)
    , _program_size(program_size)
    , _erase_size(erase_size)
{
}

int ClusteringBlockDevice::init()
{
    int err = _bd->init();
    if (err) {
        return err;
    }

    if (!_read_size) {
        _read_size = _bd->get_read_size();
    }

    if (!_program_size) {
        _program_size = _bd->get_program_size();
    }

    if (!_erase_size) {
        _erase_size = _bd->get_erase_size();
    }

    // Check that clusters are valid
    MBED_ASSERT(_read_size > _bd->get_read_size() &&
                _read_size % _bd->get_read_size() == 0);
    MBED_ASSERT(_program_size > _bd->get_program_size() &&
                _program_size % _bd->get_program_size() == 0);
    MBED_ASSERT(_erase_size > _bd->get_erase_size() &&
                _erase_size % _bd->get_erase_size() == 0);

    return 0;
}

int ClusteringBlockDevice::deinit()
{
    return _bd->deinit();
}

int ClusteringBlockDevice::read(void *b, bd_addr_t addr, bd_size_t size)
{
    MBED_ASSERT(is_valid_read(addr, size));
    return _bd->read(b, addr, size);
}

int ClusteringBlockDevice::program(const void *b, bd_addr_t addr, bd_size_t size)
{
    MBED_ASSERT(is_valid_program(addr, size));
    return _bd->program(b, addr, size);
}

int ClusteringBlockDevice::erase(bd_addr_t addr, bd_size_t size)
{
    MBED_ASSERT(is_valid_erase(addr, size));
    return _bd->erase(addr, size);
}

bd_size_t ClusteringBlockDevice::get_read_size() const
{
    return _read_size;
}

bd_size_t ClusteringBlockDevice::get_program_size() const
{
    return _program_size;
}

bd_size_t ClusteringBlockDevice::get_erase_size() const
{
    return _erase_size;
}

bd_size_t ClusteringBlockDevice::size() const
{
    // Ignore wasted space due to unaligned clusters
    return (_bd->size() / _erase_size) * _erase_size;
}
