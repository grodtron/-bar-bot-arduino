#include "led.h"

#include "hsv.h"

LedStrip::LedStrip(uint8_t _pin)
: pin(_pin)
, adaStrip(30, pin, NEO_GRB + NEO_KHZ800)
, waitingForTime(0){
	adaStrip.begin();

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


void LedStrip::setLed(Adafruit_NeoPixel & adaStrip, uint8_t column, uint8_t led, uint32_t color){

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


void LedStrip::setPattern(LedStrip::PatternType p){
	waitingForTime = 0;
	start = 0;
	pattern = p;
}

void LedStrip::update(){
	switch (pattern){
	case LedStrip::Rotating:
		updateRotating();
		return;
	case LedStrip::Pouring:
		updatePouring();
		return;
	case LedStrip::Idle:
		updateIdle();
		return;
	}
}

void LedStrip::updateIdle(){

	if (millis() > waitingForTime){
		HsvColor hsv = { start, 255, 255 };
		
		for (int i = 0; i < 5; ++i) {
			RgbColor rgb = HsvToRgb(hsv);

			for (int j = 0; j < 6; ++j){
				LedStrip::setLed(adaStrip, j, i, Adafruit_NeoPixel::Color(rgb.r, rgb.g, rgb.b));
			}

			hsv.h += 51;
		}

		start += 100;
		waitingForTime = millis() + 250;
	}

	adaStrip.show();
}


void LedStrip::updateRotating() {
	if (millis() > waitingForTime){
		HsvColor hsv = { start, 255, 255 };

		for (int i = 0; i < 5; ++i) {
			RgbColor rgb = HsvToRgb(hsv);

			for (int j = 0; j < 6; ++j){
				LedStrip::setLed(adaStrip, i, j, Adafruit_NeoPixel::Color(rgb.r, rgb.g, rgb.b));
			}

			hsv.h += 128;// 51;
		}

		waitingForTime = millis() + 250;
		start += 51;
	}

	adaStrip.show();

}


void LedStrip::updatePouring() {
	if (millis() > waitingForTime){
		HsvColor hsv = { start, 255, 255 };

		for (int i = 0; i < 5; ++i) {
			RgbColor rgb = HsvToRgb(hsv);

			for (int j = 0; j < 6; ++j){
				LedStrip::setLed(adaStrip, i, j, Adafruit_NeoPixel::Color(rgb.r, rgb.g, rgb.b));
			}

			hsv.h += 51;
		}

		waitingForTime = millis() + 100;
		start += 51;
	}

	adaStrip.show();

}