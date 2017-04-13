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
#ifndef MBED_NAMEHANDLE_H
#define MBED_NAMEHANDLE_H

#include "platform/FileHandle.h"
#include "platform/DirHandle.h"
#include "platform/FileSystemHandle.h"

namespace mbed {
/** \addtogroup drivers */
/** @{*/


// predeclared tree class
class NameTree;

typedef enum mbed_handle {
    MBED_UNKNOWNHANDLE     = 0,
    MBED_FILEHANDLE        = 1,
    MBED_DIRHANDLE         = 2,
    MBED_FILESYSTEMHANDLE  = 3,
} mbed_handle_t;

class NameHandle {
public:
    NameHandle(FileHandle *handle, const char *name = NULL, NameTree *tree = NULL);
    NameHandle(FileSystemHandle *handle, const char *name = NULL, NameTree *tree = NULL);

    NameHandle(void *handle, mbed_handle_t type,
            const char *name = NULL, NameTree *tree = NULL);
    virtual ~NameHandle();

    const char *get_handlename() const
    {
        return _name;
    }

    mbed_handle_t get_handletype() const
    {
        return _type;
    }

    NameTree *get_handletree() const
    {
        return _tree;
    }

    FileHandle *get_filehandle() const
    {
        MBED_ASSERT(get_handletype() == MBED_FILEHANDLE);
        return static_cast<FileHandle*>(_handle);
    }

    FileSystemHandle *get_filesystemhandle() const
    {
        MBED_ASSERT(get_handletype() == MBED_FILESYSTEMHANDLE);
        return static_cast<FileSystemHandle*>(_handle);
    }

private:
    /* disallow copy constructor and assignment operators */
    NameHandle(const NameHandle &);
    NameHandle &operator=(const NameHandle &);

private:
    void *_handle;
    mbed_handle_t _type;
    const char *_name;

    friend class NameTree;
    NameTree *_tree;
    NameHandle *_next;
};


} // namespace mbed

#endif

/** @}*/
