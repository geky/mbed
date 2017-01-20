/* Dir 
 * Copyright (c) 2015 ARM Limited
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

#include "Dir.h"
#include "mbed.h"


Dir::Dir()
    : _fs(0), _dir(0)
{
}

Dir::Dir(FileSystem *fs, const char *path)
    : _fs(0), _dir(0)
{
    open(fs, path);
}

Dir::~Dir()
{
    if (_fs) {
        close();
    }
}

fs_error_t Dir::open(FileSystem *fs, const char *path)
{
    if (_fs) {
        return FS_ERROR_PARAMETER;
    }

    _fs = fs;
    return _fs->dir_open(&_dir, path);
}

fs_error_t Dir::close()
{
    if (!_fs) {
        return FS_ERROR_PARAMETER;
    }

    fs_error_t err = _fs->dir_close(_dir);
    _fs = 0;
    return err;
}

fs_size_or_error_t Dir::read(char *path, size_t len)
{
    MBED_ASSERT(_fs);
    return _fs->dir_read(_dir, path, len);
}

void Dir::seek(fs_off_t offset)
{
    MBED_ASSERT(_fs);
    return _fs->dir_seek(_dir, offset);
}

fs_off_t Dir::tell()
{
    MBED_ASSERT(_fs);
    return _fs->dir_tell(_dir);
}

void Dir::rewind()
{
    MBED_ASSERT(_fs);
    return _fs->dir_rewind(_dir);
}

fs_size_t Dir::size()
{
    MBED_ASSERT(_fs);
    return _fs->dir_size(_dir);
}
