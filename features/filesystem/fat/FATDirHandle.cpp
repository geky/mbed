/* mbed Microcontroller Library
 * Copyright (c) 2006-2012 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <string.h>
#include "ff.h"
#include "FATDirHandle.h"
#include "FATMisc.h"
#include "mbed.h"


FATDirHandle::FATDirHandle(const FATFS_DIR &the_dir, PlatformMutex * mutex): _mutex(mutex) {
    dir = the_dir;
}

int FATDirHandle::closedir() {
    lock();
    FRESULT retval = f_closedir(&dir);
    fat_filesystem_set_errno(retval);
    unlock();
    delete this;
    return retval;
}

struct dirent *FATDirHandle::readdir() {
    FILINFO finfo;

    lock();
#if _USE_LFN
    finfo.lfname = cur_entry.d_name;
    finfo.lfsize = sizeof(cur_entry.d_name);
#endif // _USE_LFN

    FRESULT res = f_readdir(&dir, &finfo);
    fat_filesystem_set_errno(res);
    cur_entry.d_type = (finfo.fattrib & AM_DIR) ? DT_DIR : DT_REG;

#if _USE_LFN
    if(res != 0 || finfo.fname[0]==0) {
        unlock();
        return NULL;
    } else {
        if(cur_entry.d_name[0]==0) {
            // No long filename so use short filename.
            memcpy(cur_entry.d_name, finfo.fname, sizeof(finfo.fname));
        }
        unlock();
        return &cur_entry;
    }
#else
    if(res != 0 || finfo.fname[0]==0) {
        unlock();
        return NULL;
    } else {
        memcpy(cur_entry.d_name, finfo.fname, sizeof(finfo.fname));
        unlock();
        return &cur_entry;
    }
#endif /* _USE_LFN */
}

void FATDirHandle::rewinddir() {
    lock();
    dir.index = 0;
    fat_filesystem_set_errno(FR_OK);
    unlock();
}

off_t FATDirHandle::telldir() {
    lock();
    off_t offset = dir.index;
    fat_filesystem_set_errno(FR_OK);
    unlock();
    return offset;
}

void FATDirHandle::seekdir(off_t location) {
    lock();
    dir.index = location;
    fat_filesystem_set_errno(FR_OK);
    unlock();
}

void FATDirHandle::lock() {
    _mutex->lock();
}

void FATDirHandle::unlock() {
    _mutex->unlock();
}

