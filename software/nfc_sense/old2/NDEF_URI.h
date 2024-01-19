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

#ifndef NDEF_URI_H
#define NDEF_URI_H

#include "NDEF.h"

class NDEF_URI : public NDEF {
    protected:
        // Inherited: tnf, type_length, id_length, payload_length, type, id, payload
        uint8_t prefix;

    public:
        NDEF_URI();
        NDEF_URI(const char *uri);

        int setURI(const char *uri);
        static uint8_t compressPrefix(const char *uri);
        static const char * decompressPrefix(const uint8_t pfx);
        int storeURI(char *buf, size_t maxlen);
        int printURI(Print &p);

        int sendTo(Print &p, boolean first_msg = true, boolean last_msg = true);   // For outputting to a suitable NFC passive device
        int import(Stream &s);  // For reading from a suitable NFC passive device, or any form of stream
};

#endif /* NDEF_URI_H */
