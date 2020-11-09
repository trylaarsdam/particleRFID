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
boolean match = false;
uint8_t successRead;

MFRC522::MIFARE_Key key;
MFRC522::MIFARE_Key newKeyA = {keyByte: {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}};
MFRC522::MIFARE_Key newKeyB = {keyByte: {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5}};

byte readCard[4];
byte masterCard[4];
byte storedCard[4];   // Stores an ID read from EEPROM


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

void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != 0 )      // Make sure there is something in the array first
    match = true;       // Assume they match at first
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] )     // IF a != b then set match = false, one fails, all fail
      match = false;
  }
  if ( match ) {      // Check to see if if match is still true
    return true;      // Return true
  }
  else  {
    return false;       // Return false
  }
}

boolean findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
      break;  // Stop looking we found it
    }
    else {    // If not, return false
    }
  }
  return false;
}

void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    Serial.println(F("Succesfully added ID record to EEPROM"));
  }
  else {
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
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
  else{
    if(findID(readCard)){
      Serial.println("Card Known");
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
  do {
    successRead = getID();
  }
  while(!successRead);
  successRead = false;
  unlock();
}