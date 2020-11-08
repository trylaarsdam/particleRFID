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

#define RST_PIN         9         
#define SS_PIN          10

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
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

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  unlock();
  delay(5000);
}