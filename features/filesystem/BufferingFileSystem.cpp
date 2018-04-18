/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
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
#include "BufferingFileSystem.h"
#include <algorithm>


////// Generic filesystem operations //////

// Filesystem implementation (See BufferingFileSystem.h)
BufferingFileSystem::BufferingFileSystem(const char *name, mbed::FileSystem *fs)
        : FileSystem(name)
        , _fs(fs) {
}

BufferingFileSystem::~BufferingFileSystem() {
    // nop if unmounted
    unmount();
}

int BufferingFileSystem::mount(BlockDevice *bd)
{
    return _fs->mount(bd);
}

int BufferingFileSystem::unmount()
{
    return _fs->unmount();
}

int BufferingFileSystem::reformat(BlockDevice *bd)
{
    return _fs->reformat(bd);
}

int BufferingFileSystem::remove(const char *filename)
{
    return _fs->remove(filename);
}

int BufferingFileSystem::rename(const char *oldname, const char *newname)
{
    return _fs->rename(oldname, newname);
}

int BufferingFileSystem::mkdir(const char *name, mode_t mode)
{
    return _fs->mkdir(name, mode);
}

int BufferingFileSystem::stat(const char *name, struct stat *st)
{
    return _fs->stat(name, st);
}

int BufferingFileSystem::statvfs(const char *name, struct statvfs *st)
{
    return _fs->statvfs(name, st);
}

////// File operations //////
enum buffering_file_flags {
    MBED_BUFFERINGFS_DIRTY = 0x40000000
};

struct buffering_file {
    uint8_t *data;
    off_t off;
    off_t len;
    off_t cap;

    char *path;
    int flags;
};

int BufferingFileSystem::file_open(fs_file_t *file, const char *path, int flags)
{
    // allocate and check for errors since memory errors are
    // actually quite likely here
    struct buffering_file *f = (struct buffering_file*)malloc(
            sizeof(struct buffering_file));
    if (!f) {
        return -ENOMEM;
    }

    // buffer name
    f->path = (char*)malloc(strlen(path) + 1);
    if (!f->path) {
        free(f);
        return -ENOMEM;
    }
    strcpy(f->path, path);

    // load up file if it exists and we're not truncating
    f->cap = MBED_BUFFERINGFS_DEFAULT_SIZE;
    if (!(flags & O_TRUNC)) {
        fs_file_t rawfile;
        int err = _fs->file_open(&rawfile, path, O_RDONLY);
        if (err && (err != -ENOENT || !(flags & O_CREAT))) {
            free(f->path);
            free(f);
            return err;
        }

        if (err != -ENOENT) {
            off_t len = _fs->file_size(rawfile);
            if (len < 0) {
                free(f->path);
                free(f);
                return len;
            }

            f->off = 0;
            f->len = len;
            while (f->cap < len) {
                f->cap *= 2;
            }

            f->data = (uint8_t*)malloc(f->cap);
            if (!f->data) {
                free(f->path);
                free(f);
                return -ENOMEM;
            }

            off_t res = _fs->file_read(rawfile, f->data, len);
            if (res < 0) {
                free(f->data);
                free(f->path);
                free(f);
                return res;
            }

            err = _fs->file_close(rawfile);
            if (err) {
                free(f->data);
                free(f->path);
                free(f);
                return err;
            }

            f->flags = flags;
            *file = f;
            return 0;
        }
    }

    // setup default buffer otherwise
    f->off = 0;
    f->len = 0;
    f->cap = MBED_BUFFERINGFS_DEFAULT_SIZE;
    f->data = (uint8_t*)malloc(f->cap);
    if (!f->data) {
        free(f->path);
        free(f);
        return -ENOMEM;
    }

    f->flags = flags;
    *file = f;
    return 0;
}

int BufferingFileSystem::file_close(fs_file_t file)
{
    struct buffering_file *f = (struct buffering_file*)file;

    // no op if not dirty
    // otherwise writes out file
    int err = file_sync(file);

    // always free
    free(f->data);
    free(f->path);
    free(f);
    return err;
}

ssize_t BufferingFileSystem::file_read(fs_file_t file, void *buffer, size_t len)
{
    struct buffering_file *f = (struct buffering_file*)file;

    if ((f->flags & 0x3) == O_WRONLY) {
        return -EBADF;
    }

    off_t diff = std::min<off_t>(f->len - f->off, len);
    memcpy(buffer, f->data + f->off, diff);
    f->off += diff;
    return diff;
}

ssize_t BufferingFileSystem::file_write(fs_file_t file, const void *buffer, size_t len)
{
    struct buffering_file *f = (struct buffering_file*)file;

    if ((f->flags & 0x3) == O_RDONLY) {
        return -EBADF;
    }

    if (f->flags & O_APPEND) {
        f->off = f->len;
    }

    off_t end = f->off+len;
    if (end < f->off) {
        // overflowed off_t!
        return -ERANGE;
    }

    // Realloc if necessary
    off_t ncap = f->cap;
    while (end > ncap) {
        ncap *= 2;
    }

    if (ncap != f->cap) {
        void *ndata = realloc(f->data, ncap);
        if (!ndata) {
            return -ENOMEM;
        }

        f->data = (uint8_t*)ndata;
        f->cap = ncap;
    }

    // zero until new offset if needed
    if (f->off > f->len) {
        memset(f->data + f->len, 0, f->off - f->len);
    }

    // copy over the data and mark dirty
    memcpy(f->data + f->off, buffer, len);
    f->off += len;
    f->len = std::max(f->off, f->len);
    f->flags |= MBED_BUFFERINGFS_DIRTY;
    return len;
}

int BufferingFileSystem::file_sync(fs_file_t file)
{
    struct buffering_file *f = (struct buffering_file*)file;
    if (f->flags & MBED_BUFFERINGFS_DIRTY) {
        // write out to underlying filesystem
        fs_file_t rawfile;
        int err = _fs->file_open(&rawfile, f->path,
                (f->flags & ~0x3) | O_WRONLY | O_TRUNC);
        if (err) {
            return err;
        }

        off_t res = _fs->file_write(rawfile, f->data, f->len);
        if (res < 0) {
            return err;
        }

        err = _fs->file_close(rawfile);
        if (err) {
            return err;
        }

        f->flags &= ~MBED_BUFFERINGFS_DIRTY;
    }

    return 0;
}

off_t BufferingFileSystem::file_seek(fs_file_t file, off_t offset, int whence)
{
    struct buffering_file *f = (struct buffering_file*)file;

    if (whence == SEEK_SET) {
        f->off = offset;
    } else if (whence == SEEK_CUR) {
        f->off = f->off + offset;
    } else if (whence == SEEK_END) {
        f->off = f->len + offset;
    }

    return f->off;
}

off_t BufferingFileSystem::file_tell(fs_file_t file)
{
    struct buffering_file *f = (struct buffering_file*)file;
    return f->off;
}

off_t BufferingFileSystem::file_size(fs_file_t file)
{
    struct buffering_file *f = (struct buffering_file*)file;
    return f->len;
}


////// Dir operations //////
int BufferingFileSystem::dir_open(fs_dir_t *dir, const char *path)
{
    return _fs->dir_open(dir, path);
}

int BufferingFileSystem::dir_close(fs_dir_t dir)
{
    return _fs->dir_close(dir);
}

ssize_t BufferingFileSystem::dir_read(fs_dir_t dir, struct dirent *ent)
{
    return _fs->dir_read(dir, ent);
}

void BufferingFileSystem::dir_seek(fs_dir_t dir, off_t offset)
{
    return _fs->dir_seek(dir, offset);
}

off_t BufferingFileSystem::dir_tell(fs_dir_t dir)
{
    return _fs->dir_tell(dir);
}

void BufferingFileSystem::dir_rewind(fs_dir_t dir)
{
    return _fs->dir_rewind(dir);
}

