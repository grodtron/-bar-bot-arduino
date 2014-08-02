#include "led.h"

LedStrip::LedStrip(uint8_t _pin): pin(_pin), adaStrip(30, pin, NEO_GRB + NEO_KHZ800){}

void LedStrip::Initialize(){
  adaStrip.begin();
  // Initialize all pixels to 'off'
  for (int i = 0; i < 30; ++i){
	  adaStrip.setPixelColor(i, 0xff, 0x33, 0x00);
  }
  adaStrip.show(); 
}

void LedStrip::SetBottle(uint8_t bottleNum, uint8_t displayMode, uint8_t color){
 
}


