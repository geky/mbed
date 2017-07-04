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
#include "SimulatedPeripheral.h"
#include "SimulatedSerial.h"
#include <stdio.h>

SimulatedPeripheral *SimulatedPeripheral::peripheral_list;

class PinEntry {
public:
    PinEntry(PinName pin0, PinName pin1, PinName pin2, PinName pin3, PinName pin4) : next(0) {
        pin[0] = pin0;
        pin[1] = pin1;
        pin[2] = pin2;
        pin[3] = pin3;
        pin[4] = pin4;
    }

    PinName pin[5];
    PinEntry *next;

    bool operator ==(const PinEntry &b) const {
        for (uint32_t i = 0; i < sizeof(pin) / sizeof(pin[0]); i++) {
            if (pin[i] != b.pin[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator !=(const PinEntry &b) const {
        return !(*this == b);
    }

};

class SimulatedTerminalSerial : public SimulatedSerial {
public:
    SimulatedTerminalSerial() : SimulatedSerial("SerialStdIn") {

    }

    virtual void init(serial_t *obj, PinName tx, PinName rx) {

    }

    virtual void free(serial_t *obj) {

    }

    virtual void baud(serial_t *obj, int baudrate) {

    }

    virtual void format(serial_t *obj, int data_bits, SerialParity parity, int stop_bits) {

    }

    virtual void irq_handler(serial_t *obj, uart_irq_handler handler, uint32_t id) {

    }

    virtual void irq_set(serial_t *obj, SerialIrq irq, uint32_t enable) {

    }

    virtual int  getc(serial_t *obj) {
        return ::getc(stdin);
    }

    virtual void putc(serial_t *obj, int c) {
        ::putc(c, stdout);
    }

    virtual int  readable(serial_t *obj) {
        return 0;
    }

    virtual int  writable(serial_t *obj) {
        return 1;
    }

    virtual void clear(serial_t *obj) {

    }

    virtual void break_set(serial_t *obj) {

    }

    virtual void break_clear(serial_t *obj) {

    }

    virtual void pinout_tx(PinName tx) {

    }

    virtual void set_flow_control(serial_t *obj, FlowControl type, PinName rxflow, PinName txflow) {

    }

    virtual ~SimulatedTerminalSerial() {

    }
};

static void setup_default_peripherals(bool reset)
{
    static SimulatedTerminalSerial serial;
    static bool init_done = false;
    if (reset || !init_done) {
        //TODO - reset
        serial.add_pins(USBTX, USBRX);
    }
}

SimulatedPeripheral* SimulatedPeripheral::lookup(PeripheralType type, PinName pin0, PinName pin1, PinName pin2, PinName pin3, PinName pin4)
{
    setup_default_peripherals(false);
    PinEntry pin_to_find(pin0, pin1, pin2, pin3, pin4);
    for (SimulatedPeripheral *per = peripheral_list; per != 0; per = per->next) {
        for (PinEntry *pin = per->pins; pin != 0; pin = pin->next) {
            if (pin_to_find == *pin) {
                return per;
            }
        }
    }
    return 0;
}

void SimulatedPeripheral::reset()
{
    //TODO - reset all simulator state
    // -Set time back to 0
    // -Reset default peripherals
    // -Reset peripheral list
    // -Reset timers
    // -Clear all scheduled interrupts
}

SimulatedPeripheral::SimulatedPeripheral(const char *name, const PeripheralType type) : name(name), type(type), next(0), pins(0)
{
    // No work to do
}

/**
 * Register this peripheral so it can be used from the HAL
 */
void SimulatedPeripheral::_attach()
{
    // Check if this peripheral is already attached
    for (SimulatedPeripheral *per = peripheral_list; per != 0; per = per->next) {
        if (this == per) {
            return;
        }
    }

    this->next = peripheral_list;
    peripheral_list = this;

}

void SimulatedPeripheral::_detach()
{
    // 'this' is at the head of the list
    if (this == peripheral_list) {
        peripheral_list = this->next;
        this->next = 0;
        return;
    }

    // 'this' is somewhere else in the list
    SimulatedPeripheral *prev = 0;
    for (SimulatedPeripheral *per = peripheral_list; per != 0; per = per->next) {
        if (this == per) {
            // Found self
            if (0 != prev) {
                prev->next = this->next;
            }
            this->next = 0;
            return;
        }
        prev = per;
    }
}

void SimulatedPeripheral::_add_pins(PinName pin0, PinName pin1, PinName pin2, PinName pin3, PinName pin4)
{
    PinEntry *new_pin = new PinEntry(pin0, pin1, pin2, pin3, pin4);
    new_pin->next = pins;
    pins = new_pin;
    _attach();
}

/**
 * Remove pins this peripheral is allowed to be instantiated on
 */
void SimulatedPeripheral::_remove_pins(PinName pin0, PinName pin1, PinName pin2, PinName pin3, PinName pin4)
{
    PinEntry entry(pin0, pin1, pin2, pin3, pin4);

    // 'entry' is at the head of the list
    if (*pins == entry) {
        PinEntry *tmp = pins;
        pins = tmp->next;
        delete tmp;
    }

    // 'entry' is somewhere else in the list
    PinEntry *prev = 0;
    for (PinEntry* tmp = pins; tmp != 0; tmp = tmp->next) {
        if (*tmp == entry) {
            if (prev != 0) {
                prev->next = tmp->next;
            }
            delete tmp;
            return;
        }
    }

    if (0 == pins) {
        _detach();
    }
}

SimulatedPeripheral::~SimulatedPeripheral()
{
    PinEntry *pin = pins;
    while (pin != 0) {
        PinEntry *next = pin->next;
        delete pin;
        pin = next;
    }
    pins = 0;
    _detach();
}
