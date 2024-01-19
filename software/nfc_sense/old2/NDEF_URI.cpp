/* NDEF_URI - Handle URI NFC NDEF records
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

#include "NDEF_URI.h"

const uint8_t ndef_uri_record_type = 'U';

const char *ndef_uri_prefixes[] = {
    "",
    "http://www.", // 11
    "https://www.", // 12
    "http://", // 7
    "https://", // 8
    "tel:", // 4
    "mailto:", // 7
    "ftp://anonymous:anonymous@", // 26
    "ftp://ftp.", // 10
    "ftps://", // 7
    "sftp://", // 7
    "smb://", // 6
    "nfs://", // 6
    "ftp://", // 6
    "dav://", // 6
    "news:", // 5
    "telnet://", // 9
    "imap:", // 5
    "rtsp://", // 7
    "urn:", // 4
    "pop:", // 4
    "sip:", // 4
    "sips:", // 5
    "tftp:", // 5
    "btspp://", // 8
    "btl2cap://", // 10
    "btgoep://", // 9
    "tcpobex://", // 10
    "irdaobex://", // 11
    "file://", // 7
    "urn:epc:id:", // 11
    "urn:epc:tag:", // 12
    "urn:epc:pat:", // 12
    "urn:epc:raw:", // 12
    "urn:epc:", // 8
    "urn:nfc:" // 8
};

const uint8_t ndef_uri_prefixes_strlen[] = {
    0, 11, 12, 7, 8, 4, 7, 26, 10, 7, 7, 6, 6, 6, 6, 5, 9, 5, 7, 4, 4, 4, 5, 5, 8, 10, 9, 10, 11, 7, 11, 12, 12, 12, 8, 8
};

NDEF_URI::NDEF_URI()
{
    tnf = NDEF_TNF_WELLKNOWN;
    type_length = 1;
    type = (char *)&ndef_uri_record_type;
    id_length = 0;
    id = NULL;
    payload_length = 0;
    payload_buf_maxlen = 0;
    payload = NULL;
    prefix = 0x00;
}

NDEF_URI::NDEF_URI(const char *uri)
{
    tnf = NDEF_TNF_WELLKNOWN;
    type_length = 1;
    type = (char *)&ndef_uri_record_type;
    id_length = 0;
    id = NULL;
    prefix = compressPrefix(uri);
    size_t plen = ndef_uri_prefixes_strlen[prefix];
    payload_length = strlen(uri) - plen;
    payload = (uint8_t *)uri + plen;
    payload_buf_maxlen = 0;  // not used here
}

uint8_t NDEF_URI::compressPrefix(const char *uri)
{
    int i;
    size_t ulen, clen;
    uint8_t pfx = 0x00;

    if (uri == NULL || uri[0] == '\0')
        return 0x00;
    ulen = strlen(uri);

    for (i=1; i < 0x36; i++) {
        clen = ndef_uri_prefixes_strlen[i];
        if (clen > ulen)
            clen = ulen;

        if (!strncmp(uri, ndef_uri_prefixes[i], clen)) {
            pfx = i;
            break;
        }
    }
    if (pfx == 0x00)
        return 0x00;

    return pfx;
}

const char * NDEF_URI::decompressPrefix(const uint8_t pfx)
{
    if (pfx < 0x01 || pfx > 0x35)
        return "";
    return ndef_uri_prefixes[pfx];
}

int NDEF_URI::setURI(const char *uri)
{
    if (uri == NULL || uri[0] == '\0')
        return 0;

    prefix = compressPrefix(uri);
    size_t plen = ndef_uri_prefixes_strlen[prefix];
    payload_length = strlen(uri) - plen;
    payload = (uint8_t *)uri + plen;
    payload_buf_maxlen = 0;  // not used here

    return payload_length;
}

int NDEF_URI::storeURI(char *buf, size_t maxlen)
{
    int total_size = 0;

    if (buf == NULL || maxlen < 1)
        return -1;
    if (maxlen < ndef_uri_prefixes_strlen[prefix])
        return -1;

    strcpy(buf, ndef_uri_prefixes[prefix]);
    total_size = ndef_uri_prefixes_strlen[prefix];

    if (maxlen < payload_length) {
        strncat(buf, (char *)payload, maxlen-1);
        total_size += maxlen-1;
    } else {
        strncat(buf, (char *)payload, payload_length);
        total_size += payload_length;
    }

    return total_size;
}

int NDEF_URI::printURI(Print &p)
{
    p.write((const uint8_t *)ndef_uri_prefixes[prefix], ndef_uri_prefixes_strlen[prefix]);
    p.write((const uint8_t *)payload, payload_length);
}

int NDEF_URI::sendTo(Print &p, boolean first_msg, boolean last_msg)
{
    uint32_t real_plen = payload_length + 1;
    int printedSize = 0;
    uint8_t msghdr;

    // Output valid NDEF binary format
    msghdr = 0x00;
    if (first_msg)
        msghdr |= NDEF_FIELD_MB;
    if (last_msg)
        msghdr |= NDEF_FIELD_ME;

    if (real_plen > 254) {
        p.write(tnf | msghdr);  // SR cleared, using 32-bit Payload Length
    } else {
        p.write(tnf | msghdr | NDEF_FIELD_SR);
    }
    p.write(1);  // Length of TYPE field
    printedSize = 2;

    if (real_plen > 254) {
        // PAYLOAD_LENGTH is a 32-bit Big-Endian unsigned integer
        p.write((uint8_t) (real_plen >> 24));
        p.write((uint8_t) ((real_plen >> 16) & 0xFF));
        p.write((uint8_t) ((real_plen >> 8) & 0xFF));
        p.write((uint8_t) (real_plen & 0xFF));
        printedSize += 4;
    } else {
        // PAYLOAD_LENGTH is an 8-bit unsigned integer
        p.write(real_plen);
        printedSize++;
    }
    p.write((uint8_t) type[0]);  // TYPE
    // PAYLOAD
    p.write(prefix);
    printedSize += 2;
    p.write((const uint8_t *)payload, payload_length);
    printedSize += payload_length;
    // NOTE: printedSize may be a smaller variable than payload_length

    return printedSize;
}

int NDEF_URI::import(Stream &s)
{
    int c, ndef_hdr;
    size_t plen, plen_write;
    uint8_t plen32[4];

    // Requires setPayloadBuffer() to have been used previously
    if (payload == NULL || payload_buf_maxlen < 1)
        return -1;  // Not going to bother attempting to read

    // read NDEF header byte
    c = s.read();       if (c < 0) return -1;
    if ( (c & NDEF_FIELD_IL) || ((c & 0x03) != NDEF_TNF_WELLKNOWN) )
        return -1;  // Not a URI RTD...
    ndef_hdr = c;

    // read TYPE_LENGTH
    c = s.read();       if (c < 0) return -1;
    if (c != 0x01)
        return -1;  // Not a URI RTD, as the TYPE field should be a single character

    // read PAYLOAD_LENGTH
    if (ndef_hdr & NDEF_FIELD_SR) {
        c = s.read();       if (c < 0) return -1;
        plen = c;
    } else {
        if (s.readBytes((char *)&plen32[0], 4) < 4)
            return -1;
        plen = ((uint32_t) plen32[0]) << 24 |
               ((uint32_t) plen32[1]) << 16 |
               ((uint32_t) plen32[2]) << 8 |
               ((uint32_t) plen32[3]);
    }
    payload_length = plen - 1;  // Length of non-abbreviation portion
    
    // read TYPE
    c = s.read();       if (c < 0) return -1;
    if (c != type[0])
        return -1;

    // read PAYLOAD
    c = s.read();       if (c < 0) return -1;
    if (c < 0x01 || c > 0x35)
        return -1;  // Invalid abbreviation byte
    prefix = c;
    plen--;

    plen_write = plen;  // Max # of bytes writable to payload[] before we just read & discard
    if (payload_buf_maxlen < plen_write)
        plen_write = payload_buf_maxlen;

    if (s.readBytes((char *)payload, plen_write) < plen_write)
        return -1;

    // Drain remaining bytes that wouldn't fit into payload[] buffer
    plen -= plen_write;
    while (plen && c != -1) {
        c = s.read();
        plen--;
    }

    // All done!
    return plen_write;
}
