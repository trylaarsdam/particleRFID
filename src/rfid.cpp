/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "d:/Dev/rfid/src/rfid.ino"
void setup();
uint8_t getID();
void unlock();
void resetInfo();
void dump_byte_array(byte* buffer, byte bufferSize);
void loop();
#line 1 "d:/Dev/rfid/src/rfid.ino"
SYSTEM_MODE(MANUAL);

#include <MFRC522.h>
#include <neopixel.h>
/*
 * Project rfid
 * Description:
 * Author: Todd Rylaarsdam
 * Date: 11/7/2020
 */

#define EEPROMRESET D2
#define LED_PIN D6
#define PROGRAMINPUT D7
#define NUMPIXELS 12
#define DELAYVAL 250
#define FADEVAL 5
#define MAXVAL 50

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, WS2812B);

#define RST_PIN         D5         
#define SS_PIN          D4

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;
uint8_t successRead;

MFRC522::MIFARE_Key key;
MFRC522::MIFARE_Key newKeyA = {keyByte: {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}};
MFRC522::MIFARE_Key newKeyB = {keyByte: {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5}};

byte readCard[4];
byte masterCard[4];

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  pinMode(D3, OUTPUT);
  pinMode(EEPROMRESET, INPUT);


  Serial.begin(9600);
  while(!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  pixels.begin();
  delay(1000);
  if(digitalRead(EEPROMRESET) == HIGH){
    Serial.println("CLEARING EEPROM");
    EEPROM.clear();
    for(int i=0; i<NUMPIXELS; i = i + 2){
      pixels.setPixelColor(i, pixels.Color(0,0,255));
      pixels.show();
      delay(100);
    }
    delay(500);
  }

  if(EEPROM.read(1) != 143) {
    Serial.println("No master card defined");
    Serial.println(EEPROM.read(1));
    Serial.println("Scan card to define as master card.");
    do {
      successRead = getID();
      for(int i=0; i<NUMPIXELS; i = i + 2){
        pixels.setPixelColor(i, pixels.Color(0,50,50));
        pixels.show();
      }
      delay(500);
      for(int i=0; i<NUMPIXELS; i = i + 2){
        pixels.setPixelColor(i, pixels.Color(0,0,50));
        pixels.show();
      }
      delay(500);
    }
    while(!successRead);
    successRead = false;
    for(uint8_t j=0; j<4; j++){
      EEPROM.write(2+j, readCard[j]);
    }
    for(uint8_t i=0; i<4; i++){
      masterCard[i] = EEPROM.read(2+i);
    }
    EEPROM.write(1,(uint8_t)143);
    Serial.println("Master Card Defined");
    for(int i=0; i<NUMPIXELS; i = i + 2){
      pixels.setPixelColor(i, pixels.Color(0,50,0));
      pixels.show();
    }
    delay(500);
  }
  for(uint8_t i=0; i<4; i++){
    masterCard[i] = EEPROM.read(2+i);
  }
  Serial.print("Master card stored: ");
  for(uint8_t i=0; i<4; i++){
    Serial.println(masterCard[i]);
  }
  Serial.println();
  for(int i=0; i<NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();    
  }
}

uint8_t getID() {
  if(!mfrc522.PICC_IsNewCardPresent()){
    return 0;
  }
  if(!mfrc522.PICC_ReadCardSerial()){
    return 0;
  }
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void unlock(){
  
  do{
    successRead = getID();
  }
  while(!successRead);
  successRead = false;
  if(((uint8_t)readCard[0] == (uint8_t)masterCard[0]) && ((uint8_t)readCard[1] == (uint8_t)masterCard[1]) && ((uint8_t)readCard[2] == (uint8_t)masterCard[2]) && ((uint8_t)readCard[3] == (uint8_t)masterCard[3])){
    Serial.println("Master card detected");
    do {
      successRead = getID();
      for(int i=0; i<NUMPIXELS; i = i + 2){
        pixels.setPixelColor(i, pixels.Color(0,50,50));
        pixels.show();
      }
      delay(500);
      for(int i=0; i<NUMPIXELS; i = i + 2){
        pixels.setPixelColor(i, pixels.Color(0,50,0));
        pixels.show();
      }
      delay(500);
    }
    while(!successRead);
    successRead = false;
    if(((uint8_t)readCard[0] == (uint8_t)masterCard[0]) && ((uint8_t)readCard[1] == (uint8_t)masterCard[1]) && ((uint8_t)readCard[2] == (uint8_t)masterCard[2]) && ((uint8_t)readCard[3] == (uint8_t)masterCard[3])){
      Serial.println("Master card detected again");
      Serial.println("Next card scanned will be blacklisted");
      do {
        successRead = getID();
        for(int i=0; i<NUMPIXELS; i = i + 2){
          pixels.setPixelColor(i, pixels.Color(0,50,50));
          pixels.show();
        }
        delay(500);
        for(int i=0; i<NUMPIXELS; i = i + 2){
          pixels.setPixelColor(i, pixels.Color(50,0,0));
          pixels.show();
        }
        delay(500);
      }
      while(!successRead);
      Serial.println("Card removed");
      for(int i=0; i<NUMPIXELS; i = i + 2){
        pixels.setPixelColor(i, pixels.Color(0,50,0));
        pixels.show();
      }
      delay(500);
      for(int i=0; i<NUMPIXELS; i = i + 2){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
      }
    }
    else{
      Serial.println("Card whitelisted");
      for(int i=0; i<NUMPIXELS; i = i + 2){
        pixels.setPixelColor(i, pixels.Color(0,50,0));
        pixels.show();
      }
      delay(500);
      for(int i=0; i<NUMPIXELS; i = i + 2){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
      }
    }
  }
  
  //unlock animation
  digitalWrite(D3, HIGH);
  for(int fade=0; fade<MAXVAL; fade++){
    for(int i=0; i<NUMPIXELS; i++){
      pixels.setPixelColor(i, pixels.Color(0,fade,0));
      pixels.show();
    }
    delay(FADEVAL);
  }
  
  delay(DELAYVAL * 7);
  for(int i=0; i<NUMPIXELS; i++){
    for(int fade=0; fade<MAXVAL; fade++){
      pixels.setPixelColor(i, pixels.Color(0,MAXVAL-fade,0));
      pixels.show();
      delay(FADEVAL);
    }
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();
  }
  digitalWrite(D3, LOW);
  //lock animation
  for(int fade=0; fade<MAXVAL; fade++){
    for(int i=0; i<NUMPIXELS; i++){
      pixels.setPixelColor(i, pixels.Color(fade,0,0));
      pixels.show();
    }
    delay(FADEVAL-3);
  }
  for(int fade=0; fade<MAXVAL; fade++){
    for(int i=0; i<NUMPIXELS; i++){
      pixels.setPixelColor(i, pixels.Color(MAXVAL-fade,0,0));
      pixels.show();
    }
    delay(FADEVAL-3);
  }
  for(int i=0; i<NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();
  }
}
byte buffer[18];
byte Card[16][4];

void resetInfo(){
	for (int i=0; i<=15; i++){
		for (int j=0; j<=4; j++){
			Card[i][j]=0;
		}
	}
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}



// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.


  rfid_tag_present_prev = rfid_tag_present;

  _rfid_error_counter += 1;
  if(_rfid_error_counter > 2){
    _tag_found = false;
  }

  // Detect Tag without looking for collisions
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Reset baud rates
  mfrc522.PCD_WriteRegister(mfrc522.TxModeReg, 0x00);
  mfrc522.PCD_WriteRegister(mfrc522.RxModeReg, 0x00);
  // Reset ModWidthReg
  mfrc522.PCD_WriteRegister(mfrc522.ModWidthReg, 0x26);


  if(mfrc522.PICC_RequestA(bufferATQA, &bufferSize) == mfrc522.STATUS_OK){
    if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue   
      return;
    }
    _rfid_error_counter = 0;
    _tag_found = true;        
  }
  
  rfid_tag_present = _tag_found;
  
  // rising edge
  if (rfid_tag_present && !rfid_tag_present_prev){
    Serial.println("Tag found");
    for(byte page=0; page <=15; page+=4){
      byte byteCount = sizeof(buffer);
      if(mfrc522.MIFARE_Read(page,buffer,&byteCount) == !mfrc522.STATUS_OK){
        Serial.println("MIFARE_Read() failed");
        return;
      }
      int i_=0;
      for (int i=page; i<=page+3; i++){
        for (int j=0; j<=3; j++){
          Card[i][j]=buffer[4*i_ + j];
        }
        i_++;
      }
    }
    mfrc522.PICC_HaltA();
    Serial.println("--------------------------");
    for (int i=0; i<16; i++){
      for (int j=0; j<4; j++){
        Serial.print(Card[i][j],HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    Serial.println("--------------------------");
    unlock();
  }
  
  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev){
    Serial.println("Tag gone");
  }
}