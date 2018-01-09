/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
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
#ifndef MBED_ENCRYPTED_BLOCK_DEVICE_H
#define MBED_ENCRYPTED_BLOCK_DEVICE_H

#include "BlockDevice.h"
#include <stddef.h>

enum {
    BD_ERROR_ENC_BD_NO_MEM                   = -3501,
    BD_ERROR_ENC_BD_INSUFFICIENT_SOTP_TYPES  = -3502,
    BD_ERROR_ENC_BD_SOTP_ERROR               = -3503,
    BD_ERROR_ENC_BD_AUTHENTICATION_FAILED    = -3504,
    BD_ERROR_ENC_BD_CALC_CMAC_ERROR          = -3505,
};

/** Encrypted block device
 *
 * Encrypts the underlying BD data with AES-CTR encryption.
 *
 */
class EncryptedBlockDevice : public BlockDevice
{
public:
    /** Lifetime of the block device
     *
     * @param bd            Block device to back the EncryptedBlockDevice
     * @param erase_cycles  Number of erase cycles before failure
     */
    EncryptedBlockDevice(BlockDevice *bd);
    virtual ~EncryptedBlockDevice();

    /** Initialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual int init();

    /** Deinitialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual int deinit();

    /** Read blocks from a block device
     *
     *  @param buffer   Buffer to read blocks into
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes, must be a multiple of read block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int read(void *buffer, bd_addr_t addr, bd_size_t size);

    /** Program blocks to a block device
     *
     *  The blocks must have been erased prior to being programmed
     *
     *  @param buffer   Buffer of data to write to blocks
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes, must be a multiple of program block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int program(const void *buffer, bd_addr_t addr, bd_size_t size);

    /** Erase blocks on a block device
     *
     *  The state of an erased block is undefined until it has been programmed
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes, must be a multiple of erase block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int erase(bd_addr_t addr, bd_size_t size);

    /** Get the size of a readable block
     *
     *  @return         Size of a readable block in bytes
     */
    virtual bd_size_t get_read_size() const;

    /** Get the size of a programmable block
     *
     *  @return         Size of a programmable block in bytes
     */
    virtual bd_size_t get_program_size() const;

    /** Get the size of a erasable block
     *
     *  @return         Size of a erasable block in bytes
     */
    virtual bd_size_t get_erase_size() const;

    /** Get the total size of the underlying device
     *
     *  @return         Size of the underlying device in bytes
     */
    virtual bd_size_t size() const;

    /** Get the number of reads (cleared on read)
     *
     *  @return         number of reads
     */
    virtual bd_size_t get_num_reads();

    /** Get the number of writes (cleared on read)
     *
     *  @return         number of writes
     */
    virtual bd_size_t get_num_writes();

    /** Get the number of erases (cleared on read)
     *
     *  @return         number of erases
     */
    virtual bd_size_t get_num_erases();

private:
    static int _num_instances;
    BlockDevice *_bd;
    uint8_t _sotp_start_type;
    bd_size_t _erase_size;
    int _init_done;
    // statistics
    bd_size_t _num_reads, _prev_num_reads;
    bd_size_t _num_writes, _prev_num_writes;
    bd_size_t _num_erases, _prev_num_erases;

    // All functions dealing with encryption and authentication should be standalone in order
    // to allow them to be part of the trusted environment in the future.
    int sign_erase_unit(uint16_t erase_unit_num, uint16_t num_erases);
    int authenticate_chunk(void *auth_ctx, const uint8_t *buf, bd_size_t chunk_size, bd_size_t &total_size,
                           uint8_t* given_cmac, int &start);
    int encrypt_chunk(uint8_t *src_buf, uint8_t *&dst_buf, bd_addr_t addr,
                      bd_size_t chunk_size, uint16_t iv);
    int decrypt_chunk(uint8_t *src_buf, uint8_t *&dst_buf, bd_addr_t &addr,
                      bd_addr_t aligned_addr, bd_size_t chunk_size, bd_size_t &total_size,
                      uint16_t iv);
};


#endif
