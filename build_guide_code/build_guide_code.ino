//Brett Stoddard

//uses analog pins on M0 
//saves data to sd card

//SD card stuff
#include <SPI.h>
#include "SdFat.h"
SdFat SD;
#define RFM95_CS 8  //for turning off lora
#define SD_CS_PIN 9 //pin 5 is select for SD card
File myFile;
#define FILENAME "sap_5_16.csv"
String data = "";

//Sap flow stuff
int a_pins[2] = { A1 , A2 }; //Input pins
int a_data[2]; //Where values are saved

void setup() {
  //heater
  pinMode(12, OUTPUT ); //set pin 12 as output for heater controller
  
  Serial.begin( 9600 ); //wait for Serial before starting
  while( !Serial ){ //If Serial doesn't open after 5 secs, continue
    if( millis() > 5000 )
      break;
    delay(10);
  } //This is used in case this is used without a usb

  //setup for SD card
  digitalWrite( RFM95_CS , HIGH ); //deselect lora 
  Serial.println(F("Initializing SD card..."));
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("SD initialization failed!"));
    return;
  }  
  sd_write( "A1 (mV), A2 , Diff (mV), min, hour, day" );
}

int count = 0;
void loop() {
  count++;
  heat();
  take_temp();
  time_();
  sd_write( data );
  data = ""; //clear buffer
  delay( 60000*5 ); // wait 5 min
}

//Sends a heat pulse for 10 seconds 
//Turns on heater, wait 10 sec, turn off heater
void heat(){
  digitalWrite(12, HIGH); //turn on heater
  int delay_time = 10000; //http://www.open-sensing.org/sapflowmeter-blog/2018/6/4/sap-flux-heat-calculations
  delay( delay_time );   //wait delay_time sec
  digitalWrite(12, LOW); //turn off heater
}

//write data to sd card
bool sd_write( String to_log ){ 
  myFile = SD.open( FILENAME , FILE_WRITE );
  if (myFile) {                 //if file ok, log data
    Serial.print(F("Writing to file..."));
    myFile.println( to_log );
    //close the file:
    myFile.close();
    Serial.println(F("Done writing to file."));
    
  } else {                      //if file bad, return error
    // if the file didn't open, print an error:
   Serial.println(F("Error opening file"));
   //blink_a();
   return false;
  }
  Serial.println( to_log );
  return true;
}

//record the time of the sensor
//this is pretty inaccurate
//for better accuracy, a dedicated RTC should be used
void time_(){
  long t = millis();
  long sec_ = (t / 1000) % 60;
  long min_ = ((t / 1000) / 60) % 60;
  long hour_ =(((t / 1000) / 60 ) / 60) % 24;
  data += String( sec_ )+","+String( min_ )+","+String( hour_ );
}

//Take the temperature of the top and bottom plants as voltage
void take_temp(){
  for(int i=0; i<2; i++){ //loop through to take values for both top and bottom probes
    a_data[i] = analogRead( a_pins[i] ); //analog Read gives values from 0-1023 based on 0-3.3V
    a_data[i] = map(a_data[i], 0, 1023, 0, 3300);//map these to mV value
    data += String( a_data[i] ); data+=","; //Add to buffer
    Serial.print("Printing pin ");Serial.print(i);Serial.print(" ");Serial.println(a_data[i]); //prints to debug console 
  }
}

