/* Hello World - NFC edition
 *
 * Designed for the RF430CL330H BoosterPack with Energia
 */

#include "RF430CL.h"
#include <Wire.h>

#define RF430CL330H_BOOSTERPACK_RESET_PIN  1
#define RF430CL330H_BOOSTERPACK_IRQ_PIN    0

RF430 nfc(RF430CL330H_BOOSTERPACK_RESET_PIN, RF430CL330H_BOOSTERPACK_IRQ_PIN);

const uint8_t ndef_data_HelloWorld[] = {
  0xD1,           // MB (Message Begin), SR (Short Record), TNF = 1
  0x01, 16,       // Type Length = 0x01, Payload Length
  'T',            // Type = T (Text)
  0x02,           // 1st payload byte - UTF-8, IANA country code = 2 characters
  'e', 'n',       // 'e', 'n' (2-character IANA country code for text language)
  'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'  // Text!
};

const uint8_t ndef_data_HelloWorld2[] =
{                                                                                       \
/*NDEF Tag Application Name*/                                                           \
0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01,                                               \
                                                                                        \
/*Capability Container ID*/                                                             \
0xE1, 0x03,                                                                             \
0x00, 0x0F, /* CCLEN */                                                                 \
0x20,       /* Mapping version 2.0 */                                                   \
0x00, 0xF9, /* MLe (249 bytes); Maximum R-APDU data size */                             \
0x00, 0xF6, /* MLc (246 bytes); Maximum C-APDU data size */                             \
0x04,       /* Tag, File Control TLV (4 = NDEF file) */                                 \
0x06,       /* Length, File Control TLV (6 = 6 bytes of data for this tag) */           \
0xE1, 0x04, /* File Identifier */                                                       \
0x0B, 0xDF, /* Max NDEF size (3037 bytes of useable memory) */                          \
0x00,       /* NDEF file read access condition, read access without any security */     \
0x00,       /* NDEF file write access condition; write access without any security */   \
                                                                                        \
/* NDEF File ID */                                                                      \
0xE1, 0x04,                                                                             \
                                                                                        \
/* NDEF File for Hello World  (48 bytes total length) */                                \
0x00, 0x2A, /* NLEN; NDEF length (2 byte long message) */                               \
0xD1, /* Record Header  */                                                              \
0x01, /* Type Length */                                                                 \
0x26, /* bytes after this -1  = NLEN - 4*/                                              \
0x54, /* type  T = text */                                                              \
0x02,  /* ID length  */                                                                 \
0x65, 0x6E, /* 'e', 'n', */                                                             \
                                                                                        \
/* PAYLOAD NDEF data; Empty NDEF message, length should match NLEN*/            \
0x68, 0x65, 0x72, 0x65, 0x20, 0x63, 0x6f, 0x75, 0x6c, 0x64, 0x20, 0x62, 0x65, 0x20,     \
0x61, 0x20, 0x6a, 0x73, 0x6f, 0x6e, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x20, 0x77, 0x69,     \
0x74, 0x68, 0x20, 0x69, 0x6e, 0x66, 0x6f      \
};

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("Initializing I2C-");
  Wire.begin();

  Serial.println("Initializing RF430CL330H-");
  nfc.begin();

  Serial.println("Writing Hello World NDEF data-");
  nfc.write(ndef_data_HelloWorld, sizeof(ndef_data_HelloWorld));

  Serial.print("Writing NDEF size ("); Serial.print(sizeof(ndef_data_HelloWorld)); Serial.println(" bytes)-");
  nfc.setDataLength(sizeof(ndef_data_HelloWorld));

  Serial.println("RF enable-");
  nfc.enable();  // Activate RF
}

void loop() {
  uint16_t i;


  if (!digitalRead(RF430CL330H_BOOSTERPACK_IRQ_PIN)) {  // IRQ line active?
    nfc.disable();  // Must turn off RF before accessing registers or SRAM

    i = nfc.readReg(RF430_REG_INT_FLAG);
    if (i) {
      Interpret_RF430_Interrupt_Flag(i);
      nfc.writeReg(RF430_REG_INT_FLAG, i);
    } else {
      Serial.println('.');
    }

    nfc.enable();
  }

  delay(100);
}

void Interpret_RF430_Interrupt_Flag(uint16_t flg)
{
  Serial.print("RF430 Interrupt Flags: ");

  if (flg & RF430_INT_END_OF_READ) Serial.print("END_OF_READ ");
  if (flg & RF430_INT_END_OF_WRITE) Serial.print("END_OF_WRITE ");
  if (flg & RF430_INT_CRC_COMPLETED) Serial.print("CRC_COMPLETED ");
  if (flg & RF430_INT_BIP8_ERROR) Serial.print("BIP-8_ERROR ");
  if (flg & RF430_INT_NDEF_ERROR) Serial.print("NDEF_FORMAT_ERROR ");
  if (flg & RF430_INT_GENERIC_ERROR) Serial.print("GENERIC_ERROR ");
  Serial.println();
}
