/* mbed Microcontroller Library
 * Copyright (c) 2016 ARM Limited
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

#include "block_api.h"


bd_error_t bd_init(block_device_t *bd) {
    if (!bd) {
        return BD_ERROR_PARAMETER;
    } else if (!bd->ops->init) {
        return BD_ERROR_UNSUPPORTED;
    }

    return bd->ops->init(bd->bd);
}

bd_error_t bd_deinit(block_device_t *bd) {
    if (!bd) {
        return BD_ERROR_PARAMETER;
    } else if (!bd->ops->deinit) {
        return BD_ERROR_UNSUPPORTED;
    }

    return bd->ops->deinit(bd->bd);
}

bd_count_or_error_t bd_read(block_device_t *bd,
        void *buffer, bd_block_t block, bd_count_t count) {
    if (!bd) {
        return BD_ERROR_PARAMETER;
    } else if (!bd->ops->read) {
        return BD_ERROR_UNSUPPORTED;
    }

    return bd->ops->read(bd->bd, buffer, block, count);
}

bd_count_or_error_t bd_write(block_device_t *bd,
        const void *buffer, bd_block_t block, bd_count_t count) {
    if (!bd) {
        return BD_ERROR_PARAMETER;
    } else if (!bd->ops->write) {
        return BD_ERROR_UNSUPPORTED;
    }

    return bd->ops->write(bd->bd, buffer, block, count);
}

bd_error_t bd_sync(block_device_t *bd) {
    if (!bd) {
        return BD_ERROR_PARAMETER;
    } else if (!bd->ops->sync) {
        return BD_ERROR_OK;
    }

    return bd->ops->sync(bd->bd);
}

bd_status_t bd_get_status(block_device_t *bd) {
    if (!bd || !bd->ops->get_status) {
        return BD_STATUS_UNINITIALIZED;
    }

    return bd->ops->get_status(bd->bd);
}

bd_size_t bd_get_block_size(block_device_t *bd) {
    if (!bd || !bd->ops->get_block_size) {
        return 0;
    }

    return bd->ops->get_block_size(bd->bd);
}

bd_count_t bd_get_block_count(block_device_t *bd) {
    if (!bd || !bd->ops->get_block_count) {
        return 0;
    }

    return bd->ops->get_block_count(bd->bd);
}
