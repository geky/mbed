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

#ifndef FILE_H
#define FILE_H

#include "filesystem/FileSystem.h"
#include "platform/ReadStream.h"
#include "platform/WriteStream.h"

namespace mbed {
/** \addtogroup drivers */
/** @{*/


/** File class
 */
class File : public ReadStream, public WriteStream {
public:
    /** Create an uninitialized file
     *
     *  Must call open to initialize the file on a file system
     */
    File();

    /** Create a file on a filesystem 
     *
     *  Creates and opens a file on a filesystem
     *
     *  @param fs       Filesystem as target for the file
     *  @param path     The name of the file to open
     *  @param mode     The mode to open the file in, one of FS_RDONLY, FS_WRONLY, FS_RDWR,
     *                  bitwise or'd with one of FS_CREAT, FS_TRUNC, FS_APPEND
     */
    File(FileSystem *fs, const char *path, fs_mode_t mode=FS_MODE_READ);

    /** Destroy a file
     *
     *  Closes file if the file is still open
     */
    virtual ~File();

    /** Open a file on the filesystem
     *
     *  @param fs       Filesystem as target for the file
     *  @param path     The name of the file to open
     *  @param mode     The mode to open the file in, one of FS_RDONLY, FS_WRONLY, FS_RDWR,
     *                  bitwise or'd with one of FS_CREAT, FS_TRUNC, FS_APPEND
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t open(FileSystem *fs, const char *path, fs_mode_t mode=FS_MODE_READ);

    /** Close a file
     *
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t close();

    /** Read the contents of a file into a buffer
     *
     *  @param buffer   The buffer to read in to
     *  @param size     The number of bytes to read
     *  @return         The number of bytes read, 0 at end of file, negative error on failure
     */

    virtual fs_size_or_error_t read(void *buffer, fs_size_t len);

    /** Write the contents of a buffer to a file
     *
     *  @param buffer   The buffer to write from
     *  @param size     The number of bytes to write 
     *  @return         The number of bytes written, negative error on failure
     */
    virtual fs_size_or_error_t write(const void *buffer, fs_size_t len);

    /** Flush any buffers associated with the file
     *
     *  @return         0 on success, negative error code on failure
     */
    virtual fs_error_t sync();

    /** Check if the file in an interactive terminal device
     *
     *  @return         True if the file is a terminal
     */
    virtual bool isatty();

    /** Move the file position to a given offset from from a given location
     *
     *  @param offset   The offset from whence to move to
     *  @param whence   The start of where to seek
     *      FS_SEEK_SET to start from beginning of file,
     *      FS_SEEK_CUR to start from current position in file,
     *      FS_SEEK_END to start from end of file
     *  @return         The new offset of the file
     */
    virtual fs_off_t seek(fs_off_t offset, fs_whence_t whence=FS_SEEK_SET);

    /** Get the file position of the file
     *
     *  @return         The current offset in the file
     */
    virtual fs_off_t tell();

    /** Rewind the file position to the beginning of the file
     *
     *  @note This is equivalent to file_seek(file, 0, FS_SEEK_SET)
     */
    virtual void rewind();

    /** Get the size of the file
     *
     *  @return         Size of the file in bytes
     */
    virtual fs_size_t size();

private:
    FileSystem *_fs;
    fs_file_t _file;
};


/** @}*/
} // namespace mbed

#endif
