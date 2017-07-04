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
#ifndef MBED_SIMULATED_SERIAL_H
#define MBED_SIMULATED_SERIAL_H

#include "SimulatedPeripheral.h"
#include "serial_api.h"

class SimulatedSerial : public SimulatedPeripheral {
public:
    SimulatedSerial(const char *name = "Serial") : SimulatedPeripheral(name, SimulatedPeripheral::SERIAL) {
        // No work to do
    }

    void add_pins(PinName tx, PinName rx) {
        _add_pins(tx, rx);
    }

    void remove_pins(PinName tx, PinName rx) {
        _remove_pins(tx, rx);
    }

    virtual void init(serial_t *obj, PinName tx, PinName rx) = 0;

    virtual void free(serial_t *obj) = 0;

    virtual void baud(serial_t *obj, int baudrate) = 0;

    virtual void format(serial_t *obj, int data_bits, SerialParity parity, int stop_bits) = 0;

    virtual void irq_handler(serial_t *obj, uart_irq_handler handler, uint32_t id) = 0;

    virtual void irq_set(serial_t *obj, SerialIrq irq, uint32_t enable) = 0;

    virtual int  getc(serial_t *obj) = 0;

    virtual void putc(serial_t *obj, int c) = 0;

    virtual int  readable(serial_t *obj) = 0;

    virtual int  writable(serial_t *obj) = 0;

    virtual void clear(serial_t *obj) = 0;

    virtual void break_set(serial_t *obj) = 0;

    virtual void break_clear(serial_t *obj) = 0;

    virtual void pinout_tx(PinName tx) = 0;

    virtual void set_flow_control(serial_t *obj, FlowControl type, PinName rxflow, PinName txflow) = 0;

    virtual ~SimulatedSerial() {};
};

#endif
