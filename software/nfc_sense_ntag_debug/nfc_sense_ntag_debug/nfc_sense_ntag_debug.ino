#include <ntagsramadapter.h>
#include "Arduino.h"
#define HARDI2C
#include <Wire.h>

Ntag ntag(Ntag::NTAG_I2C_1K, 0x54);
NtagSramAdapter ntagAdapter(&ntag);

void setup(){
    Serial.begin(115200);
    
}

void loop(){
    Serial.println("start");
    if(!ntag.begin()){
        Serial.println("Can't find ntag");
    }
    getSerialNumber();
    testUserMem();
    // testRegisterAccess();
    // testSramMirror();
    // testSram();

    delay(1000);
}

void testWriteAdapter(){
    NdefMessage message = NdefMessage();
    message.addUriRecord("http://www.google.be");
    if(ntagAdapter.write(message)){
        Serial.println("Message written to tag.");
    }
}

void testSram(){
    byte data[16];
    Serial.println("Reading SRAM block 0xF8");
    if(ntag.readSram(0,data,16)){
        showBlockInHex(data,16);
    }
    for(byte i=0;i<16;i++){
        data[i]=0xF0 | i;
    }
    Serial.println("Writing dummy data to SRAM block 0xF8");
    if(!ntag.writeSram(0,data,16)){
        return;
    }
    Serial.println("Reading SRAM block 0xF8 again");
    if(ntag.readSram(0,data,16)){
        showBlockInHex(data,16);
    }
}

void testSramMirror(){
    byte readeeprom[16];
    byte data;

    if(!ntag.setSramMirrorRf(false,1))return;
    Serial.println("\nReading memory block 1, no mirroring of SRAM");
    if(ntag.readEeprom(0,readeeprom,16)){
        showBlockInHex(readeeprom,16);
    }
    Serial.println("\nReading SRAM block 1");
    if(ntag.readSram(0,readeeprom,16)){
        showBlockInHex(readeeprom,16);
    }
    if(!ntag.setSramMirrorRf(true,1))return;
    Serial.print("NC_REG: ");
    if(ntag.readRegister(Ntag::NC_REG,data)){
        Serial.println(data, HEX);
    }
    Serial.println("Use an NFC-reader to verify that the SRAM has been mapped to the memory area that the reader will access by default.");
}

void testRegisterAccess(){
    byte data;
    Serial.println(ntag.readRegister(Ntag::NC_REG,data));
    Serial.println(data,HEX);
    Serial.println(ntag.writeRegister(Ntag::NC_REG,0x0C,0x0A));
    Serial.println(ntag.readRegister(Ntag::NC_REG,data));
    Serial.println(data,HEX);
}

void getSerialNumber(){
    byte* sn=(byte*)malloc(ntag.getUidLength());
    Serial.println();
    if(ntag.getUid(sn,2))
    {
        Serial.print("Serial number of the tag is: ");
        for(byte i=0;i<ntag.getUidLength();i++)
        {
            Serial.print(sn[i], HEX);
            Serial.print(" ");
        }
    }
    Serial.println();
    free(sn);
}

void testUserMem(){
    byte eepromdata[2*16];
    byte readeeprom[16];

    for(byte i=0;i<2*16;i++){
        eepromdata[i]=0x80 | i;
    }
    
    
    Serial.println("Writing block 1");
    if(!ntag.writeEeprom(0,eepromdata,16)){
        Serial.println("Write block 1 failed");
    }
    
    /*
    Serial.println("Writing block 2");
    if(!ntag.writeEeprom(16,eepromdata+16,16)){
        Serial.println("Write block 2 failed");
    }*/
    
    Serial.println("\nReading memory block 1");
    if(ntag.readEepromMod(0x0006,readeeprom,4)){
        showBlockInHex(readeeprom,4);
    }
    /*
    Serial.println("Reading memory block 2");
    if(ntag.readEeprom(16,readeeprom,16)){
        showBlockInHex(readeeprom,16);
    }
    Serial.println("Reading bytes 10 to 20: partly block 1, partly block 2");
    if(ntag.readEeprom(10,readeeprom,10)){
        showBlockInHex(readeeprom,10);
    }
    /*
    Serial.println("Writing byte 15 to 20: partly block 1, partly block 2");
    for(byte i=0;i<6;i++){
        eepromdata[i]=0x70 | i;
    }
    if(ntag.writeEeprom(15,eepromdata,6)){
        Serial.println("Write success");
    }
    
    
    Serial.println("\nReading memory block 1");
    if(ntag.readEeprom(0,readeeprom,16)){
        showBlockInHex(readeeprom,16);
    }
    Serial.println("Reading memory block 2");
    if(ntag.readEeprom(16,readeeprom,16)){
        showBlockInHex(readeeprom,16);
    }
    */
}

void showBlockInHex(byte* data, byte size){
    for(int i=0;i<size;i++){
        Serial.print(data[i],HEX);
        Serial.print(" ");
    }
    Serial.println();
}
