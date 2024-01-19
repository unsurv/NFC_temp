/* NDEF - Handle generic NDEF records
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

#include "NDEF.h"

NDEF::NDEF()
{
    tnf = 0x00;
    payload = NULL;
    type = NULL;
    id = NULL;

    payload_length = 0;
    payload_buf_maxlen = 0;
    type_length = 0;
    id_length = 0;
    type_buf_maxlen = 0;
    id_buf_maxlen = 0;
}

// Default implementation of sendTo()
int NDEF::sendTo(Print &p, boolean first_msg, boolean last_msg)
{
    uint8_t msghdr;
    int printedSize = 0;

    // Sanity checking
    if (type_length && type == NULL)
        return -1;
    if (id_length && id == NULL)
        return -1;
    if (payload_length && payload == NULL)
        return -1;

    // Output valid NDEF binary format
    msghdr = 0x00;
    if (first_msg)
        msghdr |= NDEF_FIELD_MB;
    if (last_msg)
        msghdr |= NDEF_FIELD_ME;

    if (id_length)
        msghdr |= NDEF_FIELD_IL;

    if (payload_length > 254) {
        p.write(tnf | msghdr);  // SR cleared, using 32-bit Payload Length
    } else {
        p.write(tnf | msghdr | NDEF_FIELD_SR);
    }
    p.write((uint8_t)type_length);  // Length of TYPE field
    printedSize = 2;

    if (payload_length > 254) {
        // PAYLOAD_LENGTH is a 32-bit Big-Endian unsigned integer
        p.write((uint8_t) (payload_length >> 24));
        p.write((uint8_t) ((payload_length >> 16) & 0xFF));
        p.write((uint8_t) ((payload_length >> 8) & 0xFF));
        p.write((uint8_t) (payload_length & 0xFF));
        printedSize += 4;
    } else {
        // PAYLOAD_LENGTH is an 8-bit unsigned integer
        p.write(payload_length);
        printedSize++;
    }

    // ID length
    if (id_length) {
        p.write((uint8_t) id_length);
        printedSize++;
    }

    /* For TYPE and ID, and PAYLOAD for that matter, keep in mind if their length = 0, p.write() won't do anything.
     * So no need to gate these statements with if (type_length) { ... } or if (id_length) { ... } or
     * if (payload_length) { ... }
     */

    // TYPE
    p.write((const uint8_t *)type, type_length);
    printedSize += type_length;

    // ID
    p.write((const uint8_t *)id, id_length);
    printedSize += id_length;

    // PAYLOAD
    p.write((const uint8_t *)payload, payload_length);
    printedSize += payload_length;

    // NOTE: printedSize may be a smaller variable than payload_length

    return printedSize;
}

int NDEF::import(Stream &s)
{
    int c, ndef_hdr;
    size_t rlen = 0;  // Total NDEF record size
    uint8_t plen32[4];
    int i;

    // Requires payload to be a valid buffer (we assume it's writable)
    if (payload == NULL || type == NULL || !type_buf_maxlen)
        return -1;  // Not going to bother attempting to read

    type_length = 0;
    id_length = 0;
    payload_length = 0;

    // read NDEF header byte
    c = s.read();       if (c < 0) return -1;
    ndef_hdr = c;
    rlen++;

    // Extract TNF
    tnf = ndef_hdr & 0x07;

    // read TYPE_LENGTH
    c = s.read();       if (c < 0) return -1;
    type_length = c;
    rlen++;

    // read PAYLOAD_LENGTH
    if (ndef_hdr & NDEF_FIELD_SR) {
        c = s.read();       if (c < 0) return -1;
        payload_length = c;
        rlen++;
    } else {
        if (s.readBytes((char *)&plen32[0], 4) < 4)
            return -1;
        payload_length = ((uint32_t) plen32[0]) << 24 |
                         ((uint32_t) plen32[1]) << 16 |
                         ((uint32_t) plen32[2]) << 8 |
                         ((uint32_t) plen32[3]);
        rlen += 4;
    }

    // Was the ID bit set?  If so, read ID length.
    if (ndef_hdr & NDEF_FIELD_IL) {
        c = s.read();      if (c < 0) return -1;
        id_length = c;
        // Is id_length > 0?  If so, be sure we have a buffer for 'id'
        if (id_length && id == NULL)
            return -1;  // No buffer for holding the ID!
        rlen++;
    } else {
        if (id_buf_maxlen && id != NULL) {
            // Clear the ID field since there will be none here
            id_length = 0;
            id[id_length] = '\0';
        }
    }
    
    // read TYPE
    rlen += type_length;
    if (type_length >= type_buf_maxlen) {
        size_t t = type_buf_maxlen - 1;
        if (s.readBytes(type, t) < t)
            return -1;
        // Drain remaining bytes from the TYPE field
        while (type_length > type_buf_maxlen) {
            if (s.read() < 0) return -1;
            type_length--;
        }
    } else {
        if (s.readBytes(type, type_length) < type_length)
            return -1;
    }
    type[type_length] = '\0';  // Type is always meant to be a string

    // read ID, if applicable
    if (ndef_hdr & NDEF_FIELD_IL) {
        rlen += id_length;
        if (id_length >= id_buf_maxlen) {
            size_t ti = id_buf_maxlen - 1;  // accounting for the '\0' at the end
            if (s.readBytes(id, ti) < ti)
                return -1;
            // Drain remaining bytes from the ID field
            while (id_length > id_buf_maxlen) {
                if (s.read() < 0) return -1;
                id_length--;
            }
        } else {
            if (s.readBytes(id, id_length) < id_length)
             return -1;
        }
        id[id_length] = '\0';  // ID is always meant to be a string
    }

    // read PAYLOAD
    if (s.readBytes((char *)payload, payload_length) < payload_length)
        return -1;
    rlen += payload_length;

    // All done!
    return rlen;  // Total size of NDEF record
}
