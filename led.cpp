#include "led.h"

#include "hsv.h"

LedStrip::LedStrip(uint8_t _pin): pin(_pin), adaStrip(30, pin, NEO_GRB + NEO_KHZ800){}

void LedStrip::Initialize(){
  adaStrip.begin();
  // Initialize all pixels to 'off'
  waitingForTime = millis();
}

void LedStrip::SetBottle(uint8_t bottleNum, uint8_t displayMode, uint8_t color){
 
}


static uint32_t getRandomColor(){
	uint8_t a = random() & 0xff;
	uint8_t b = 255 - a;

	switch (random() % 3){
	case 0: return Adafruit_NeoPixel::Color(a, b, 0);
	case 1: return Adafruit_NeoPixel::Color(a, 0, b);
	case 2: return Adafruit_NeoPixel::Color(0, a, b);
	}
}


void LedStrip::setLed(uint8_t column, uint8_t led, uint32_t color){

	uint8_t i = column * 5;

	if (column % 2) {
		i += 4;

		i -= led;
	}
	else{
		i += led;
	}

	adaStrip.setPixelColor(i, color);
}


void LedStrip::update(){

	static uint8_t start = 0;

	if (millis() > waitingForTime){
		HsvColor hsv = { start, 255, 255 };
		
		for (int i = 0; i < 6; ++i) {
			RgbColor rgb = HsvToRgb(hsv);

			for (int j = 0; j < 5; ++j){
				setLed(i, j, Adafruit_NeoPixel::Color(rgb.r, rgb.g, rgb.b));
			}

			//hsv.h += 51;
		}

		start += 1;
		waitingForTime = millis() + 250;
	}

	adaStrip.show();
}

