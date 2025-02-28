/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdio>

#define SLEEP_TIMER 100ms // definerer oppdateringsfrekvensen som 10Hz / oppdaterer hver 100ms

AnalogIn potMeter(A0);      //definerer potensiometer input på port A0 som "potMeter"
DigitalIn button(D3);  //definerer knapp input fra knapp på brettet som "button"

//definerer de tre diodene som "ledX" på utgangene D0-D2
PwmOut led1(D0);
PwmOut led2(D1);
PwmOut led3(D2);


int main()
{
    
    //initialiserer verdiene for diode variablene
    led1 = 0.0f;
    led2 = 0.0f;
    led3 = 0.0f;

    while (true) 
    {
        ThisThread::sleep_for(SLEEP_TIMER); // setter oppdateringsfrekvensen til 10Hz / 10 oppdateringer per sekund

        float potValue = potMeter.read(); //leser potensiometer verdien og lagrer den i variablen "potValue"

        if (button == 0) //hvis knappen trykkes lyser alle dioder -- knapp er pull-up
        {
            led1 = 1.0f;
            led2 = 1.0f;
            led3 = 1.0f;
        }

        else 
        {
            if(potValue < (1.0f/3.0f)) // hvis potValue < 1/3 lyser en diode -- styrke avhengig av avlesning
            {
                float frac = potValue * 3.0f;
                led1 = frac;
                led2 = 0.0f;
                led3 = 0.0f;
            }
            else if(potValue <(2.0f/3.0f)) // hvis potValue verdien er mellom 1/3 og 2/3 lyser to dioder -- styrke på andre diode avhengig av avlesning
            {
                float frac = (potValue - 1.0f/3.0f) * 3.0f;
                led1 = 1.0f;
                led2 = frac;
                led3 = 0.0f;
            }
            else // hvis potValue er >2/3 lyser alle dioder -- styrke på siste diode avhengig av avlesning
            {
                float frac = (potValue - 2.0f/3.0f) * 3.0f;
                led1 = 1.0f;
                led2 = 1.0f;
                led3 = frac;
            }
        }
    }
}
