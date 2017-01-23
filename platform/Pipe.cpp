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

#include "platform/Pipe.h"
#include "mbed.h"


Pipe::Pipe(size_t buffer)
    : _size(buffer), _r(0), _w(0)
{
    _buffer = new uint8_t[buffer];
}

Pipe::~Pipe()
{
    delete[] _buffer;
}

ssize_t Pipe::read(void *b, size_t size)
{
    uint8_t *buffer = static_cast<uint8_t*>(b);
    size_t readed = 0;

    _mutex.lock();
    while (_r >= _w) {
        _mutex.unlock();
        _rsem.wait();
        _mutex.lock();
    }

    while (_r < _w && readed < size) {
        *buffer++ = _buffer[_r++ % _size];
        readed++;
    }
    _mutex.unlock();

    _wsem.release();
    return readed;
}

ssize_t Pipe::write(const void *b, size_t size)
{
    const uint8_t *buffer = static_cast<const uint8_t*>(b);
    size_t written = 0;

    _mutex.lock();
    while (_w >= _r + _size) {
        _mutex.unlock();
        _wsem.wait();
        _mutex.lock();
    }

    while (_w < _r + _size && written < size) {
        _buffer[_w++ % _size] = *buffer++;
        written++;
    }
    _mutex.unlock();

    _rsem.release();
    return written;
}
