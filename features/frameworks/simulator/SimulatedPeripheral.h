/* mbed Microcontroller Library
 * Copyright (c) 2017-2017 ARM Limited
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
#ifndef MBED_SIMULATED_PERIPHERAL_H
#define MBED_SIMULATED_PERIPHERAL_H

#include "platform/platform.h"

class PinEntry;
class SimulatedPeripheral {
public:
    enum PeripheralType {
        ANALOGIN,
        ANALOGOUT,
        I2C,
        I2CSLAVE,
        INTERRUPTIN,
        PORTIN,
        PORTINOUT,
        PORTOUT,
        PWMOUT,
        RTC,
        SERIAL,
        SPI,
        TRNG,
        EMAC,
    };

    /**
     * Lookup the peripheral of the given type with the given pins
     */
    static SimulatedPeripheral* lookup(PeripheralType type, PinName pin0=NC, PinName pin1=NC, PinName pin2=NC, PinName pin3=NC, PinName pin4=NC);

    /**
     * Reset built-in peripherals and remove user added peripherals
     */
    static void reset();

protected:

    SimulatedPeripheral(const char *name, const PeripheralType type);

    /**
     * Register this peripheral so it can be used from the HAL
     */
    void _attach();

    /**
     * Unregister this peripheral so it is no available in the HAL
     */
    void _detach();

    /**
     * Add pins this peripheral is allowed to be instantiated on
     */
    void _add_pins(PinName pin0=NC, PinName pin1=NC, PinName pin2=NC, PinName pin3=NC, PinName pin4=NC);

    /**
     * Remove pins this peripheral is allowed to be instantiated on
     */
    void _remove_pins(PinName pin0=NC, PinName pin1=NC, PinName pin2=NC, PinName pin3=NC, PinName pin4=NC);

    virtual ~SimulatedPeripheral();

private:
    const char *const name;
    const PeripheralType type;
    SimulatedPeripheral *next;
    PinEntry *pins;
    static SimulatedPeripheral *peripheral_list;
};

#endif
