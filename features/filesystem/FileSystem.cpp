
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

#include "mbed.h"
#include "filesystem/FileSystem.h"


fs_error_t FileSystem::file_sync(fs_file_t file)
{
    return 0;
}

bool FileSystem::file_isatty(fs_file_t file)
{
    return false;
}

fs_off_t FileSystem::file_tell(fs_file_t file)
{
    return file_seek(file, 0, FS_SEEK_CUR);
}

void FileSystem::file_rewind(fs_file_t file)
{
    file_seek(file, 0, FS_SEEK_SET);
}

fs_size_t FileSystem::file_size(fs_file_t file)
{
    fs_off_t off = file_tell(file);
    fs_size_t size = file_seek(file, 0, FS_SEEK_END);
    file_seek(file, off, FS_SEEK_SET);
    return size;
}

fs_error_t FileSystem::mkdir(const char *path, fs_mode_t mode)
{
    return FS_ERROR_UNSUPPORTED;
}

fs_error_t FileSystem::dir_open(fs_dir_t *dir, const char *path)
{
    return FS_ERROR_UNSUPPORTED;
}

fs_error_t FileSystem::dir_close(fs_dir_t dir)
{
    return FS_ERROR_UNSUPPORTED;
}

fs_size_or_error_t FileSystem::dir_read(fs_dir_t dir, char *path, size_t len)
{
    return FS_ERROR_UNSUPPORTED;
}

void FileSystem::dir_seek(fs_dir_t dir, fs_off_t offset)
{
}

fs_off_t FileSystem::dir_tell(fs_dir_t dir)
{
    return 0;
}

void FileSystem::dir_rewind(fs_dir_t dir)
{
    // Note, the may not satisfy rewind on all filesystems
    dir_seek(dir, 0);
}

fs_size_t FileSystem::dir_size(fs_dir_t dir)
{
    fs_off_t off = dir_tell(dir);
    fs_size_t size = 0;

    dir_rewind(dir);
    while (true) {
        fs_error_t res = dir_read(dir, NULL, 0);
        if (res <= 0) {
            break;
        }

        size += 1;
    }
    dir_seek(dir, off);

    return size;
}

