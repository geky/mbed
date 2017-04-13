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
#ifndef MBED_NAMETREE_H
#define MBED_NAMETREE_H

#include "platform/platform.h"

#include "platform/FileSystemHandle.h"
#include "platform/FileHandle.h"
#include "platform/DirHandle.h"
#include "platform/NameHandle.h"

namespace mbed {
/** \addtogroup drivers */
/** @{*/


/** A filesystem-like object is one that can be used to open file-like
 *  objects though it by fopen("/name/filename", mode)
 *
 *  Implementations must define at least open (the default definitions
 *  of the rest of the functions just return error values).
 *
 * @Note Synchronization level: Set by subclass
 */
class NameTree : public FileSystemHandle, public NameHandle {
public:
    /** NameTree lifetime
     */
    NameTree(const char *name = NULL)
            : NameHandle(this, name), _head(NULL) {}
    virtual ~NameTree() {}

    /** Insert a new name-handle into the tree
     *
     *  @param handle   NameHandle to insert
     */
    virtual void insert(NameHandle *handle);

    /** Remove a name-handle from the tree
     *
     *  @param handle   NameHandle to remove
     */
    virtual void remove(NameHandle *handle);

    /** Find the best matching name-handle
     *
     *  @param name     The name of the path to find. The pointer itself
     *                  will be modified to reflect the remaining path that
     *                  needs to be resolved on the found handle.
     *  @param handle   A pointer where to store the best matching name-handle.
     *  @return         0 on success, negative error code on failure
     */
    virtual int find(NameHandle **handle, const char **name) const;

    /** Open a file on the filesystem
     *
     *  @param file     Destination for the handle to a newly created file
     *  @param name     The name of the file to open
     *  @param flags    The flags to open the file in, one of O_RDONLY, O_WRONLY, O_RDWR,
     *                  bitwise or'd with one of O_CREAT, O_TRUNC, O_APPEND
     *  @return         0 on success, negative error code on failure
     */
    virtual int open(FileHandle **file, const char *name, int flags);

    /** Remove a file from the filesystem.
     *
     *  @param name     The name of the file to remove.
     *  @return         0 on success, negative error code on failure
     */
    virtual int remove(const char *name);

    /** Rename a file in the filesystem.
     *
     *  @param name     The name of the file to rename.
     *  @param newname  The name to rename it to
     *  @return         0 on success, negative error code on failure
     */
    virtual int rename(const char *name, const char *newname);

    /** Store information about the file in a stat structure
     *
     *  @param name     The name of the file to find information about
     *  @param st       The stat buffer to write to
     *  @return         0 on success, negative error code on failure
     */
    virtual int stat(const char *name, struct stat *st);

    /** Create a directory in the filesystem.
     *
     *  @param name     The name of the directory to create.
     *  @param mode     The permissions with which to create the directory
     *  @return         0 on success, negative error code on failure
     */
    virtual int mkdir(const char *name, mode_t mode);

private:
    PlatformMutex *_mutex;
    NameHandle *_head;
};


} // namespace mbed

#endif

/** @}*/
