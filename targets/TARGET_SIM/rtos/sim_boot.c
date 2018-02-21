/*
 * Copyright (c) 2017-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "cmsis_os2.h"
#include "mbed_rtos_storage.h"
#include "mbed_toolchain.h"
#include <string.h>
#include <stdio.h>

osMutexId_t               singleton_mutex_id;
mbed_rtos_storage_mutex_t singleton_mutex_obj;
osMutexAttr_t             singleton_mutex_attr;

/* mbed_main is a function that is called before main()
 * mbed_sdk_init() is also a function that is called before main(), but unlike
 * mbed_main(), it is not meant for user code, but for the SDK itself to perform
 * initializations before main() is called.
 */
MBED_WEAK void mbed_main(void) 
{
}

extern int __real_main(void);

MBED_WEAK int __wrap_main(void) 
{
    osKernelInitialize();
    memset((void*)&singleton_mutex_attr, 0, sizeof(singleton_mutex_attr));
    singleton_mutex_attr.cb_mem = &singleton_mutex_obj;
    singleton_mutex_attr.cb_size = sizeof(singleton_mutex_obj);
    singleton_mutex_attr.attr_bits = osMutexRecursive | osMutexRobust | osMutexPrioInherit;
    singleton_mutex_id = osMutexNew(&singleton_mutex_attr);
    mbed_main();
    return __real_main();
}
