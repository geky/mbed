/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SOTP_H
#define __SOTP_H

#include <stdint.h>
#include "sotp_int_flash_wrapper.h"
#include "sotp_os_wrapper.h"

#ifdef SOTP_TESTING
#undef SOTP_PROBE_ONLY
#endif

typedef enum {
    SOTP_SUCCESS                = 0,
    SOTP_READ_ERROR             = 1,
    SOTP_WRITE_ERROR            = 2,
    SOTP_NOT_FOUND              = 3,
    SOTP_DATA_CORRUPT           = 4,
    SOTP_BAD_VALUE              = 5,
    SOTP_BUFF_TOO_SMALL         = 6,
    SOTP_FLASH_AREA_TOO_SMALL   = 7,
    SOTP_OS_ERROR               = 8,
    SOTP_BUFF_NOT_ALIGNED       = 9,
    SOTP_ALREADY_EXISTS         = 10,
    SOTP_ERROR_MAXVAL           = 0xFFFF
} sotp_result_e;

#define SOTP_MASTER_RECORD_TYPE 0xFE
#define SOTP_NO_TYPE            0xFF

#ifndef SOTP_MAX_TYPES
#define SOTP_MAX_TYPES 16
#endif

class SOTP {
public:

/**
 * @brief As a singleton, return the single instance of the class.
 *        Reason for this class being a singleton is the following:
 *        - Ease the use for users of this class not having to coordinate instantiations.
 *        - Lazy instantiation of internal data (which we can't achieve with simple static classes).
 *
 * @returns Singleton instance reference.
 */
    static SOTP& get_instance()
    {
        // Use this implementation of singleton (Meyer's) rather than the one that allocates
        // the instance on the heap, as it ensures destruction at program end (preventing warnings
        // from memory checking tools such as valgrind).
        static SOTP instance;
        return instance;
    }

    virtual ~SOTP();

/**
 * @brief Returns number of types.
 *
 * @returns Number of types.
 */
    uint8_t get_num_types();

/**
 * @brief Set number of types.
 *
 * @returns None.
 */
    void set_num_types(uint8_t in_num_types);

/**
 * @brief Returns one item of data programmed on Flash, given type.
 *
 * @param [in] type
 *               Type of stored item.
 *
 * @param [in] buf_len_bytes
 *               Length of input buffer in bytes.
 *
 * @param [in] buf
 *               Buffer to store data on (must be aligned to a 32 bit boundary).
 *
 * @param [out] actual_len_bytes.
 *               Actual length of returned data
 *
 * @returns SOTP_SUCCESS           Value was found on Flash.
 *          SOTP_NOT_FOUND         Value was not found on Flash.
 *          SOTP_READ_ERROR        Physical error reading data.
 *          SOTP_DATA_CORRUPT      Data on Flash is corrupt.
 *          SOTP_BAD_VALUE         Bad value in any of the parameters.
 *          SOTP_BUFF_TOO_SMALL    Not enough memory in user buffer.
 *          SOTP_BUFF_NOT_ALIGNED  Buffer not aligned to 32 bits.
 */
    sotp_result_e get(uint8_t type, uint16_t buf_len_bytes, uint32_t *buf, uint16_t *actual_len_bytes);

/**
 * @brief Returns one item of data programmed on Flash, given type.
 *
 * @param [in] type
 *               Type of stored item.
 *
 * @param [out] actual_len_bytes.
 *               Actual length of item
 *
 * @returns SOTP_SUCCESS           Value was found on Flash.
 *          SOTP_NOT_FOUND         Value was not found on Flash.
 *          SOTP_READ_ERROR        Physical error reading data.
 *          SOTP_DATA_CORRUPT      Data on Flash is corrupt.
 *          SOTP_BAD_VALUE         Bad value in any of the parameters.
 */
    sotp_result_e get_item_size(uint8_t type, uint16_t *actual_len_bytes);


/**
 * @brief Programs one item of data on Flash, given type.
 *
 * @param [in] type
 *               Type of stored item.
 *
 * @param [in] buf_len_bytes
 *               Item length in bytes.
 *
 * @param [in] buf
 *               Buffer containing data  (must be aligned to a 32 bit boundary).
 *
 * @returns SOTP_SUCCESS           Value was successfully written on Flash.
 *          SOTP_WRITE_ERROR       Physical error writing data.
 *          SOTP_BAD_VALUE         Bad value in any of the parameters.
 *          SOTP_FLASH_AREA_TOO_SMALL
 *                                 Not enough space in Flash area.
 *          SOTP_BUFF_NOT_ALIGNED  Buffer not aligned to 32 bits.
 *          SOTP_ALREADY_EXISTS    Item (OTP type) already exists.
 *
 */
    sotp_result_e set(uint8_t type, uint16_t buf_len_bytes, const uint32_t *buf);

#ifdef SOTP_TESTING
/**
 * @brief Remove an item from flash.
 *
 * @param [in] type
 *               Type of stored item.
 *
 * @param [in] buf_len_bytes
 *               Item length in bytes.
 *
 * @param [in] buf
 *               Buffer containing data  (must be aligned to a 32 bit boundary).
 *
 * @returns SOTP_SUCCESS           Value was successfully written on Flash.
 *          SOTP_WRITE_ERROR       Physical error writing data.
 *          SOTP_BAD_VALUE         Bad value in any of the parameters.
 *          SOTP_FLASH_AREA_TOO_SMALL
 *                                 Not enough space in Flash area.
 *          SOTP_BUFF_NOT_ALIGNED  Buffer not aligned to 32 bits.
 *
 */
    sotp_result_e remove(uint8_t type);

#endif

/**
 * @brief Initializes SOTP component.
 *
 * @returns SOTP_SUCCESS       Initialization completed successfully.
 *          SOTP_READ_ERROR    Physical error reading data.
 *          SOTP_WRITE_ERROR   Physical error writing data (on recovery).
 *          SOTP_FLASH_AREA_TOO_SMALL
 *                             Not enough space in Flash area.
 */
    sotp_result_e init();

/**
 * @brief Deinitializes SOTP component.
 *        Warning: This function is not thread safe and should not be called
 *        concurrently with other SOTP functions.
 *
 * @returns SOTP_SUCCESS       Deinitialization completed successfully.
 */
    sotp_result_e deinit();

/**
 * @brief Reset Flash SOTP areas.
 *        Warning: This function is not thread safe and should not be called
 *        concurrently with other SOTP functions.
 *
 * @returns SOTP_SUCCESS       Reset completed successfully.
 *          SOTP_READ_ERROR    Physical error reading data.
 *          SOTP_WRITE_ERROR   Physical error writing data.
 */
    sotp_result_e reset();

#ifdef SOTP_TESTING

/**
 * @brief Initiate a forced garbage collection.
 *
 * @returns SOTP_SUCCESS       GC completed successfully.
 *          SOTP_READ_ERROR    Physical error reading data.
 *          SOTP_WRITE_ERROR   Physical error writing data.
 *          SOTP_FLASH_AREA_TOO_SMALL
 *                             Not enough space in Flash area.
 */
    sotp_result_e force_garbage_collection();
#endif

/**
 * @brief Returns one item of data programmed on Flash, given type.
 *        This is the "initless" version of the function, traversing the flash if triggered.
 *
 * @param [in] type
 *               Type of stored item (must be between 0-15).
 *
 * @param [in] buf_len_bytes
 *               Length of input buffer in bytes.
 *
 * @param [in] buf
 *               Buffer to store data on (must be aligned to a 32 bit boundary).
 *
 * @param [out] actual_len_bytes.
 *               Actual length of returned data
 *
 * @returns SOTP_SUCCESS           Value was found on Flash.
 *          SOTP_NOT_FOUND         Value was not found on Flash.
 *          SOTP_READ_ERROR        Physical error reading data.
 *          SOTP_DATA_CORRUPT      Data on Flash is corrupt.
 *          SOTP_BAD_VALUE         Bad value in any of the parameters.
 *          SOTP_BUFF_TOO_SMALL    Not enough memory in user buffer.
 *          SOTP_BUFF_NOT_ALIGNED  Buffer not aligned to 32 bits.
 */
    sotp_result_e probe(uint8_t type, uint16_t buf_len_bytes, uint32_t *buf, uint16_t *actual_len_bytes);

private:
    int init_done;
    uint32_t init_attempts;
    uint8_t active_area;
    uint8_t num_types;
    uint16_t active_area_version;
    uint32_t free_space_offset;
    sotp_shared_lock_t write_lock;
    uint32_t *offset_by_type;
    sotp_area_data_t *flash_area_params;

    // Private constructor, as class is a singleton
    SOTP();

    // Declare these (not implemented) to make sure they're not accidentally declared
    // by derived classes
    SOTP(SOTP const&);
    void operator=(SOTP const&);

    int flash_read_area(uint8_t area, uint32_t offset, uint32_t len_bytes, uint32_t *buf);
    int flash_write_area(uint8_t area, uint32_t offset, uint32_t len_bytes, const uint32_t *buf);
    int flash_erase_area(uint8_t area);

    int calc_empty_space(uint8_t area, uint32_t *offset);

    sotp_result_e read_record(uint8_t area, uint32_t offset, uint16_t buf_len_bytes, uint32_t *buf,
                              uint16_t *actual_len_bytes, int validate_only, int *valid,
                              uint8_t *type, uint8_t *flags, uint32_t *next_offset);

    sotp_result_e write_record(uint8_t area, uint32_t offset, uint8_t type, uint8_t flags,
                               uint32_t data_len, const uint32_t *data_buf, uint32_t *next_offset);

    sotp_result_e write_master_record(uint8_t area, uint16_t version, uint32_t *next_offset);

    sotp_result_e copy_record(uint8_t from_area, uint32_t from_offset, uint32_t to_offset,
                              uint32_t *next_offset);

    sotp_result_e garbage_collection(uint8_t type, uint16_t buf_len_bytes, const uint32_t *buf);

    sotp_result_e do_get(uint8_t type, uint16_t buf_len_bytes, uint32_t *buf, uint16_t *actual_len_bytes,
                         int validate_only);

    sotp_result_e do_set(uint8_t type, uint16_t buf_len_bytes, const uint32_t *buf, uint8_t flags);

};


#endif


