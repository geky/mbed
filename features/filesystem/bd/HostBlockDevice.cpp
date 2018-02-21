/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "HostBlockDevice.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#if !defined(__FreeBSD__)
#include <stropts.h>
#include <linux/fs.h>
#elif defined(__FreeBSD__)
#define BLKSSZGET DIOCGSECTORSIZE
#define BLKGETSIZE DIOCGMEDIASIZE
#include <sys/disk.h>
#endif


HostBlockDevice::HostBlockDevice(const char *path)
    : _path(path), _rc(0) {
}

int HostBlockDevice::init() {
    _rc += 1;
    if (_rc != 1) {
        return 0;
    }

    _fd = open(_path, O_RDWR);
    if (_fd < 0) {
        return -errno;
    }

    // get sector size
    long ssize;
    int err = ioctl(_fd, BLKSSZGET, &ssize);
    if (err) {
        return -errno;
    }
    _block_size = ssize;

    // get size in sectors
    long size;
    err = ioctl(_fd, BLKGETSIZE, &size);
    if (err) {
        return -errno;
    }
    _block_count = size;

    return 0;
}

int HostBlockDevice::deinit() {
    if (_rc == 0) {
        return 0;
    }
    _rc -= 1;

    return close(_fd);
}

int HostBlockDevice::read(void *buffer, bd_addr_t addr, bd_size_t size) {
    assert(is_valid_read(addr, size));

    // go to block
    int err = lseek(_fd, (off_t)addr, SEEK_SET);
    if (err < 0) {
        return -errno;
    }

    // read block
    ssize_t res = ::read(_fd, buffer, (size_t)size);
    if (res < 0) {
        return -errno;
    }

    return 0;
}

int HostBlockDevice::program(const void *buffer, bd_addr_t addr, bd_size_t size) {
    assert(is_valid_program(addr, size));

    // go to block
    int err = lseek(_fd, (off_t)addr, SEEK_SET);
    if (err < 0) {
        return -errno;
    }

    // write block
    ssize_t res = write(_fd, buffer, (size_t)size);
    if (res < 0) {
        return -errno;
    }

    return 0;
}

int HostBlockDevice::sync() {
    int err = fsync(_fd);
    if (err) {
        return -errno;
    }

    return 0;
}

bd_size_t HostBlockDevice::get_read_size() const {
    return _block_size;
}

bd_size_t HostBlockDevice::get_program_size() const {
    return _block_size;
}

bd_size_t HostBlockDevice::size() const {
    return (bd_size_t)_block_size * (bd_size_t)_block_count;
}
