/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdio>

#define MAX_BUFF_SIZE 10

static DigitalOut led(D7);

BufferedSerial pc(USBTX, USBRX);



int main()
{
    pc.set_baud(115200);

    char buf[MAX_BUFF_SIZE];
    while (true)
    {
        if (pc.readable()) {
        int len = pc.read(buf, sizeof(buf)-1);
        buf[len] = '\0';

        if (strcmp(buf, "1")==0) {
        led = 1;
        pc.write("LED is ON\n", 11);
        }

        else if (strcmp(buf, "0")==0) {
        led = 0;
        pc.write("LED is OFF\n", 12);
        }

        else{
            pc.write("Input not recognised", 22);
        }
        }
    }
}
