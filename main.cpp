/* Copyright (c) 2018 Arm Limited
*
* SPDX-License-Identifier: Apache-2.0
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
* See the License for the specific language governing permissions an
* limitations under the License.
*/

#include "mbed.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "BlockDevice.h"
#include "SPIFReducedBlockDevice.h"
#include "TDBStore.h"
#include "LittleStore.h"
#include "lfs.h"
#include "lfs_util.h"

SPIFReducedBlockDevice rspif(
    MBED_CONF_SPIF_DRIVER_SPI_MOSI,
    MBED_CONF_SPIF_DRIVER_SPI_MISO,
    MBED_CONF_SPIF_DRIVER_SPI_CLK,
    MBED_CONF_SPIF_DRIVER_SPI_CS,
    MBED_CONF_SPIF_DRIVER_SPI_FREQ
);
BlockDevice *bd = &rspif;
//BlockDevice *bd = BlockDevice::get_default_instance();
#ifdef USETDB
TDBStore store(bd);
char buffer[64];
int err;
#elif defined(USELS)
LittleStore store(bd);
char buffer[64];
int err;
#else
lfs_t lfs;
lfs_file_t file;
struct lfs_config cfg;
char buffer[64];
int err;
extern "C" int bdread(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    return bd->read(buffer, block*c->block_size + off, size);
}

extern "C" int bdprog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    return bd->program(buffer, block*c->block_size + off, size);
}

extern "C" int bderase(const struct lfs_config *c, lfs_block_t block) {
    return bd->erase(block*c->block_size, c->block_size);
}

extern "C" int bdsync(const struct lfs_config *c) {
    return bd->sync();
}
#endif

InterruptIn irq(BUTTON1);
void reset() {
#if defined(USETDB) || defined(USELS)
    err = store.init();
    printf("init() -> %d\n", err);
    err = store.reset();
    printf("reset() -> %d\n", err);
    err = store.deinit();
    printf("deinit() -> %d\n", err);
#else
    err = lfs_format(&lfs, &cfg);
    printf("format() -> %d\n", err);
#endif
}

int main()
{
    printf("-- KVStore hack --\n");
    irq.fall(mbed_event_queue()->event(reset));

#if defined(USETDB) || defined(USELS)
    err = store.init();
    printf("init() -> %d\n", err);

    err = store.get("test", buffer, 64);
    printf("get(\"%s\", %p, %d) -> %d\n", "test", buffer, 64, err);
    printf("[%.64s]\n", buffer);

    strcpy(buffer, "hello there!");
    err = store.set("test", buffer, strlen(buffer), 0);
    printf("set(\"%s\", %p, %d, %d) -> %d\n", "test", buffer, strlen(buffer)+1, 0, err);
    printf("[%.64s]\n", buffer);
    err = store.get("test", buffer, 64);
    printf("get(\"%s\", %p, %d) -> %d\n", "test", buffer, 64, err);
    printf("[%.64s]\n", buffer);

    err = store.deinit();
    printf("deinit() -> %d\n", err);
#else
    bd->init();
    cfg.read = bdread;
    cfg.prog = bdprog;
    cfg.erase = bderase;
    cfg.sync = bdsync;
    cfg.read_size = bd->get_read_size();
    cfg.prog_size = bd->get_program_size();
    cfg.block_size = bd->get_erase_size();
    cfg.block_count = bd->size() / bd->get_erase_size();
    cfg.lookahead_size = 64;
    cfg.cache_size = lfs_max(64, cfg.prog_size);

    err = lfs_mount(&lfs, &cfg);
    printf("init() -> %d\n", err);
    while (err) {}

//    err  = lfs_file_open(&lfs, &file, "test", LFS_O_RDONLY);
//    err |= lfs_file_read(&lfs, &file, buffer, 64);
//    err |= lfs_file_close(&lfs, &file);
    err = lfs_get(&lfs, "test", buffer, 64);
    printf("get(\"%s\", %p, %d) -> %d\n", "test", buffer, 64, err);
    printf("[%.64s]\n", buffer);

    strcpy(buffer, "hello there!");
//    err  = lfs_file_open(&lfs, &file, "test", LFS_O_CREAT | LFS_O_TRUNC | LFS_O_WRONLY);
//    err |= lfs_file_write(&lfs, &file, buffer, strlen(buffer)+1);
//    err |= lfs_file_close(&lfs, &file);
    err = lfs_set(&lfs, "test", buffer, 64);
    printf("set(\"%s\", %p, %d, %d) -> %d\n", "test", buffer, strlen(buffer)+1, 0, err);
    printf("[%.64s]\n", buffer);
//    err  = lfs_file_open(&lfs, &file, "test", LFS_O_RDONLY);
//    err |= lfs_file_read(&lfs, &file, buffer, 64);
//    err |= lfs_file_close(&lfs, &file);
    err = lfs_get(&lfs, "test", buffer, 64);
    printf("get(\"%s\", %p, %d) -> %d\n", "test", buffer, 64, err);
    printf("[%.64s]\n", buffer);

    err = lfs_unmount(&lfs);
    printf("deinit() -> %d\n", err);
#endif
}
