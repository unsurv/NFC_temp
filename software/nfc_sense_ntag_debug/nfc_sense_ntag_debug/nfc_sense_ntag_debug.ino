#include <ntageepromadapter.h>
#include "NdefMessage.h"
#include "Arduino.h"
#define HARDI2C
#include <Wire.h>

#include <avr/sleep.h>

// #define NFC_SENSE_DEBUG

#include <Adafruit_TMP116.h>
#include <Adafruit_TMP117.h>

#include <Adafruit_Sensor.h>

// Adafruit_TMP116  tmpThermometer;
Adafruit_TMP117  tmpThermometer;

Ntag ntag(Ntag::NTAG_I2C_1K, 0x54);
NtagEepromAdapter ntagEepromAdapter(&ntag);

void setup(){

  // RTC.CTRLA |= (0x28);
  
  // TODO add check if NTAG is in I2C master mode. if in master mode. go to sleep.
  
  ntagEepromAdapter.clean();
  
  #ifdef NFC_SENSE_DEBUG
    Serial.begin(9600);
  #endif

  Wire.begin();
  // power saving
  // TCA0.SPLIT.CTRLA = 0;
  // ADCPowerOptions(ADC_DISABLE);


  /*
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  pinMode(5, OUTPUT);
  
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  
  pinMode(17, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);

  // can't do too much computation before the actual ndef msg creation or we are too slow
  if (!getSerialNumber()) {
    // memory access is not possible when in i2c master mode
    Serial.println("access not possible, assuming master mode goto sleep");
    // goto sleep
    ADC0.CTRLA &= ~ADC_ENABLE_bm; // Very important on the tinyAVR 2-series
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
  
    }

  */
  

  if (!tmpThermometer.begin()) {
    // updateNFC(targetOS, "TMP117 not found. Aborting...");
      
    #ifdef NFC_SENSE_DEBUG
      Serial.println("TMP thermometer not found aborting");
    #endif


    // NFC info if TMP not found
    NdefMessage message = NdefMessage();
    message.addUrlRecord(KNOWN_TYPE_HTTP, "127.0.0.1/ThermometerNotFound");
  
    // ntagEepromAdapter.clean();
    ntag.releaseI2c();
    ntagEepromAdapter.writeMod(message);
    
    // Before sleeping
    ADC0.CTRLA &= ~ADC_ENABLE_bm; // Very important on the tinyAVR 2-series
    
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
  }
  else
  {
  // sensors_event_t temp; // create an empty event to be filled
  tmpThermometer.reset();
  delay(5);
  tmpThermometer.setMeasurementMode(TMP117_MODE_ONE_SHOT);
  sensors_event_t temp; // create an empty event to be filled

  int i = 0;
  while (!tmpThermometer.dataReady()) {
    delay(1);
    i++;
  }
  #ifdef NFC_SENSE_DEBUG
    Serial.println(i);
  #endif
  
  tmpThermometer.getEvent(&temp);
  
  NdefMessage message = NdefMessage();
  message.addUrlRecord(KNOWN_TYPE_HTTP, "127.0.0.1/temp=" + String(temp.temperature, 1) + "C");
  
  // ntagEepromAdapter.clean();
  // ntag.releaseI2c();
  ntagEepromAdapter.writeMod(message);
  #ifdef NFC_SENSE_DEBUG
    Serial.println("127.0.0.1/temp=" + String(temp.temperature, 1) + "C");
  #endif
  
  // tmpThermometer.getEvent(&temp); //fill the empty event object with the current measurements
  // NdefMessage emptyMessage = NdefMessage();
  // message.addUriRecord("http://123");
  // message.addUrlRecord(KNOWN_TYPE_HTTP, "127.0.0.1/temp=" + String(temp.temperature, 1) + "C");
  // emptyMessage.addEmptyRecord();
  
  // ntag.unlockEeprom();
  }
  

  // Before sleeping
  /*
  ADC0.CTRLA &= ~ADC_ENABLE_bm; // Very important on the tinyAVR 2-series
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  */

  

  
}

void loop(){

  /*
  sensors_event_t temp; // create an empty event to be filled

  tmpThermometer.getEvent(&temp); //fill the empty event object with the current measurements

  NdefMessage message = NdefMessage();
  message.addUrlRecord(KNOWN_TYPE_HTTP, "127.0.0.1/temp=" + String(temp.temperature, 1) + "C");
  
  // ntagEepromAdapter.clean();
  ntag.releaseI2c();
  ntagEepromAdapter.writeMod(message);
  #ifdef NFC_SENSE_DEBUG
    Serial.println("127.0.0.1/temp=" + String(temp.temperature, 1) + "C");
  #endif
  
  delay(250);
  */
  // getEnergyHarvestingStatus();
    
}

void getEnergyHarvestingStatus(){
    byte* sn=(byte*)malloc(ntag.getHarvestingLength());
    Serial.println();
    if(ntag.getEnergyHarvestingStatus(sn,2))
    {
        Serial.print("Energy Harvesting Status is: ");
        for(byte i=0;i<ntag.getUidLength();i++)
        {
            Serial.print(sn[i], HEX);
            Serial.print(" ");
        }
    }
    Serial.println();
    free(sn);
}

void testWriteAdapter(){
    NdefMessage message = NdefMessage();
    // message.addUriRecord("http://123");
    message.addUrlRecord(KNOWN_TYPE_HTTP, "127.0.0.1/temp=23.9C");
    if(ntagEepromAdapter.writeMod(message)){
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

bool getSerialNumber(){
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
        free(sn);
        return true;
    } else {
      free(sn);
      return false;
    }
    Serial.println();
    
}

void testUserMem(){
    int writeLength = 20;
    int readLength = 40;
    byte eepromdata[writeLength];
    byte readeeprom[readLength];

    for(byte i=0;i<writeLength;i++){
        eepromdata[i]=0x80 | i;
    }
    
    /*
    Serial.println("Writing block 1");
    if(!ntag.writeEepromMod(0x0000,eepromdata,writeLength)){
        Serial.println("Write block 1 failed");
    }
    
    
    Serial.println("Writing block 2");
    if(!ntag.writeEeprom(16,eepromdata+16,16)){
        Serial.println("Write block 2 failed");
    }*/
    
    Serial.println("\nReading memory block 1");
    if(ntag.readEepromMod(0x0000,readeeprom,readLength)){
        showBlockInHex(readeeprom,readLength);
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
