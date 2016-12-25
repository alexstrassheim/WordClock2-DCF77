/* Arduino DCF77 library v0.1
 * Copyright (C) 2006 Mathias Dalheimer (md@gonium.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 *
 * Modified 2009 by Manfred Fuchs
 * manfred@fuchsrudel.de / http://www.mafu-foto.de
 */

#include <Arduino.h>
#include <WConstants.h>

// include this library's description file
#include "DCF77.h"
#undef abs

DCF77::DCF77(unsigned char dcfPin) {
    DCF77Pin            = dcfPin;

    previousSignalState = 0;
    previousFlankTime   = 0;
    bufferPosition      = 0;
    dcf_rx_buffer       = 0;
    finalized           = false;
    ready               = false;
    parity.parity_flag = parity.parity_min = parity.parity_hour = parity.parity_date = 0;
    ss = mm = hh = day = wkd = mon = year = 0;

    pinMode(DCF77Pin, INPUT);
}

void DCF77::begin() {
    DEBUG_PRINTLN("START DCF");
}

int DCF77::scanSignal(void) {
    finalized = false;
    unsigned char DCFsignal = digitalRead(DCF77Pin);

    if (DCFsignal != previousSignalState) {
        if (DCFsignal == 1) {
            /* We detected a raising flank, increase per one second! */
            int thisFlankTime = millis();

            DEBUG_PRINT("\n\nthis Flank Time: "); DEBUG_PRINT(thisFlankTime);
            DEBUG_PRINT("\nprevious Flank Time: "); DEBUG_PRINT(previousFlankTime);
            DEBUG_PRINT("\nFlank Time: "); DEBUG_PRINT(thisFlankTime-previousFlankTime);

            if (thisFlankTime - previousFlankTime > 300) {
                addSecond();
            }

            if (thisFlankTime - previousFlankTime > DCF_sync_millis) {
                DEBUG_PRINTLN("\n#### Begin of new Minute!!!");
                finalizeBuffer();
            }
            else if (thisFlankTime - previousFlankTime < 300) {
                DEBUG_PRINTLN(" <- Double flank detected\n");

                bufferPosition--;
                if (bufferPosition < 0)
                    bufferPosition = 0;
            }

            if (thisFlankTime - previousFlankTime > 300)
                previousFlankTime = thisFlankTime;

            // DEBUG_PRINTLN(previousFlankTime);
            // DEBUG_PRINTLN("");
            // DEBUG_PRINTLN(": DCF77 Signal detected, ");
            // serialDumpTime(); // DEBUG
        }
        else {
            /* or a falling flank */
            int difference = millis() - previousFlankTime;
            DEBUG_PRINT("duration: ");  DEBUG_PRINT(difference, DEC);

            if (difference < DCF_split_millis) {
                appendSignal(0);
            }
            else {
                appendSignal(1);
            }
        }
        previousSignalState = DCFsignal;
    }
    return DCFsignal;
}

void DCF77::appendSignal(unsigned char signal) {
    DEBUG_PRINT(" position: ");  DEBUG_PRINT(bufferPosition, DEC);
    DEBUG_PRINT(" value: ");     DEBUG_PRINT(signal, DEC);

    // Checking of the parity
    // We check only minute, hour and the date for partiy, so reset the
    // temporaty parity flag when minute, hour or date bit-stream starts
    if (bufferPosition == 21 || bufferPosition == 29 || bufferPosition == 36) {
        // DEBUG_PRINTLN("set parity bit to 0");
        parity.parity_flag = 0;
    }

    // If the last bit of minute, hour or data is received, save our caluculated
    // parity flag
    if (bufferPosition == 28) {
        // DEBUG_PRINTLN("save min parity bit");
        parity.parity_min = parity.parity_flag;
    }

    if (bufferPosition == 35) {
        // DEBUG_PRINTLN("save hour parity bit");
        parity.parity_hour = parity.parity_flag;
    }

    if (bufferPosition == 58) {
        // DEBUG_PRINTLN("save date parity bit");
        parity.parity_date = parity.parity_flag;
    }

    DEBUG_PRINT(" DCF_RX_BUFFER: "); DEBUG_PRINTLN((unsigned long)dcf_rx_buffer);

    DEBUG_PRINT(" SIGNAL: "); DEBUG_PRINTLN((unsigned long)signal);
    DEBUG_PRINT(" BUFFER_POSITION: "); DEBUG_PRINTLN(bufferPosition);
    DEBUG_PRINT(" CALC: "); DEBUG_PRINTLN((unsigned long)signal<<bufferPosition);

    dcf_rx_buffer = dcf_rx_buffer | ((unsigned long long) signal << bufferPosition);

    DEBUG_PRINT(" DCF_RX_BUFFER_C: "); DEBUG_PRINTLN((unsigned long)dcf_rx_buffer);

    if (signal == 1) {
        parity.parity_flag ^= 1;
        // DEBUG_PRINT("toggle parity bit, new value = ");
        // DEBUG_PRINTLN(parity.parity_flag, DEC);
    }

    bufferPosition++;
    if (bufferPosition > 59)
        finalizeBuffer();
}

void DCF77::finalizeBuffer(void) {
    // DEBUG_PRINTLN("Finalizing Buffer");

    if (bufferPosition >= 59) {
        finalized = true;
        rx_buffer = (struct DCF77Buffer *)(unsigned long long)&dcf_rx_buffer;

        // DEBUG_PRINTLN("Check Parity");
        // DEBUG_PRINT("parity_min  = ");
        // DEBUG_PRINT(parity.parity_min, DEC);
        // DEBUG_PRINT(" / P1 = ");
        // DEBUG_PRINT(rx_buffer->P1, DEC);
        // DEBUG_PRINT("parity_hour = ");
        // DEBUG_PRINT(parity.parity_hour, DEC);
        // DEBUG_PRINT(" / P2 = ");
        // DEBUG_PRINT(rx_buffer->P2, DEC);
        // DEBUG_PRINT("parity_date = ");
        // DEBUG_PRINT(parity.parity_date, DEC);
        // DEBUG_PRINT(" / P3 = ");
        // DEBUG_PRINTLN(rx_buffer->P3, DEC);

        if (parity.parity_min  == rx_buffer->P1 &&
                parity.parity_hour == rx_buffer->P2 &&
                parity.parity_date == rx_buffer->P3) {

            // DEBUG_PRINTLN("Parity check OK");
            DEBUG_PRINT(" Hour1: "); DEBUG_PRINTLN((unsigned long)rx_buffer->Hour);
            DEBUG_PRINT(" Hour2: "); DEBUG_PRINTLN((unsigned long)(rx_buffer->Hour/16)*6);

            // convert the received bits from BCD
            dst   = rx_buffer->DST;
            mm    = rx_buffer->Min     - ((rx_buffer->Min     / 16) * 6);
            hh    = rx_buffer->Hour    - ((rx_buffer->Hour    / 16) * 6);
            day   = rx_buffer->Day     - ((rx_buffer->Day     / 16) * 6);
            wkd   = rx_buffer->Weekday;
            mon   = rx_buffer->Month   - ((rx_buffer->Month   / 16) * 6);
            year  = rx_buffer->Year    - ((rx_buffer->Year    / 16) * 6) + 2000;
            ready = true;
        }
        // else {
            // DEBUG_PRINTLN(">>> Parity check failed <<<");
        // }
    }

    // reset stuff
    ss             = 0;
    bufferPosition = 0;
    dcf_rx_buffer  = 0;
    parity.parity_flag = parity.parity_min = parity.parity_hour = parity.parity_date = 0;
}




void DCF77::serialDumpTime(void) {
    DEBUG_PRINT("\nTime: ");
    DEBUG_PRINT(hh, DEC);
    DEBUG_PRINT(":");
    DEBUG_PRINT(mm, DEC);
    DEBUG_PRINT(":");
    DEBUG_PRINT(ss, DEC);
    DEBUG_PRINT(" Date: ");

    switch (wkd) {
        case 1:
            DEBUG_PRINT("Mo, ");
            break;
        case 2:
            DEBUG_PRINT("Tu, ");
            break;
        case 3:
            DEBUG_PRINT("We, ");
            break;
        case 4:
            DEBUG_PRINT("Th, ");
            break;
        case 5:
            DEBUG_PRINT("Fr, ");
            break;
        case 6:
            DEBUG_PRINT("Sa, ");
            break;
        case 7:
            DEBUG_PRINT("Su, ");
            break;
        default:
            DEBUG_PRINT("--, ");
    }
    DEBUG_PRINT(day, DEC);
    DEBUG_PRINT(".");
    DEBUG_PRINT(mon, DEC);
    DEBUG_PRINT(".");
    DEBUG_PRINT(year, DEC);
    switch (dst) {
        case B01:
            DEBUG_PRINTLN(" (Sommerzeit)");
            break;
        case B10:
            DEBUG_PRINTLN(" (Winterzeit)");
            break;
        default:
            DEBUG_PRINT(" (");
            DEBUG_PRINT(dst, DEC);
            DEBUG_PRINTLN(")");
    }
}

/**
 * ########################################################
 * ###                 Helpers below                    ###
 * ########################################################
 */

/**
 * Add one second to the hh:mm:ss variables.
 */
void DCF77::addSecond() {
    ss++;
    if (ss == 60) {
        ss = 0;
        mm++;
        if (mm == 60) {
            mm = 0;
            hh++;
            if (hh == 24)
                hh = 0;
        }
    }
}
