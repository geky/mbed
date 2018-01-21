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

#include "EncryptedBlockDevice.h"
#include "cmsis.h"
#include "mbed.h"
#include "aes.h"
#include "cmac.h"
#include "sotp.h"
#include "perf.h"

// TODO: Currently use a constant key until a key generator is implemented
const unsigned char *default_key = (const unsigned char *) "Fear is the key.";
#define DEFAULT_KEY_SIZE_BITS 128

const unsigned char *encrypt_key = default_key;
#define ENC_KEY_SIZE_BITS DEFAULT_KEY_SIZE_BITS
const unsigned char *auth_key = default_key;
#define AUTH_KEY_SIZE_BITS DEFAULT_KEY_SIZE_BITS

#define ENC_BLK_SIZE 16

#define NONCE_CTR_POS               12
#define NONCE_NUM_ERASES_POS        6
#define NONCE_ERASE_UNIT_NUM_POS    4

#ifndef ENC_BD_READ_BUF_SIZE
#define ENC_BD_READ_BUF_SIZE  512
#endif

#ifndef ENC_BD_WRITE_BUF_SIZE
#define ENC_BD_WRITE_BUF_SIZE 512
#endif

#undef MIN
#define MIN(a,b)       ((a) < (b) ? (a) : (b))

// Need to determine byte order for HTONL macro, following logics supports GCC and ARM compilers
#ifndef BYTE_ORDER
#ifdef __BIG_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#define BYTE_ORDER BIG_ENDIAN
#else
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

#undef HTONL
#if BYTE_ORDER == BIG_ENDIAN
#define HTONL(x) (x)
#else
#define HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                  (((x) & 0x0000ff00UL) <<  8) | \
                  (((x) & 0x00ff0000UL) >>  8) | \
                  (((x) & 0xff000000UL) >> 24))
#endif

// Info kept for each erase unit by SOTP. Used for both encryption and authentication.
typedef struct {
    uint8_t  cmac[MBEDTLS_CIPHER_BLKSIZE_MAX];
    uint16_t num_erases;
} erase_unit_info_t __attribute__((aligned(sizeof(uint32_t)))); // SOTP requires alignment to 32 bit

static inline uint64_t align_down(uint64_t val, uint32_t size)
{
    return val / size * size;
}

static inline uint64_t align_up(uint64_t val, uint32_t size)
{
    return (((val-1) / size) + 1) * size;
}

// Calculate CMAC functions - start/update and finish

int calc_cmac(mbedtls_cipher_context_t *ctx, int &start, const unsigned char *input, size_t ilen)
{
    int ret;

    if (start) {
        const mbedtls_cipher_info_t *cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB);

        mbedtls_cipher_init(ctx);

        if ((ret = mbedtls_cipher_setup(ctx, cipher_info)) != 0 )
            goto exit;

        ret = mbedtls_cipher_cmac_starts(ctx, auth_key, AUTH_KEY_SIZE_BITS);
        if( ret != 0 )
            goto exit;
        start = 0;
    }

    ret = mbedtls_cipher_cmac_update(ctx, input, ilen);
    if( ret != 0 )
        goto exit;

    return 0;

exit:
    mbedtls_cipher_free( ctx );

    return ret;
}

int finish_cmac(mbedtls_cipher_context_t *ctx, unsigned char *output)
{
    int ret;

    ret = mbedtls_cipher_cmac_finish(ctx, output);

    mbedtls_cipher_free(ctx);

    return ret;
}



int EncryptedBlockDevice::_num_instances = 0;

EncryptedBlockDevice::EncryptedBlockDevice(BlockDevice *bd)
    : _bd(bd), _sotp_start_type(0), _erase_size(0),
      _init_done(0), _num_reads(0), _prev_num_reads(0),
      _num_writes(0), _prev_num_writes(0), _num_erases(0), _prev_num_erases(0)
{
    _num_instances++;
    // Usage of SOTP doesn't allow us to have more than one instance of this class at a time, as
    // dedicated SOTP types are added after the predefined one. This isn't a singleton class per se,
    // as we want to allow creating and destroying instances of it - but just one at a time.
    MBED_ASSERT(_num_instances == 1);
}

EncryptedBlockDevice::~EncryptedBlockDevice()
{
    SOTP &sotp = SOTP::get_instance();
    // Return SOTP to previous state (in case we have an additional init/instantiation)
    sotp.set_num_types(_sotp_start_type);

    _num_instances--;
}

// Calculate CMAC on erase unit (after change) and update SOTP
int EncryptedBlockDevice::sign_erase_unit(uint16_t erase_unit_num, uint16_t num_erases)
{
    mbedtls_cipher_context_t auth_ctx;
    uint8_t read_buf[ENC_BD_READ_BUF_SIZE];
    bd_size_t auth_size = _erase_size;
    bd_addr_t auth_addr = erase_unit_num * _erase_size;
    int cmac_calc_start = 1;
    int ret;
    erase_unit_info_t erase_unit_info;
    SOTP &sotp = SOTP::get_instance();

    while (auth_size) {
        bd_size_t auth_chunk_size = MIN(auth_size, ENC_BD_READ_BUF_SIZE);
        ret = _bd->read(read_buf, auth_addr, align_up(auth_chunk_size, get_read_size()));
        if (ret) {
            return ret;
        }
        ret = calc_cmac(&auth_ctx, cmac_calc_start, read_buf, auth_chunk_size);
        if (ret) {
            return BD_ERROR_ENC_BD_CALC_CMAC_ERROR;
        }
        auth_addr += auth_chunk_size;
        auth_size -= auth_chunk_size;
    }
    ret = finish_cmac(&auth_ctx, erase_unit_info.cmac);
    if (ret) {
        return BD_ERROR_ENC_BD_CALC_CMAC_ERROR;
    }
    erase_unit_info.num_erases = num_erases;
    ret = sotp.set(_sotp_start_type + erase_unit_num, sizeof(erase_unit_info),
                      (uint32_t *)&erase_unit_info);
    if (ret != SOTP_SUCCESS) {
        return BD_ERROR_ENC_BD_SOTP_ERROR;
    }
    return 0;
}

// Authenticate a chunk of data with a given CMAC
int EncryptedBlockDevice::authenticate_chunk(void *auth_ctx, const uint8_t *buf, bd_size_t chunk_size,
                                             bd_size_t &total_size, uint8_t* given_cmac, int &start)
{
    int ret;
    uint8_t calculated_cmac[MBEDTLS_AES_BLOCK_SIZE];
    mbedtls_cipher_context_t *ctx = static_cast<mbedtls_cipher_context_t *> (auth_ctx);
    ret = calc_cmac(ctx, start, buf, chunk_size);
    if (ret) {
        return BD_ERROR_ENC_BD_CALC_CMAC_ERROR;
    }
    total_size -= chunk_size;

    if (!total_size) {
        ret = finish_cmac(ctx, calculated_cmac);
        if (ret) {
            return BD_ERROR_ENC_BD_CALC_CMAC_ERROR;
        }
        if (memcmp(calculated_cmac, given_cmac, MBEDTLS_AES_BLOCK_SIZE)) {
            return BD_ERROR_ENC_BD_AUTHENTICATION_FAILED;
        }
    }
    return 0;
}

// Encrypt a chunk of data
int EncryptedBlockDevice::encrypt_chunk(uint8_t *src_buf, uint8_t *&dst_buf, bd_addr_t addr,
                                        bd_size_t chunk_size, uint16_t iv)
{
    uint32_t counter = (uint32_t) (addr % _erase_size / ENC_BLK_SIZE);
    size_t aes_offs = 0;
    uint8_t stream_block[ENC_BLK_SIZE];
    uint16_t erase_unit_num = addr / _erase_size;

    mbedtls_aes_context enc_aes_ctx;
    mbedtls_aes_init(&enc_aes_ctx);
    mbedtls_aes_setkey_enc(&enc_aes_ctx, encrypt_key, ENC_KEY_SIZE_BITS);

    uint8_t nonce[ENC_BLK_SIZE] = {0};
    memcpy(nonce + NONCE_NUM_ERASES_POS, &iv, sizeof(iv));
    memcpy(nonce + NONCE_ERASE_UNIT_NUM_POS, &erase_unit_num, sizeof(erase_unit_num));

    // Counter in nonce should be represented in big endian
    counter = HTONL(counter);
    memcpy(nonce + NONCE_CTR_POS, &counter, sizeof(counter));

    bd_size_t part_offs = addr % ENC_BLK_SIZE;
    memcpy(dst_buf + part_offs, src_buf, chunk_size);
    mbedtls_aes_crypt_ctr(&enc_aes_ctx, align_up(chunk_size, ENC_BLK_SIZE), &aes_offs, nonce,
                          stream_block, dst_buf, dst_buf);
    dst_buf += part_offs;

    return 0;
}

// Decrypt a chunk of data
int EncryptedBlockDevice::decrypt_chunk(uint8_t *src_buf, uint8_t *&dst_buf, bd_addr_t &addr,
                                        bd_addr_t aligned_addr, bd_size_t chunk_size, bd_size_t &total_size,
                                        uint16_t iv)
{
    bd_addr_t enc_offs = addr - aligned_addr;
    bd_size_t enc_size = MIN(chunk_size - enc_offs, total_size);
    uint32_t counter = (uint32_t) (addr % _erase_size / ENC_BLK_SIZE);
    size_t aes_offs = 0;
    uint8_t stream_block[ENC_BLK_SIZE];
    uint16_t erase_unit_num = addr / _erase_size;

    mbedtls_aes_context enc_aes_ctx;
    mbedtls_aes_init(&enc_aes_ctx);
    mbedtls_aes_setkey_enc(&enc_aes_ctx, encrypt_key, ENC_KEY_SIZE_BITS);

    uint8_t nonce[ENC_BLK_SIZE] = {0};
    memcpy(nonce + NONCE_NUM_ERASES_POS, &iv, sizeof(iv));
    memcpy(nonce + NONCE_ERASE_UNIT_NUM_POS, &erase_unit_num, sizeof(erase_unit_num));

    // Counter in nonce should be represented in big endian
    counter = HTONL(counter);
    memcpy(nonce + NONCE_CTR_POS, &counter, sizeof(counter));

    // Can't decrypt a partial block, so start decrypting from a block boundary (in place),
    // and copy the partial block to the user buffer.
    bd_size_t part_offs = addr % ENC_BLK_SIZE;
    uint8_t *enc_buf = src_buf + enc_offs - part_offs;
    mbedtls_aes_crypt_ctr(&enc_aes_ctx, align_up(enc_size, ENC_BLK_SIZE), &aes_offs, nonce,
                          stream_block, enc_buf, enc_buf);
    memcpy(dst_buf, enc_buf + part_offs, enc_size);

    dst_buf += enc_size;
    addr += enc_size;
    total_size -= enc_size;

    return 0;
}


int EncryptedBlockDevice::init()
{
    int err = _bd->init();
    if (err) {
        return err;
    }

    // User may call init function multiple times. If already called, stop here, as other than underlying
    // device initialization, all other init actions are one time actions.
    if (_init_done) {
        return BD_ERROR_OK;
    }
    _erase_size = 512; //_bd->get_erase_size();

    SOTP &sotp = SOTP::get_instance();

    // Add SOTP types for our device (one per erase block)
    _sotp_start_type = sotp.get_num_types();
    sotp.set_num_types(_sotp_start_type + size() / _erase_size);

    MBED_ASSERT(ENC_BD_READ_BUF_SIZE % get_read_size() == 0);
    MBED_ASSERT(ENC_BD_READ_BUF_SIZE % ENC_BLK_SIZE == 0);
    MBED_ASSERT(ENC_BD_READ_BUF_SIZE >= get_read_size());

    MBED_ASSERT(ENC_BD_WRITE_BUF_SIZE % get_program_size() == 0);
    MBED_ASSERT(ENC_BD_WRITE_BUF_SIZE % ENC_BLK_SIZE == 0);
    MBED_ASSERT(ENC_BD_WRITE_BUF_SIZE >= get_program_size());

    _init_done = 1;

    return BD_ERROR_OK;
}

int EncryptedBlockDevice::deinit()
{

    MBED_ASSERT(_init_done);
    // Leave all memory deallocations to the destructor

    return _bd->deinit();
}

int EncryptedBlockDevice::read(void *buffer, bd_addr_t addr_, bd_size_t size_)
{
    perf_enter(PERF_ENC);

    while (size_ > 0) {
        bd_addr_t addr = addr_;
        bd_size_t size = _erase_size;
        //printf("read %llx:%lld (%08x)\n", addr_, size_, ((uint32_t*)buffer)[0]);

    MBED_ASSERT(is_valid_read(addr, size));
    MBED_ASSERT(_init_done);

    _num_reads++;

    uint8_t *buf = (uint8_t *) buffer;
    bd_size_t prev_erase_unit_num = (bd_size_t) -1;
    SOTP &sotp = SOTP::get_instance();
    bd_size_t auth_size = 0;
    uint8_t read_buf[ENC_BD_READ_BUF_SIZE];
    int cmac_calc_start;
    bd_addr_t auth_addr = 0;
    mbedtls_cipher_context_t auth_ctx;

    while (size || auth_size) {
        uint16_t erase_unit_num = (uint16_t) (addr / _erase_size);

        erase_unit_info_t erase_unit_info;
        if (erase_unit_num != prev_erase_unit_num) {
            uint16_t ret_len;
            sotp_result_e result;

            result = sotp.get(_sotp_start_type + erase_unit_num, sizeof(erase_unit_info_t),
                              (uint32_t *)&erase_unit_info, &ret_len);
            // In case we have an SOTP not found error, it means we are reading from an unwritten erase
            // unit (which is legal). Code below knows not to authenticate this data or decrypt it.
            // However, we still need to deduct this part from from the total size.
            if (result == SOTP_NOT_FOUND) {
                auth_size = 0;
                size -= MIN(addr % _erase_size + size, _erase_size);
                erase_unit_info.num_erases = 0;
            }
            else if (result == SOTP_SUCCESS) {
                // This means we need to start cmac calculation from the beginning
                cmac_calc_start = 1;
                auth_size = _erase_size;
            }
            else {
                perf_exit(PERF_ENC);
                return BD_ERROR_ENC_BD_SOTP_ERROR;
            }
            auth_addr = align_down(addr, _erase_size);

            prev_erase_unit_num = erase_unit_num;
        }

        bd_size_t auth_chunk_size = MIN(auth_size, ENC_BD_READ_BUF_SIZE);
        int ret = _bd->read(read_buf, auth_addr, align_up(auth_chunk_size, get_read_size()));
        if (ret) {
            perf_exit(PERF_ENC);
            return ret;
        }

        // First authenticate currently read chunk
        if (auth_chunk_size) {
            ret = authenticate_chunk(&auth_ctx, read_buf, auth_chunk_size, auth_size,
                                     erase_unit_info.cmac, cmac_calc_start);
            if (ret) {
                perf_exit(PERF_ENC);
                return ret;
            }
        }

        // Now decrypt the chunk
        if ((addr >= auth_addr) && (addr < auth_addr + auth_chunk_size)) {
            // User address is within authenticated chunk - decrypt it
            decrypt_chunk(read_buf, buf, addr, auth_addr, auth_chunk_size, size, erase_unit_info.num_erases);
        }
        auth_addr += auth_chunk_size;
    }

        addr_ += _erase_size;
        size_ -= _erase_size;
        buffer = (uint8_t*)buffer + _erase_size;
    }

    perf_exit(PERF_ENC);
    return BD_ERROR_OK;
}

int EncryptedBlockDevice::program(const void *buffer, bd_addr_t addr_, bd_size_t size_)
{
    perf_enter(PERF_ENC);

    while (size_ > 0) {
        bd_addr_t addr = addr_;
        bd_size_t size = _erase_size;
        //printf("prog %llx:%lld (%08x)\n", addr_, size_, ((uint32_t*)buffer)[0]);

    MBED_ASSERT(is_valid_program(addr, size));
    MBED_ASSERT(_init_done);

    _num_writes++;

    mbedtls_aes_context enc_aes_ctx;
    mbedtls_aes_init(&enc_aes_ctx);
    mbedtls_aes_setkey_enc(&enc_aes_ctx, encrypt_key, ENC_KEY_SIZE_BITS);

    uint8_t *src_buf = (uint8_t *) buffer;
    bd_size_t prev_erase_unit_num = (bd_size_t) -1;
    SOTP &sotp = SOTP::get_instance();
    uint8_t write_buf[ENC_BD_READ_BUF_SIZE];

    while (size) {
        uint16_t erase_unit_num = (uint16_t) (addr / _erase_size);
        erase_unit_info_t erase_unit_info;

        if (erase_unit_num != prev_erase_unit_num) {
            uint16_t ret_len;
            sotp_result_e result;

            result = sotp.get(_sotp_start_type + erase_unit_num, sizeof(erase_unit_info),
                              (uint32_t *)&erase_unit_info, &ret_len);
            if (result == SOTP_NOT_FOUND) {
                erase_unit_info.num_erases =  0;
            }
            if (result != SOTP_SUCCESS) {
                perf_exit(PERF_ENC);
                return BD_ERROR_ENC_BD_SOTP_ERROR;
            }

            prev_erase_unit_num = erase_unit_num;
        }

        // First encrypt current chunk
        bd_size_t chunk_size = MIN(_erase_size - addr % _erase_size,
                                  ENC_BD_READ_BUF_SIZE - addr % ENC_BD_READ_BUF_SIZE);
        chunk_size = MIN(chunk_size, size);
        // encrypt function will return the location in write buffer to this variable
        uint8_t *prog_buf = write_buf;
        encrypt_chunk(src_buf, prog_buf, addr, chunk_size, erase_unit_info.num_erases);
        int ret = _bd->program(prog_buf, addr, chunk_size);
        if (ret) {
            perf_exit(PERF_ENC);
            return ret;
        }
        src_buf += chunk_size;
        addr += chunk_size;
        size -= chunk_size;

        // Now need to sign erase unit again, as its data was changed
        if (!size || !(addr % _erase_size)) {
            ret = sign_erase_unit(erase_unit_num, erase_unit_info.num_erases);
            if (ret) {
                perf_exit(PERF_ENC);
                return ret;
            }
        }
    }

        addr_ += _erase_size;
        size_ -= _erase_size;
        buffer = (uint8_t*)buffer + _erase_size;
    }

    perf_exit(PERF_ENC);
    return BD_ERROR_OK;
}

int EncryptedBlockDevice::erase(bd_addr_t addr_, bd_size_t size_)
{
    perf_enter(PERF_ENC);
    int ret = _bd->erase(addr_, size_);
    if (ret) {
        perf_exit(PERF_ENC);
        return ret;
    }


    while (size_ > 0) {
        bd_addr_t addr = addr_;
        bd_size_t size = _erase_size;
    _num_erases++;

    SOTP &sotp = SOTP::get_instance();

    while (size) {
        uint16_t erase_unit_num = (uint16_t) (addr / _erase_size);
        erase_unit_info_t erase_unit_info;
        uint16_t ret_len;
        sotp_result_e result;
        int ret;

        result = sotp.get(_sotp_start_type + erase_unit_num, sizeof(erase_unit_info),
                          (uint32_t *)&erase_unit_info, &ret_len);
        if (result == SOTP_NOT_FOUND) {
            erase_unit_info.num_erases = 0;
        }
        else if (result != SOTP_SUCCESS) {
            perf_exit(PERF_ENC);
            return BD_ERROR_ENC_BD_SOTP_ERROR;
        }
        else {
            erase_unit_info.num_erases++;
        }

        ret = sign_erase_unit(erase_unit_num, erase_unit_info.num_erases);
        if (ret) {
            perf_exit(PERF_ENC);
            return ret;
        }

        bd_size_t chunk_size = MIN(_erase_size, size);
//        ret = _bd->erase(addr, chunk_size);
//        if (ret) {
//            perf_exit(PERF_ENC);
//            return ret;
//        }
        addr += chunk_size;
        size -= chunk_size;
    }

        addr_ += _erase_size;
        size_ -= _erase_size;
    }

    perf_exit(PERF_ENC);
    return BD_ERROR_OK;
}

bd_size_t EncryptedBlockDevice::get_read_size() const
{
    return _bd->get_read_size();
}

bd_size_t EncryptedBlockDevice::get_program_size() const
{
    return _bd->get_program_size();
}

bd_size_t EncryptedBlockDevice::get_erase_size() const
{
    return _bd->get_erase_size();
}

bd_size_t EncryptedBlockDevice::size() const
{
    return _bd->size();
}

bd_size_t EncryptedBlockDevice::get_num_reads()
{
    bd_size_t num = _num_reads - _prev_num_reads;
    _prev_num_reads = _num_reads;
    return num;
}

bd_size_t EncryptedBlockDevice::get_num_writes()
{
    bd_size_t num = _num_writes - _prev_num_writes;
    _prev_num_writes = _num_writes;
    return num;
}

bd_size_t EncryptedBlockDevice::get_num_erases()
{
    bd_size_t num = _num_erases - _prev_num_erases;
    _prev_num_erases = _num_erases;
    return num;
}

