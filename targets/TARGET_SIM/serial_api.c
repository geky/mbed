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
#include "serial_api.h"
#include <stdio.h>

int stdio_uart_inited = 0;
serial_t stdio_uart;

void serial_init(serial_t *obj, PinName tx, PinName rx)
{

}

void serial_free(serial_t *obj)
{

}

void serial_baud(serial_t *obj, int baudrate)
{

}

void serial_format(serial_t *obj, int data_bits, SerialParity parity, int stop_bits)
{

}

void serial_irq_handler(serial_t *obj, uart_irq_handler handler, uint32_t id)
{

}

void serial_irq_set(serial_t *obj, SerialIrq irq, uint32_t enable)
{

}

int  serial_getc(serial_t *obj)
{
    return getc(stdin);
}

void serial_putc(serial_t *obj, int c)
{
    putc(c, stdout);
}

int  serial_readable(serial_t *obj)
{
    return 0;
}

int  serial_writable(serial_t *obj)
{
    return 0;
}

void serial_clear(serial_t *obj)
{

}

void serial_break_set(serial_t *obj)
{

}

void serial_break_clear(serial_t *obj)
{

}

void serial_pinout_tx(PinName tx)
{

}

void serial_set_flow_control(serial_t *obj, FlowControl type, PinName rxflow, PinName txflow)
{

}
