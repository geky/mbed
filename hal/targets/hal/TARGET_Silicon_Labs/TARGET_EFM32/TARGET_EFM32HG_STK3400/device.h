/***************************************************************************//**
 * @file device.h
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/
#ifndef MBED_DEVICE_H
#define MBED_DEVICE_H















// Redefine OPEN_MAX from sys_limits.h to save on RAM.
// Effect: maximum amount of file handlers = OPEN_MAX
// This is not going to have an impact, since this is a RAM-limited part anyway.
#define OPEN_MAX                24

#include "objects.h"
#include "Modules.h"
#include "device_peripherals.h"

#endif
