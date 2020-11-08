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
}
byte buffer[18];
byte Card[16][4];

void ResetInfo(){
	for (int i=0; i<=15; i++){
		for (int j=0; j<=4; j++){
			Card[i][j]=0;
		}
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
    unlock();
  }
  
  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev){
    Serial.println("Tag gone");
  }
}