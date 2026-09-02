

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>
#include "RF430CL330H_Shield.h"

#define IRQ   (5)
#define RESET (4)

#define OS_ANDROID 0
#define OS_IOS 1

#define RF430_DEFAULT_DATA                                                              \
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
0x00, 0x14, /* NLEN; NDEF length (2 byte long message) */                               \
0xD1, 0x01, 0x10,                                                                       \
0x54, /* T = text */                                                                    \
0x02,                                                                                   \
0x65, 0x6E, /* 'e', 'n', */                                                             \
                                                                                        \
/* 'Hello, world!' NDEF data; Empty NDEF message, length should match NLEN*/            \
0x48, 0x65, 0x6C, 0x6C, 0x6f, 0x2c, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21            \
}



volatile byte into_fired = 0;
uint16_t flags = 0;


byte nfcTemplate[] = {
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
/* NDEF File */                                                                         \
                                                                                        \
/* NLEN; NDEF length (2 byte long message) all bytes below, gets set in code below */   \
0x00, 0x00,                                                                             \
                                                                                        \
0xC1, /* Record Header , not a short record 11010001*/                                  \
0x01, /* Type Length */                                                                 \
0x00, 0x00, 0x00, 0x00, /* payload length*/                                             \
0x54, /* type  T = text */                                                              \
                                                                                        \
/* PAYLOAD NDEF data;*/                                                                 \
0x02,  /* ID length  */                                                                 \
0x65, 0x6E, /* 'e', 'n', */                                                             \

            
};

RF430CL330H_Shield nfc(IRQ, RESET);

/**
**  @brief  interrupt service
**/
void RF430_Interrupt()
{
    into_fired = 1;
    // enable ADC
    // ADC0.CTRLA |= ~ADC_ENABLE_bm;
    detachInterrupt(1);//cancel interrupt


}

void setupNFC()
{

  // pinMode(IRQ, INPUT);
  pinMode(RESET, OUTPUT);
  
  // Reset the RF430
  
  digitalWrite(RESET, HIGH);
  digitalWrite(RESET, LOW);
  delay(10);                   //Reset:low level 100ms
  digitalWrite(RESET, HIGH);
  delay(10);
  
  /*
  nfc.Write_Register(CONTROL_REG, SW_RESET);

  delay(20);
  */
  //Enable interrupts for End of Read and End of Write
  // nfc.Write_Register(INT_ENABLE_REG, EOW_INT_ENABLE + EOR_INT_ENABLE);

  // Configure INTO pin for active low
  // nfc.Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE);
  
  /*
  uint16_t version;

  version = nfc.Read_Register(VERSION_REG);
      /** Errata Fix : Unresponsive RF - recommended firmware
  *   reference: RF430CL330H Device Erratasheet, SLAZ540D-June 2013-Revised January
  */

  /*
  if (version == 0x0101 || version == 0x0201)
  { // the issue exists in these two versions
      nfc.Write_Register(TEST_MODE_REG, TEST_MODE_KEY);
      nfc.Write_Register(CONTROL_REG, 0x0080);
      if (version == 0x0101)
      { // Ver C
          nfc.Write_Register(0x2a98, 0x0650);
      }
      else
      { // Ver D
          nfc.Write_Register(0x2a6e, 0x0650);

      }
      nfc.Write_Register(0x2814, 0);
      nfc.Write_Register(TEST_MODE_REG, 0);
  }
  */

  // update nfc tag

  // while(!(nfc.Read_Register(STATUS_REG) & READY)); //wait until READY bit has been set
  // Serial.print("Firmware Version:"); Serial.println(nfc.Read_Register(VERSION_REG), HEX);

  // working up to 256 bytes payload
  // static length 35 bytes
  // nlen at index 26 & 27
  
  byte nfcTemplateStatic_ASDASD[] = {
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
                                                           \
                                                                                           
  };

    byte nfcTemplateStatic[] = {
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
  0x00, 0xF0, /* Max NDEF size (255 bytes of usable memory) */                            \
  0x00,       /* NDEF file read access condition, read access without any security */     \
  0x00,       /* NDEF file write access condition; write access without any security */   \
                                                                                                                                                                             
  };

      byte nfcTemplateStatic_iOS[] = {
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
  0x00, 0xF0, /* Max NDEF size (255 bytes of usable memory) */                            \
  0x00,       /* NDEF file read access condition, read access without any security */     \
  0x00,       /* NDEF file write access condition; write access without any security */   \
  0xE1, 0x04,     /* NDEF File ID NEW NEW NEW*/                                           \  
                                                                                                                                                                             
  };
    
  //write NDEF memory with Capability Container + NDEF message
  nfc.Write_Continuous(0, nfcTemplateStatic, sizeof(nfcTemplateStatic));


  // nfc.Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE);
}



void updateNFC(int osType, String nfcString)
{


  byte NDEFfieldsLength_asdasdasdasdasd[] = {
  /* NDEF File for Hello World  (48 bytes total length) */                                \
  0x00, 0x13, /* NLEN; NDEF length (2 byte long message) */                               \
  0xD1, /* Record Header  */                                                              \
  0x01, /* Type Length */                                                                 \
  0x00, /* bytes after this -1  = NLEN - 4*/                                              \
  0x54, /* type  T = text */                                                              \
  0x02,  /* ID length  */                                                                 \
  0x65, 0x6E, /* 'e', 'n', */                                                             \
  };

  byte NDEFfieldsLength_correct[] = {
  /* NDEF File for Hello World */                                                        
  0x00, 0x00, /* NLEN; NDEF length will be updated */                                     
  0xD1,       /* Record Header (MB=1, ME=1, CF=0, SR=1, IL=0, TNF=0x01) */                                                              
  0x01,       /* Type Length */                                                                 
  0x00,       /* Payload Length (will be updated) */                                              
  0x54,       /* Type: 'T' for text */                                                             
  0x02,       /* Language Code Length */                                                                 
  0x65, 0x6E  /* Language Code: 'en' */                                                             
  };
  
  byte NDEFfieldsLength_URL[] = {
  /* NDEF File for Hello World */                                                        
  0x00, 0x00,       /* NLEN; NDEF length will be updated */                                     
  0xD1,       /* Record Header (MB=1, ME=1, CF=0, SR=1, IL=0, TNF=0x01) */                                                              
  0x01,       /* Type Length */                                                                 
  0x0B,       /* Payload Length (will be updated) */                                              
  0x55,       /* Type: 'T' for text */                                                             
  0x03,                                                             
  };
  
  if (osType == OS_IOS) {
    byte NDEFfieldsLength[] = {
      0xE1, 0x04,     /* NDEF File ID NEW NEW NEW*/
  
      // NDEF File body starts immediately after CC file
      0x00, 0x00, /* NLEN = bytes after this */                                     
      0xD1,       /* Record Header (MB=1, ME=1, CF=0, SR=1, IL=0, TNF=0x01) */                                                              
      0x01,       /* Type Length */                                                                 
      0x00,       /* Payload Length = bytes after this -1(will be updated) */                                              
      0x55,       /* Type = URI */                                                             
      0x02,
    };
  } else {
    
    byte NDEFfieldsLength[] = {
    /* NDEF File for Hello World */                                                        
      0x00, 0x00, /* NLEN; NDEF length will be updated */                                     
      0xD1,       /* Record Header (MB=1, ME=1, CF=0, SR=1, IL=0, TNF=0x01) */                                                              
      0x01,       /* Type Length */                                                                 
      0x00,       /* Payload Length (will be updated) */                                              
      0x54,       /* Type: 'T' for text */                                                             
      0x02,       /* Language Code Length */                                                                 
      0x65, 0x6E  /* Language Code: 'en' */                                                             
    };
  }

  byte NDEFfieldsLength_iOS[] = {
      // NDEF File body starts immediately after CC file
      0x00, 0x00, /* NLEN = bytes after this */                                     
      0xD1,       /* Record Header (MB=1, ME=1, CF=0, SR=1, IL=0, TNF=0x01) */                                                              
      0x01,       /* Type Length */                                                                 
      0x00,       /* Payload Length = bytes after this -1(will be updated) */                                              
      0x55,       /* Type = URI */                                                             
      0x00,
    };
 byte NDEFfieldsLength[] = {
    /* NDEF File for Hello World */                                                        
      0x00, 0x00, /* NLEN; NDEF length will be updated */                                     
      0xD1,       /* Record Header (MB=1, ME=1, CF=0, SR=1, IL=0, TNF=0x01) */                                                              
      0x01,       /* Type Length */                                                                 
      0x00,       /* Payload Length (will be updated) */                                              
      0x54,       /* Type: 'T' for text */                                                             
      0x02,       /* Language Code Length */                                                                 
      0x65, 0x6E  /* Language Code: 'en' */                                                             
    };

  // input
  int stringSize = nfcString.length() + 1;
  // Serial.println("String size");
  // Serial.println(stringSize);
  byte inputString[stringSize];
  
  // transfer String
  nfcString.getBytes(inputString, stringSize);

  if (osType == OS_IOS) {
    // set lengths in NDEF
    NDEFfieldsLength[1] = stringSize + 4;
    NDEFfieldsLength[4] = stringSize + 4 - 4;
  } else {
    // set lengths in NDEF
    NDEFfieldsLength[1] = stringSize + 8;
    NDEFfieldsLength[4] = stringSize + 8 - 4;
  }

  // total size
  int nfcInputSize = sizeof(NDEFfieldsLength) + stringSize;
  byte nfcInput[nfcInputSize];

  int pointer;
  for (int i = 0 ; i < sizeof(NDEFfieldsLength); i++) {
      nfcInput[i] = NDEFfieldsLength[i];
      pointer = i;
      
      }
  
  for (int i = 0, j = pointer + 1; i < stringSize ,  j < pointer + 1 + stringSize; i++, j++) { 
      /*Serial.println("i");
      Serial.println(i);
      Serial.println("j");
      Serial.println(j);
      nfcInput[j] = inputString[i];
      Serial.println(inputString[i]);
      */
      nfcInput[j] = inputString[i];
      }
  /*
  for (int f = 0; f < nfcInputSize; f++)
  {
    Serial.println(nfcInput[f]);
  }
  */

  //Enable interrupts for End of Read and End of Write
  // nfc.Write_Register(INT_ENABLE_REG, EOW_INT_ENABLE + EOR_INT_ENABLE);

  nfc.Write_Extended_NDEFmessage(nfcInput, sizeof(nfcInput));
  // nfc.Write_Extended_NDEFmessage(nfcUrlTest, sizeof(nfcUrlTest));
  // nfc.Write_NDEFmessage(nfcInput, sizeof(nfcInput));
  
  //Configure INTO pin for active low and enable RF
  nfc.Write_Register(CONTROL_REG, RF_ENABLE);

}


  byte nfcTemplateBackup[] = {
  /*NDEF Tag Application Name*/                                                           \
  0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01,      /* 7 bytes */                            \
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
  0x00, 0x13, /* NLEN; NDEF length (2 byte -> long message) */                               \
  0xD1, /* Record Header  */                                                              \
  0x01, /* Type Length */                                                                 \
  0x0F, /* bytes after this -1  = NLEN - 4*/                                              \
  0x54, /* type  T = text */                                                              \
  0x02,  /* ID length  */                                                                 \
  0x65, 0x6E, /* 'e', 'n', */                                                             \
                                                                                          \
  /* PAYLOAD NDEF data;*/
  'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'    // 11 bytes  
  };

// working up to 256 bytes payload
byte nfcTemplateBackup2[] = {
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
0x00, 0x12, /* NLEN; NDEF length (2 byte long message) */                               \
0xD1, /* Record Header  */                                                              \
0x01, /* Type Length */                                                                 \
0x0E, /* bytes after this -1  = NLEN - 4*/                                              \
0x54, /* type  T = text */                                                              \
0x02,  /* ID length  */                                                                 \
0x65, 0x6E, /* 'e', 'n', */                                                             \
                                                                                        \
/* PAYLOAD NDEF data;*/            
};
