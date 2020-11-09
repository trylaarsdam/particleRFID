/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "d:/Dev/rfid/src/rfid.ino"
void setup();
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

#define LED_PIN D6
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

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  pinMode(D3, OUTPUT);
  Serial.begin(9600);
  while(!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  pixels.begin();
  for(int i=0; i<NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();    
  }
}

void unlock(){
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

bool MIFARE_SetKeys(MFRC522::MIFARE_Key* oldKeyA, MFRC522::MIFARE_Key* oldKeyB,
                    MFRC522::MIFARE_Key* newKeyA, MFRC522::MIFARE_Key* newKeyB,
                    int sector) {
  MFRC522::StatusCode status;
  byte trailerBlock = sector * 4 + 3;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, oldKeyA, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  // Show the whole sector as it currently is
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), oldKeyA, sector);
  Serial.println();

  // Read data from the block
  Serial.print(F("Reading data from block ")); Serial.print(trailerBlock);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(trailerBlock, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  Serial.print(F("Data in block ")); Serial.print(trailerBlock); Serial.println(F(":"));
  dump_byte_array(buffer, 16); Serial.println();
  Serial.println();

  // Authenticate using key B
  Serial.println(F("Authenticating again using key B..."));
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, oldKeyB, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  if (newKeyA != nullptr || newKeyB != nullptr) {
    for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
      if (newKeyA != nullptr) {
        buffer[i] = newKeyA->keyByte[i];
      }
      if (newKeyB != nullptr) {
        buffer[i+10] = newKeyB->keyByte[i];
      }
    }
  }

  // Write data to the block
  Serial.print(F("Writing data into block ")); Serial.print(trailerBlock);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(trailerBlock, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  Serial.println();

  // Read data from the block (again, should now be what we have written)
  Serial.print(F("Reading data from block ")); Serial.print(trailerBlock);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(trailerBlock, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block ")); Serial.print(trailerBlock); Serial.println(F(":"));
  dump_byte_array(buffer, 16); Serial.println();
  return true;
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