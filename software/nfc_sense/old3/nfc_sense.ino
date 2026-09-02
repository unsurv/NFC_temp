#include "Wire.h"
#include "NfcUtils.h"
#include "RF430CL330H_Shield.h"
#include <avr/sleep.h>

#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>

bool postData = false;
Adafruit_TMP117  tmp117;

void setup()
{
 
  // Serial.begin(9600);

  // power saving
  TCA0.SPLIT.CTRLA = 0;
  ADCPowerOptions(ADC_DISABLE);

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
  
  /*
  pinMode(17, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);
  */
  
  // temp INPUT
  // pinMode(18, OUTPUT);
  
  
  /*
  for (uint8_t pin=0; pin < 8; pin++) {
     (&PORTA.PIN0CTRL)[pin] = PORT_ISC_INPUT_DISABLE_gc; //Disable on PAx pin
     (&PORTB.PIN0CTRL)[pin] = PORT_ISC_INPUT_DISABLE_gc; //Disable on PBx pin
     (&PORTC.PIN0CTRL)[pin] = PORT_ISC_INPUT_DISABLE_gc; //Disable on PCx pin
    }
  /*
   
  */

  // join I2C bus (I2Cdev library doesn't do this automatically)
 
  // Serial.begin(9600);

  /*
  // old analog temp sense code
  analogReference(INTERNAL2V048); // set reference to the desired voltage, and set that as the ADC reference.
  analogReference(VDD); // set reference to the desired voltage, and set that as the ADC reference.
  analogReadResolution(12);
  int temp = analogRead(18);
  */

  // int targetOS = OS_IOS;
  int targetOS = OS_ANDROID;
  #define TARGET_IOS
  // #define TARGET_ANDROID

  Wire.begin();

  //enable interrupt 1
  // attachInterrupt(digitalPinToInterrupt(IRQ), nfcIntHandler, FALLING);
  
  // Try to initialize!
  setupNFC();

  if (!tmp117.begin()) {
    updateNFC(targetOS, "TMP117 not found. Aborting...");
  }
  else
  {

  sensors_event_t temp; // create an empty event to be filled
  tmp117.getEvent(&temp); //fill the empty event object with the current measurements
   
  // Fahrenheit
  // updateNFC(targetOS, "Temperature: " + String((temp.temperature * 9/5) + 32, 1) + " °F") ;
  
  // Celcius
  updateNFC(targetOS, "Temperature: " + String(temp.temperature, 1) + " °C") ;
  
  // test
  // updateNFC(targetOS, "http://ha:8123/api/webhook/nfc-temp-value?temp=" + String(temp.temperature, 1));


  // RF430CL330H_Shield nfc(IRQ, RESET);
  
  // nfc.begin();
  }
  

  // Before sleeping
 
  ADC0.CTRLA &= ~ADC_ENABLE_bm; // Very important on the tinyAVR 2-series
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();

}



void nfcIntHandler()
{
  postData = true;
  
}


void loop()
{ 
  /*
  Serial.println("loop");
  setupNFC();
  int tempRead = analogRead(18);
  updateNFC("Temp Value: " + String(tempRead));
  delay(500);
  */
}
