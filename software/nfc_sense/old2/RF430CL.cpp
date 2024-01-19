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

#include "RF430CL.h"

RF430::RF430(int resetPin_, int irqPin_, uint8_t i2caddr_)
{
    resetPin = resetPin_;
    irqPin = irqPin_;
    i2caddr = i2caddr_;
    is_rf_active = false;
    data_ptr = 0;
    i2cbus = &Wire;
}

RF430::RF430(int resetPin_, int irqPin_)
{
    resetPin = resetPin_;
    irqPin = irqPin_;
    i2caddr = 0x28;  // Default with all I2C address pins pulled low
    is_rf_active = false;
    data_ptr = 0;
    i2cbus = &Wire;
}

void RF430::begin()
{
    uint16_t reg;

    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);  // Reset
    delay(10);
    digitalWrite(resetPin, HIGH); // Come out of reset
    delay(20);

    pinMode(irqPin, INPUT_PULLUP);

    // Wait until READY
    do {
        reg = readReg(RF430_REG_STATUS);

        Serial.print("REG STATUS");
        Serial.print(reg);
        
    } while ( !(reg & RF430_STATUS_READY) );



    // Write NDEF filesystem/application data
    format();

    // Configure CONTROL, Interrupts
    writeReg(RF430_REG_INT_ENABLE, RF430_INT_END_OF_READ | RF430_INT_END_OF_WRITE | RF430_INT_CRC_COMPLETED | RF430_INT_NDEF_ERROR | RF430_INT_GENERIC_ERROR);
    writeReg(RF430_REG_CONTROL, RF430_CONTROL_STANDBY_ENABLE | RF430_CONTROL_ENABLE_INT);

    // RF_Lock errata initialization
    reg = readReg(RF430_REG_VERSION);
    Serial.print("REG VERSION");
    Serial.print(reg);
    if (reg == 0x0101 || reg == 0x0201) {  // Silicon revisions C and D
        writeReg(0xFFE0, 0x004E);  // TEST mode key
        writeReg(0xFFFE, 0x0080);  // TEST mode
        // Rewrite some internal parameters
        if (reg == 0x0101)
            writeReg(0x2A98, 0x0650);
        else
            writeReg(0x2A6E, 0x0650);
        writeReg(0x2814, 0x0000);
        writeReg(0xFFE0, 0x0000);  // Exit TEST mode
    }

    data_ptr = RF430_SRAMFS_NDEF_START;
}

void RF430::end()
{
    writeReg(RF430_REG_CONTROL, RF430_CONTROL_SW_RESET);
    pinMode(resetPin, INPUT_PULLUP);
    is_rf_active = false;
    data_ptr = 0;
}

void RF430::enable()
{
    writeReg(RF430_REG_CONTROL, RF430_CONTROL_STANDBY_ENABLE | RF430_CONTROL_ENABLE_INT | RF430_CONTROL_ENABLE_RF);
    is_rf_active = true;
}

void RF430::disable()
{
    uint16_t r = readReg(RF430_REG_STATUS);

    while ( !(r & RF430_STATUS_READY) ) {  // Wait for in-progress RF communication to stop
        Serial.print('.');
        delay(20);
        r = readReg(RF430_REG_STATUS);
    }

    writeReg(RF430_REG_CONTROL, RF430_CONTROL_STANDBY_ENABLE | RF430_CONTROL_ENABLE_INT);  // Turn off ENABLE_RF bit
    is_rf_active = false;
}

void RF430::writeReg(uint16_t addr, uint16_t val)
{
    i2cbus->beginTransmission(i2caddr);
    i2cbus->write(addr >> 8);
    i2cbus->write(addr & 0xFF);
    i2cbus->write(val & 0xFF);
    i2cbus->write(val >> 8);
    i2cbus->endTransmission();
}

uint16_t RF430::readReg(uint16_t addr)
{
    uint16_t ret;

    i2cbus->beginTransmission(i2caddr);
    i2cbus->write(addr >> 8);
    i2cbus->write(addr & 0xFF);
    i2cbus->endTransmission();
    i2cbus->requestFrom(i2caddr, (uint8_t)2);
    if (i2cbus->available())
        ret = i2cbus->read();
    if (i2cbus->available())
        ret |= i2cbus->read() << 8;
    return ret;
}

void RF430::writeSRAM(uint16_t start, const uint8_t *buf, size_t len)
{
    data_ptr = start;
    write(buf, len);
}

size_t RF430::write(const uint8_t *buf, size_t len)
{
    size_t written = 0;
    int i;

    i2cbus->beginTransmission(i2caddr);
    i2cbus->write(data_ptr >> 8);
    i2cbus->write(data_ptr & 0xFF);

    while ( (len-written) > 8 ) {
        for (i=0; i < 8; i++) {
            i2cbus->write(buf[written++]);
            data_ptr++;
        }
        i2cbus->endTransmission();
        i2cbus->beginTransmission(i2caddr);
        i2cbus->write(data_ptr >> 8);
        i2cbus->write(data_ptr & 0xFF);
    }

    do {
        i2cbus->write(buf[written++]);
        data_ptr++;
    } while (len - written);
    i2cbus->endTransmission();

    return written;
}

size_t RF430::write(uint8_t c)
{
    i2cbus->beginTransmission(i2caddr);
    i2cbus->write(data_ptr >> 8);
    i2cbus->write(data_ptr & 0xFF);
    i2cbus->write(c);
    i2cbus->endTransmission();
    data_ptr++;

    return 1;
}

size_t RF430::readBytes(char *buf, size_t len)
{
    size_t haveread = 0;

    i2cbus->beginTransmission(i2caddr);
    i2cbus->write(data_ptr >> 8);
    i2cbus->write(data_ptr & 0xFF);
    i2cbus->endTransmission();

    while ( (len - haveread) > 8 ) {
        i2cbus->requestFrom(i2caddr, (uint8_t)8);
        while (i2cbus->available()) {
            buf[haveread++] = i2cbus->read();
            data_ptr++;
        }
    }

    i2cbus->requestFrom(i2caddr, (uint8_t) (len-haveread));
    while (i2cbus->available()) {
        buf[haveread++] = i2cbus->read();
        data_ptr++;
    }

    if (data_ptr >= end_of_ndef)
        last_known_irq &= ~RF430_INT_END_OF_WRITE;

    return haveread;
}

int RF430::peek()
{
    int c;

    i2cbus->beginTransmission(i2caddr);
    i2cbus->write(data_ptr >> 8);
    i2cbus->write(data_ptr & 0xFF);
    i2cbus->endTransmission();

    i2cbus->requestFrom(i2caddr, (uint8_t)1);
    if (i2cbus->available())
        c = i2cbus->read();

    return c;
}

int RF430::read()
{
    int c = peek();

    data_ptr++;
    if (data_ptr >= end_of_ndef)
        last_known_irq &= ~RF430_INT_END_OF_WRITE;

    return c;
}

void RF430::readSRAM(uint16_t start, uint8_t *buf, size_t len)
{
    data_ptr = start;
    readBytes((char *)buf, len);
}

// Write the RF430CL330H SRAM NDEF application filesystem
const uint8_t ndef_sram_fs_format[] = {
    0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01,               // NDEF Application
    0xE1, 0x03,                                             // Capability Container - File ID E103
    0x00, 0x0F,                                             // CCLen = 15 (length includes CCLen bytes)
    0x20,                                                   // Mapping version 2.0
    0x00, 0xF9, 0x00, 0xF6,                                 // Maximum R-APDU, C-APDU sizes
    0x04, 0x06,                                             // Tag - NDEF Control TLV
    0xE1, 0x04,                                             // File identifier for NDEF file
    0x0B, 0xDF,                                             // File size = 3037 bytes available
    0x00, 0x00,                                             // Read & Write access with no security
    0xE1, 0x04,                                             // Beginning of NDEF file
    // First 2 bytes are big-endian specifier of NDEF file size
};

const uint8_t ndef_default_blank[] = {
    NDEF_FIELD_MB | NDEF_FIELD_ME | NDEF_FIELD_SR | NDEF_TNF_EMPTY,
    0x00, 0x00  // No type field, no payload
};

void RF430::format(void)
{
    writeSRAM(0x0000, ndef_sram_fs_format, sizeof(ndef_sram_fs_format));
    writeSRAM(RF430_SRAMFS_NDEF_START, ndef_default_blank, 3);
    setDataLength(3);
}

void RF430::setDataLength(uint16_t len)
{
    uint8_t buf[2];

    // Since this is an NFC Forum standardized field, it's encoded Big-Endian.
    buf[0] = len >> 8;
    buf[1] = len & 0xFF;
    writeSRAM(RF430_SRAMFS_NDEF_LENGTH_WORD, buf, 2);
    end_of_ndef = RF430_SRAMFS_NDEF_START + len;
}

uint16_t RF430::getDataLength(void)
{
    uint8_t buf[2];

    readSRAM(RF430_SRAMFS_NDEF_LENGTH_WORD, buf, 2);
    return ( (buf[0] << 8) | buf[1] );
}

int RF430::loop(boolean use_irq_line)
{
    // Check IRQs
    uint8_t irq;

    if (use_irq_line && digitalRead(irqPin) == HIGH)
        return false;

    if (is_rf_active)
        disable();  // Disable RF before reading from device

    irq = readReg(RF430_REG_INT_FLAG);
    last_known_irq |= irq;

    if (irq & RF430_INT_END_OF_WRITE) {
        uint8_t bytes[2];
        readSRAM(RF430_SRAMFS_NDEF_LENGTH_WORD, bytes, 2);
        end_of_ndef = RF430_SRAMFS_NDEF_START;
        end_of_ndef += (bytes[0] << 8) | bytes[1];
        data_ptr = RF430_SRAMFS_NDEF_START;  // Read new NDEF size, reset pointer to 0
    }

    if (irq) {
        // Quirk I've found; IRQ line won't clear unless INT_ENABLE is disabled before clearing IRQs.
        // The RF430 example code from TI includes this without much explanation.
        writeReg(RF430_REG_INT_ENABLE, 0x0000);
        writeReg(RF430_REG_INT_FLAG, irq);  // Clear IRQs
        writeReg(RF430_REG_INT_ENABLE, RF430_INT_END_OF_READ | RF430_INT_END_OF_WRITE | RF430_INT_CRC_COMPLETED | RF430_INT_NDEF_ERROR | RF430_INT_GENERIC_ERROR);
        return true;
    }
    return false;
}

// Check if RF430_INT_END_OF_READ has recently been discovered, then clear it from our internal variable.
int RF430::wasRead(void)
{
    if (last_known_irq & RF430_INT_END_OF_READ) {
        last_known_irq &= ~RF430_INT_END_OF_READ;
        return true;
    }

    return false;
}
