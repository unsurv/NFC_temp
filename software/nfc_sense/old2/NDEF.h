/* NDEF - Abstract class for defining NFC NDEF RTD-specific classes that allow
 * a user to compose or read information over NFC in an intuitive, programmatic
 * manner.
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

#ifndef NDEF_H
#define NDEF_H

#include <Arduino.h>
#include <Print.h>

/* NDEF fields */
#define NDEF_FIELD_IL 0x08       // ID Length Present
#define NDEF_FIELD_SR 0x10       // Short Record (1-byte payload length vs. 4-byte)
#define NDEF_FIELD_CF 0x20       // Chunk Flag (chunked NDEF, more to come)
#define NDEF_FIELD_ME 0x40       // Message End
#define NDEF_FIELD_MB 0x80       // Message Begin

/* TNF fields (lower 3 bits of NDEF header) */
#define NDEF_TNF_EMPTY 0x00
#define NDEF_TNF_WELLKNOWN 0x01
#define NDEF_TNF_MEDIA 0x02
#define NDEF_TNF_ABSOLUTE_URI 0x03
#define NDEF_TNF_EXTERNAL 0x04
#define NDEF_TNF_UNKNOWN 0x05
#define NDEF_TNF_UNCHANGED 0x06

class NDEF : public Print {
    protected:
        uint8_t tnf;
        unsigned int type_length, id_length;
        size_t payload_length;
        uint8_t *payload;
        size_t payload_buf_maxlen, type_buf_maxlen, id_buf_maxlen;
        char *type;
        char *id;

    public:
        NDEF();
        virtual void setTypeBuffer(void *typebuf, size_t maxlen) { type = (char *)typebuf; type_buf_maxlen = maxlen; };
        virtual void setIDBuffer(void *idbuf, size_t maxlen) { id = (char *)idbuf; id_buf_maxlen = maxlen; };
        virtual void setType(const char *type_) { type = (char *)type_; type_length = strlen(type_); type_buf_maxlen = 0; };
        virtual char * getType(void) { return type; };
        virtual void setID(const char *id_) { id = (char *)id_; id_length = strlen(id_); id_buf_maxlen = 0; };
        virtual char * getID(void) { return id; };
	virtual uint8_t getTNF(void) { return tnf; };

        // Basic payload buffers support generic NDEF import/export
        virtual void setPayloadBuffer(void *buf, size_t maxlen) { payload = (uint8_t *)buf; payload_buf_maxlen = maxlen; payload_length = 0; };
        virtual const uint8_t * getPayload(void) { return (const uint8_t *)payload; };
        virtual size_t getPayloadLength(void) { return payload_length; };

        // Default implementations of these exist, but subclasses are encouraged to override if it's convenient.
        // This is especially true when the payload contents need to be interpreted or segregated into different buffers.
        virtual int sendTo(Print &p, boolean first_msg = true, boolean last_msg = true);
        virtual int import(Stream &s);

        // To be overridden by subclasses if applicable
        virtual size_t write(const uint8_t *buf, size_t len) { return 0; };
        virtual size_t write(uint8_t) { return 0; };
};

#endif /* NDEF_H */
