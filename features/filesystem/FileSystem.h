
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
#ifndef MBED_FILESYSTEM_H
#define MBED_FILESYSTEM_H

#include "platform/platform.h"

#include "drivers/FileBase.h"
#include "drivers/FileHandle.h"
#include "drivers/DirHandle.h"

namespace mbed {
/** \addtogroup drivers */
/** @{*/

typedef int32_t fs_error_t;
enum fs_error {
    FS_ERROR_OK          = 0,
    FS_ERROR_UNSUPPORTED = -5001,
    FS_ERROR_PARAMETER   = -5002,
};

typedef int32_t fs_mode_t;
enum fs_mode {
    FS_MODE_READ,
    FS_MODE_WRITE,
    FS_MODE_READ_WRITE,

    FS_MODE_CREATE,
    FS_MODE_TRUNCATE,
    FS_MODE_APPEND,
};

typedef uint32_t fs_size_t;
typedef uint32_t fs_off_t;

typedef int64_t fs_size_or_error_t;

typedef enum fs_whence {
    FS_SEEK_SET,
    FS_SEEK_CUR,
    FS_SEEK_END,
} fs_whence_t;

typedef struct fs_stat {
} fs_stat_t;

#define FS_NAME_MAX NAME_MAX

typedef void *fs_file_t;
typedef void *fs_dir_t;




/** A filesystem-like object is one that can be used to open files
 *  though it by fopen("/name/filename", mode)
 *
 *  Implementations must define at least open (the default definitions
 *  of the rest of the functions just return error values).
 *
 * @Note Synchronization level: Set by subclass
 */
class FileSystem {
public:
    /** FileSystem lifetime
     */
    ~FileSystem() {}

    /** Remove a file from the filesystem.
     *
     *  @param path     The name of the file to remove.
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t remove(const char *path) = 0;

    /** Rename a file in the filesystem.
     *
     *  @param path     The name of the file to rename.
     *  @param newpath  The name to rename it to
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t rename(const char *path, const char *newpath) = 0;

    /** Store information about the file in a stat structure
     *
     *  @param path     The name of the file to find information about
     *  @param st       The stat buffer to write to
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t stat(const char *path, struct stat *st) = 0;

    /** Create a directory in the filesystem.
     *
     *  @param path     The name of the directory to create.
     *  @param mode     The permissions with which to create the directory
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t mkdir(const char *path, fs_mode_t mode);

protected:
    friend class File;
    friend class Dir;

    /** Open a file on the filesystem
     *
     *  @param file     Destination for the handle to a newly created file
     *  @param path     The name of the file to open
     *  @param mode     The mode to open the file in, one of FS_RDONLY, FS_WRONLY, FS_RDWR,
     *                  bitwise or'd with one of FS_CREAT, FS_TRUNC, FS_APPEND
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t file_open(fs_file_t *file, const char *path, fs_mode_t mode) = 0;

    /** Close a file
     *
     *  @param file     File handle
     *  return          0 on success, negative error code on failure
     */
    virtual fs_error_t file_close(fs_file_t file) = 0;

    /** Read the contents of a file into a buffer
     *
     *  @param file     File handle
     *  @param buffer   The buffer to read in to
     *  @param size     The number of bytes to read
     *  @return         The number of bytes read, 0 at end of file, negative error on failure
     */
    virtual fs_size_or_error_t file_read(fs_file_t file, void *buffer, fs_size_t len) = 0;

    /** Write the contents of a buffer to a file
     *
     *  @param file     File handle
     *  @param buffer   The buffer to write from
     *  @param size     The number of bytes to write 
     *  @return         The number of bytes written, negative error on failure
     */
    virtual fs_size_or_error_t file_write(fs_file_t file, const void *buffer, fs_size_t len) = 0;

    /** Flush any buffers associated with the file
     *
     *  @param file     File handle
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t file_sync(fs_file_t file);

    /** Check if the file in an interactive terminal device
     *
     *  @param file     File handle
     *  @return         True if the file is a terminal
     */
    virtual bool file_isatty(fs_file_t file);

    /** Move the file position to a given offset from from a given location
     *
     *  @param file     File handle
     *  @param offset   The offset from whence to move to
     *  @param whence   The start of where to seek
     *      FS_SEEK_SET to start from beginning of file,
     *      FS_SEEK_CUR to start from current position in file,
     *      FS_SEEK_END to start from end of file
     *  @return         The new offset of the file
     */
    virtual fs_off_t file_seek(fs_file_t file, fs_off_t offset, fs_whence_t whence) = 0;

    /** Get the file position of the file
     *
     *  @param file     File handle
     *  @return         The current offset in the file
     */
    virtual fs_off_t file_tell(fs_file_t file);

    /** Rewind the file position to the beginning of the file
     *
     *  @param file     File handle
     *  @note This is equivalent to file_seek(file, 0, FS_SEEK_SET)
     */
    virtual void file_rewind(fs_file_t file);

    /** Get the size of the file
     *
     *  @param file     File handle
     *  @return         Size of the file in bytes
     */
    virtual fs_size_t file_size(fs_file_t);

    /** Open a directory on the filesystem
     *
     *  @param dir      Destination for the handle to the directory
     *  @param path     Name of the directory to open
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t dir_open(fs_dir_t *dir, const char *path);

    /** Close a directory
     *
     *  @param dir      Dir handle
     *  return          0 on success, negative error code on failure
     */
    virtual fs_error_t dir_close(fs_dir_t dir);

    /** Read the next directory entry
     *
     *  @param dir      Dir handle
     *  @param path     The buffer to read the null terminated path name in to
     *  @param size     The maximum number of bytes in the buffer, this is at most FS_NAME_MAX
     *  @return         1 on reading a filename, 0 at end of directory, negative error on failure
     */
    virtual fs_size_or_error_t dir_read(fs_dir_t dir, char *path, size_t len);

    /** Set the current position of the directory
     *
     *  @param dir      Dir handle
     *  @param offset   Offset of the location to seek to,
     *                  must be a value returned from dir_tell
     */
    virtual void dir_seek(fs_dir_t dir, fs_off_t offset);

    /** Get the current position of the directory
     *
     *  @param dir      Dir handle
     *  @return         Position of the directory that can be passed to dir_rewind
     */
    virtual fs_off_t dir_tell(fs_dir_t dir);

    /** Rewind the current position to the beginning of the directory
     *
     *  @param dir      Dir handle
     */
    virtual void dir_rewind(fs_dir_t dir);

    /** Get the sizeof the directory 
     *
     *  @param dir      Dir handle
     *  @return         Number of files in the directory
     */
    virtual fs_size_t dir_size(fs_dir_t dir);
};


/** @}*/
} // namespace mbed

#endif
