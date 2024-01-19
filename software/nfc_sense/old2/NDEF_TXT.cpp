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

#include "NDEF_TXT.h"

const uint8_t ndef_txt_record_type = 'T';

NDEF_TXT::NDEF_TXT()
{
    tnf = NDEF_TNF_WELLKNOWN;
    type_length = 1;
    type = (char *)&ndef_txt_record_type;
    id_length = 0;
    id = NULL;
    payload_length = 0;
    payload_buf_maxlen = 0;
    payload = NULL;

    // English by default
    strcpy(lang, "en");
    lang_length = 2;

    is_utf16 = false;  // UTF-8 by default
}

NDEF_TXT::NDEF_TXT(const char *lang_)
{
    tnf = NDEF_TNF_WELLKNOWN;
    type_length = 1;
    type = (char *)&ndef_txt_record_type;
    id_length = 0;
    id = NULL;
    payload_length = 0;
    payload_buf_maxlen = 0;
    payload = NULL;

    lang_length = strlen(lang_);
    if (lang_length > 8) {  // Invalid for this library; use "en" per defaults.
        strcpy(lang, "en");
        lang_length = 2;
    } else {
        strncpy(lang, lang_, lang_length);
    }

    is_utf16 = false;  // UTF-8 by default
}

NDEF_TXT::NDEF_TXT(const char *lang_, const char *text_, boolean utf16)
{
    tnf = NDEF_TNF_WELLKNOWN;
    type_length = 1;
    type = (char *)&ndef_txt_record_type;
    id_length = 0;
    id = NULL;
    payload = (uint8_t *)text_;
    payload_length = strlen(text_);
    payload_buf_maxlen = 0;

    lang_length = strlen(lang_);
    if (lang_length > 8) {  // Invalid for this library; use "en" per defaults.
        strcpy(lang, "en");
        lang_length = 2;
    } else {
        strncpy(lang, lang_, lang_length);
    }

    is_utf16 = utf16;
}

void NDEF_TXT::setLanguage(const char *lang_)
{
    lang_length = strlen(lang_);

    if (lang_length > 8) {  // Invalid for this library; bail without changing
        return;
    } else {
        strncpy(lang, lang_, lang_length);
    }
}

/* Perform a semi-intelligent test comparing the intended language "l" with
 * the actual language specifier to see if it's a subset; e.g. for lang[] = "en-US",
 * searching for l[] = "en" should suffice.
 */
boolean NDEF_TXT::testLanguage(const char *l)
{
    int i;
    char langlow[9], llow[9];

    if (l == NULL)
        return false;
    size_t llen = strlen(l);
    if (!llen)
        return false;

    if (llen > lang_length)
        return false;

    // Copy strings into temporary buffers with lowercase
    for (i=0; i < lang_length; i++) {
        langlow[i] = lang[i];
        if (langlow[i] >= 'A' && langlow[i] <= 'Z')
            langlow[i] += 32;
    }
    langlow[i] = '\0';
    for (i=0; i < llen; i++) {
        llow[i] = l[i];
        if (llow[i] >= 'A' && llow[i] <= 'Z')
            llow[i] += 32;
    }
    llow[i] = '\0';

    // Compare
    if (llen == lang_length) {
        if (!strncmp(llow, langlow, llen))
            return true;
        return false;
    }

    if (strchr(llow, '-') == NULL && langlow[llen] == '-' && !strncmp(llow, langlow, llen))  // Search is specifying a subset, e.g. "en" out of "en-US"
        return true;

    return false;
}

int NDEF_TXT::setText(const char *text)
{
    if (!payload_buf_maxlen) {
        // No payload buffer exists, so instead we must be intending to replace the payload entirely
        payload = (uint8_t *)text;
        payload_length = strlen(text);
        return payload_length;
    }

    // Payload buffer exists; stuff it with as much as can fit
    size_t tlen = strlen(text);
    if (tlen > payload_buf_maxlen)
        tlen = payload_buf_maxlen;

    strncpy((char *)payload, text, tlen);
    payload_length = tlen;
    return tlen;
}

size_t NDEF_TXT::write(const uint8_t *text, size_t len)
{
    if (!payload_buf_maxlen)
        return 0;  // Not possible!  This isn't a general-purpose writable buffer.

    if (len > (payload_buf_maxlen - payload_length))
        len = payload_buf_maxlen - payload_length;

    strncat((char *)payload, (const char *)text, len);
    payload_length += len;
    return len;
}

size_t NDEF_TXT::write(uint8_t c)
{
    if (!payload_buf_maxlen)
        return 0;  // Not possible!  This isn't a general-purpose writable buffer.

    if (payload_length == payload_buf_maxlen)
        return 0;  // No more room!

    payload[payload_length++] = c;
    payload[payload_length] = '\0';
    return 1;
}

int NDEF_TXT::sendTo(Print &p, boolean first_msg, boolean last_msg)
{
    uint32_t real_plen = payload_length + lang_length + 1;
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
    if (is_utf16)
        p.write(NDEF_RTD_TEXT_STATUS_UTF16 | lang_length);
    else
        p.write(lang_length);
    printedSize += 2;

    p.write((const uint8_t *)lang, lang_length);
    printedSize += lang_length;

    p.write((const uint8_t *)payload, payload_length);
    printedSize += payload_length;
    // NOTE: printedSize may be a smaller variable than payload_length

    return printedSize;
}

int NDEF_TXT::import(Stream &s)
{
    int c, ndef_hdr;
    size_t plen, plen_write, llen;
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
    if ( (c & 0x3F) < 1 || (c & 0x40) )
        return -1;  // Invalid abbreviation byte
    if (c & NDEF_RTD_TEXT_STATUS_UTF16)
        is_utf16 = true;
    else
        is_utf16 = false;
    plen--;

    // read LANGUAGE
    llen = c & 0x3F;
    if (llen > 8)
        return -1;  // We're not supporting >8 byte language codes for now
    if (s.readBytes(lang, llen) < llen)
        return -1;
    lang_length = llen;
    plen -= llen;

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
