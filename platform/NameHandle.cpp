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
#include "platform/NameHandle.h"
#include "mbed.h"

NameHandle::NameHandle(void *handle, mbed_handle_t type,
        const char *name, NameTree *tree)
        : _handle(handle), _type(type), _name(name), _tree(tree)
{
    _tree->insert(this);
}

NameHandle::NameHandle(FileHandle *handle, const char *name, NameTree *tree)
        : _handle(handle), _type(MBED_FILEHANDLE), _name(name), _tree(tree)
{
    _tree->insert(this);
}

NameHandle::NameHandle(FileSystemHandle *handle, const char *name, NameTree *tree)
        : _handle(handle), _type(MBED_FILESYSTEMHANDLE), _name(name), _tree(tree)
{
    _tree->insert(this);
}

NameHandle::~NameHandle()
{
    _tree->remove(this);
}
