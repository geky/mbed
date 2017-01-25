/* File 
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

#include "File.h"
#include "mbed.h"


File::File()
    : _fs(0), _file(0)
{
}

File::File(FileSystem *fs, const char *path, fs_mode_t mode)
    : _fs(0), _file(0)
{
    open(fs, path, mode);
}

File::~File()
{
    if (_fs) {
        close();
    }
}

fs_error_t File::open(FileSystem *fs, const char *path, fs_mode_t mode)
{
    if (_fs) {
        return FS_ERROR_PARAMETER;
    }

    _fs = fs;
    return _fs->file_open(&_file, path, mode);
}

fs_error_t File::close()
{
    if (!_fs) {
        return FS_ERROR_PARAMETER;
    }

    fs_error_t err = _fs->file_close(_file);
    _fs = 0;
    return err;
}

fs_size_or_error_t File::read(void *buffer, fs_size_t len)
{
    MBED_ASSERT(_fs);
    return _fs->file_read(_file, buffer, len);
}

fs_size_or_error_t File::write(const void *buffer, fs_size_t len)
{
    MBED_ASSERT(_fs);
    return _fs->file_write(_file, buffer, len);
}

fs_error_t File::sync()
{
    MBED_ASSERT(_fs);
    return _fs->file_sync(_file);
}

bool File::isatty()
{
    MBED_ASSERT(_fs);
    return _fs->file_isatty(_file);
}

fs_off_t File::seek(fs_off_t offset, fs_whence_t whence)
{
    MBED_ASSERT(_fs);
    return _fs->file_seek(_file, offset, whence);
}

fs_off_t File::tell()
{
    MBED_ASSERT(_fs);
    return _fs->file_tell(_file);
}

void File::rewind()
{
    MBED_ASSERT(_fs);
    return _fs->file_rewind(_file);
}

fs_size_t File::size()
{
    MBED_ASSERT(_fs);
    return _fs->file_size(_file);
}

