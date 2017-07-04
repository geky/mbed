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
#ifndef MBED_SIMULATED_ANALOG_IN_H
#define MBED_SIMULATED_ANALOG_IN_H

#include "SimulatedPeripheral.h"

class SimulatedAnalogIn : public SimulatedPeripheral {
public:
    SimulatedAnalogIn(const char *name = "AnalogIn") : SimulatedPeripheral(name, SimulatedPeripheral::ANALOGIN) {
        // No work to do
    }

    void add_pins(PinName pin) {
        _add_pins(pin);
    }

    void remove_pins(PinName pin) {
        _remove_pins(pin);
    }

    virtual void init(analogin_t *obj, PinName pin) = 0;

    virtual uint16_t read_u16(analogin_t *obj) = 0;

    virtual float read(analogin_t *obj) = 0;

    virtual ~SimulatedAnalogIn() {};
};

#endif
