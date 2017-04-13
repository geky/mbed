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
#include "NameTree.h"
#include "mbed.h"


// name-tree operations
void NameTree::insert(NameHandle *handle)
{
    _mutex->lock();

    // Make sure handle is not in list twice
    remove(handle);

    // Insert at head of list
    handle->_next = _head;
    _head = handle;

    _mutex->unlock();
}

void NameTree::remove(NameHandle *target)
{
    _mutex->lock();

    // Remove from list
    NameHandle **phandle = &_head;
    NameHandle *handle = _head;
    while (handle) {
        if (handle == target) {
            *phandle = handle->_next;
            handle->_next = NULL;
            break;
        }

        phandle = &handle->_next;
        handle = handle->_next;
    }

    _mutex->unlock();
}

static const char *nextname(const char *path, size_t *namelen)
{
    size_t pathlen = 0;

    while (true) {
        // skip leading slashes
        path += strspn(path, "/");
        pathlen = strcspn(path, "/");

        // skip '.' and root '..'
        if ((pathlen == 1 && memcmp(path, ".", 1) == 0) ||
            (pathlen == 2 && memcmp(path, "..", 2) == 0)) {
            path += pathlen;
            *namelen = pathlen;
            return path;
        }

        // skip if matched by '..'
        const char *suffix = path + pathlen;
        size_t sufflen;
        int depth = 1;

        while (true) {
            suffix += strspn(suffix, "/");
            sufflen = strcspn(suffix, "/");
            if (sufflen == 0) {
                *namelen = pathlen;
                return path;
            }

            if (sufflen == 2 && memcmp(suffix, "..", 2) == 0) {
                depth -= 1;
                if (depth == 0) {
                    path = suffix + sufflen;
                    break;
                }
            } else {
                depth += 1;
            }

            suffix += sufflen;
        }

        path += pathlen;
    }
}

static bool namecheck(const char *base, const char **name)
{
    const char *basename = base;
    const char *checkname = *name;
    size_t baselen;
    size_t checklen;

    while (true) {
        basename = nextname(basename, &baselen);
        checkname = nextname(checkname, &checklen);

        // found our match
        if (baselen == 0) {
            *name = (checkname[0] == '\0') ? "." : checkname;
            return true;
        }

        // otherwise check next part of path
        if (!(baselen == checklen &&
              memcmp(basename, checkname, checklen) == 0)) {
            return false;
        }

        basename += baselen;
        checkname += checklen;
    }
}

int NameTree::find(NameHandle **namehandle, const char **name) const
{
    _mutex->lock();

    // Check in order, recent insertions get precedence
    NameHandle *handle = _head;
    while (handle) {
        const char *reducedname = *name;
        if (namecheck(handle->get_handlename(), &reducedname)) {
            _mutex->unlock();

            // Only dirs and filesystems can allow a non-resolved name
            if ((strstr(reducedname, "/") != NULL &&
                 handle->get_handletype() != MBED_DIRHANDLE &&
                 handle->get_handletype() != MBED_FILESYSTEMHANDLE)) {
                return -ENOTDIR;
            }

            *namehandle = handle;
            *name = reducedname;
            return 0;
        }

        handle = handle->_next;
    }

    _mutex->unlock();
    return -ENOENT;
}

// file-system operations
int NameTree::open(FileHandle **file, const char *name, int flags)
{
    NameHandle *handle;
    int err = find(&handle, &name);
    if (err) {
        if (err == -ENOENT && (flags & O_CREAT)) {
            // does not support file creation
            return -EINVAL;
        } else {
            return err;
        }
    }

    switch (handle->get_handletype()) {
        case MBED_FILEHANDLE: {
            // TODO handle open mode?
            *file = handle->get_filehandle();
            return 0;
        }

        case MBED_FILESYSTEMHANDLE: {
            FileSystemHandle *fs = handle->get_filesystemhandle();
            return fs->open(file, name, flags);
        }

        default: {
            return -EINVAL;
        }
    }
}

int NameTree::stat(const char *name, struct stat *st)
{
    NameHandle *handle;
    int err = find(&handle, &name);
    if (err) {
        return err;
    }

    // TODO!!! handle dirs?

    switch (handle->get_handletype()) {
        case MBED_FILEHANDLE: {
            FileHandle *file = handle->get_filehandle();
            st->st_size = file->size();
            st->st_mode = S_IFREG;
            st->st_mode |= S_IRWXU | S_IRWXG | S_IRWXO;
            return 0;
        }

        case MBED_FILESYSTEMHANDLE: {
            FileSystemHandle *fs = handle->get_filesystemhandle();
            return fs->stat(name, st);
        }

        default: {
            return -EINVAL;
        }
    }
}

int NameTree::remove(const char *name)
{
    NameHandle *handle;
    int err = find(&handle, &name);
    if (err) {
        return err;
    }

    switch (handle->get_handletype()) {
        case MBED_FILESYSTEMHANDLE: {
            FileSystemHandle *fs = handle->get_filesystemhandle();
            return fs->remove(name);
        }

        default: {
            return -EINVAL;
        }
    }
}

int NameTree::rename(const char *name, const char *newname)
{
    NameHandle *handle;
    int err = find(&handle, &name);
    if (err) {
        return err;
    }

    NameHandle *newhandle;
    err = find(&newhandle, &newname);
    if (err) {
        return err;
    }

    if (handle != newhandle) {
        // can not move across filesystem boundaries
        return -EINVAL;
    }

    switch (handle->get_handletype()) {
        case MBED_FILESYSTEMHANDLE: {
            FileSystemHandle *fs = handle->get_filesystemhandle();
            return fs->rename(name, newname);
        }

        default: {
            return -EINVAL;
        }
    }
}

int NameTree::mkdir(const char *name, mode_t mode)
{
    // mkdir is invalid
    return -EINVAL;
}

// TODO dir iteration
