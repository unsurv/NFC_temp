/* RF430CL - TI RF430CL330H Dynamic NFC Transponder library using Energia I2C (Wire)
 *
 * Copyright (c) 2015 Eric Brundick <spirilis [at] linux dot com>
 *  Permission is hereby granted, free of charge, to any person 
 *  obtaining a copy of this software and associated documentation 
 *  files (the "Software"), to deal in the Software without 
 *  restriction, including without limitation the rights to use, copy, 
 *  modify, merge, publish, distribute, sublicense, and/or sell copies 
 *  of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be 
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 *  DEALINGS IN THE SOFTWARE.
 */

#ifndef RF430CL_H
#define RF430CL_H

#include <Arduino.h>
#include <Wire.h>
#include <Stream.h>
#include <Print.h>
#include "NDEF.h"

// For non-MSP430 chips
#ifndef BITF
#define BIT0 0x0001
#define BIT1 0x0002
#define BIT2 0x0004
#define BIT3 0x0008
#define BIT4 0x0010
#define BIT5 0x0020
#define BIT6 0x0040
#define BIT7 0x0080
#define BIT8 0x0100
#define BIT9 0x0200
#define BITA 0x0400
#define BITB 0x0800
#define BITC 0x1000
#define BITD 0x2000
#define BITE 0x4000
#define BITF 0x8000
#endif

/* Internal registers */
// CONTROL register bits
#define RF430_REG_CONTROL 0xFFFE
#define RF430_CONTROL_SW_RESET BIT0
#define RF430_CONTROL_ENABLE_RF BIT1
#define RF430_CONTROL_ENABLE_INT BIT2
#define RF430_CONTROL_INTO_HIGH BIT3
#define RF430_CONTROL_INTO_DRIVE BIT4
#define RF430_CONTROL_BIP8 BIT5
#define RF430_CONTROL_STANDBY_ENABLE BIT6
#define RF430_CONTROL_TEST430 BIT7  // Reserved

// STATUS register bits
#define RF430_REG_STATUS 0xFFFC
#define RF430_STATUS_READY BIT0
#define RF430_STATUS_CRC_ACTIVE BIT1
#define RF430_STATUS_RF_BUSY BIT2

// Interrupt Enable & Interrupt Flag registers
#define RF430_REG_INT_ENABLE 0xFFFA
#define RF430_REG_INT_FLAG 0xFFF8
#define RF430_INT_END_OF_READ BIT1
#define RF430_INT_END_OF_WRITE BIT2
#define RF430_INT_CRC_COMPLETED BIT3
#define RF430_INT_BIP8_ERROR BIT4
#define RF430_INT_NDEF_ERROR BIT5
#define RF430_INT_GENERIC_ERROR BIT7

// Communication Watchdog register bits
#define RF430_REG_WATCHDOG 0xFFF0
#define RF430_WATCHDOG_ENABLE BIT0
#define RF430_WATCHDOG_TIMEOUT_PERIOD BIT1  // 3-bit value shifted BIT1 to the left

// Version register
#define RF430_REG_VERSION 0xFFEE

// CRC registers
#define RF430_REG_CRC_RESULT 0xFFF6
#define RF430_REG_CRC_LENGTH 0xFFF4  // Calculation starts when CRC_LENGTH register is written!
#define RF430_REG_CRC_START_ADDRESS 0xFFF2

// Offset where NDEF data starts in the SRAM
#define RF430_SRAMFS_NDEF_START 0x001C
#define RF430_SRAMFS_NDEF_LENGTH_WORD 0x001A

/* I/O access class & code */

class RF430 : public Stream {
    private:
        boolean is_rf_active;
        int resetPin, irqPin;
        uint16_t data_ptr;
        uint8_t i2caddr;
        uint16_t last_known_irq, end_of_ndef;
        TwoWire *i2cbus;

    public:
        RF430(int resetPin_, int irqPin_, uint8_t i2caddr_);
        RF430(int resetPin_, int irqPin_);
        void setWire(TwoWire *wire_instance_ptr) { i2cbus = wire_instance_ptr; }  // Change instance of TwoWire used by the library

        void begin(void);
        void end(void);

        void enable(void);
        void disable(void);

        // Internal I/O
        void writeReg(uint16_t addr, uint16_t val);
        void writeSRAM(uint16_t start, const uint8_t *, size_t len); // Write to true start of SRAM (0x0000)
        void format(void);

        size_t write(uint8_t);
        size_t write(const uint8_t *, size_t);
        void setDataPointer(uint16_t addr) { data_ptr = addr + RF430_SRAMFS_NDEF_START; };  // Address positioned relative to start of NDEF file
        void setDataPointerReal(uint16_t addr) { data_ptr = addr; };  // Address positioned to real SRAM location
        void setDataLength(uint16_t len);  // Write the 2 bytes (big-endian 16 bit) just before NDEF file
        uint16_t getDataPointer(void) { return data_ptr - RF430_SRAMFS_NDEF_START; };  // Position relative to start of NDEF file
        uint16_t getDataLength(void);

        uint16_t readReg(uint16_t addr);
        void readSRAM(uint16_t start, uint8_t *, size_t len);  // Read from true start of SRAM (0x0000)
        size_t readBytes(char *buf, size_t len);
        int peek();  // Read current character w/o advancing data_ptr
        int read();  // Return peek() while advancing data_ptr and clearing available() if relevant
        void flush() { last_known_irq &= ~RF430_INT_END_OF_WRITE; setDataPointer(0); };  // Make available() return false.
        int available() { if (last_known_irq & RF430_INT_END_OF_WRITE) { return true; }; return false; };

        // IRQ handling
        int loop(boolean use_irq_line=true);  // Check and clear IRQs; use no argument unless you know what you're doing
        int wasRead();
        int isError() { if (last_known_irq & RF430_INT_NDEF_ERROR) { last_known_irq &= ~RF430_INT_NDEF_ERROR; return true; }; return false; };
};

#endif /* RF430CL_H */
