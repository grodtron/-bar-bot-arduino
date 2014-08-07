#include "led.h"

#include "hsv.h"

LedStrip::LedStrip(uint8_t _pin)
: pin(_pin)
, adaStrip(30, pin, NEO_GRB + NEO_KHZ800)
, waitingForTime(0)
, pattern(LedStrip::Idle)
, start(0)
, duration(0)
, remaining(0)
, index(0) {
	adaStrip.begin();
	adaStrip.setBrightness(190);

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


void LedStrip::setPattern(LedStrip::PatternType p, unsigned long duration){
	this->waitingForTime = 0;
	this->start = 0;
	this->duration = duration;
	this->remaining = duration;
	this->pattern = p;
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
	case LedStrip::Danger:
		updateDanger();
		return;
	}
}


// sin lookup table to avoid slow-as-balls floating point math
static uint8_t sin_tab[256] = {
	0, 3, 6, 9, 12, 15, 18, 21, 25, 28, 31, 34, 37, 40, 43, 46, 49, 53, 56, 59, 62, 65, 68, 71, 74,
	77, 80, 83, 86, 89, 92, 95, 97, 100, 103, 106, 109, 112, 115, 117, 120, 123, 126, 128, 131, 134,
	136, 139, 142, 144, 147, 149, 152, 155, 157, 159, 162, 164, 167, 169, 171, 174, 176, 178, 181,
	183, 185, 187, 189, 191, 193, 195, 197, 199, 201, 203, 205, 207, 209, 211, 212, 214, 216, 217,
	219, 221, 222, 224, 225, 227, 228, 230, 231, 232, 234, 235, 236, 237, 238, 239, 241, 242, 243,
	244, 244, 245, 246, 247, 248, 249, 249, 250, 251, 251, 252, 252, 253, 253, 254, 254, 254, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253,
	252, 252, 251, 251, 250, 249, 249, 248, 247, 246, 245, 244, 244, 243, 242, 241, 239, 238, 237,
	236, 235, 234, 232, 231, 230, 228, 227, 225, 224, 222, 221, 219, 217, 216, 214, 212, 211, 209,
	207, 205, 203, 201, 199, 197, 195, 193, 191, 189, 187, 185, 183, 181, 178, 176, 174, 171, 169,
	167, 164, 162, 159, 157, 155, 152, 149, 147, 144, 142, 139, 136, 134, 131, 128, 126, 123, 120,
	117, 115, 112, 109, 106, 103, 100, 97, 95, 92, 89, 86, 83, 80, 77, 74, 71, 68, 65, 62, 59, 56,
	53, 49, 46, 43, 40, 37, 34, 31, 28, 25, 21, 18, 15, 12, 9, 6, 3 };


void LedStrip::updateDanger(){
	if (millis() > waitingForTime){
		for (int i = 0; i < 6; ++i) {
			uint32_t color = Adafruit_NeoPixel::Color(0xff, 0, 0);

			for (int j = 0; j < 5; ++j){
				LedStrip::setLed(adaStrip, i, j, color);
			}

		}
		waitingForTime = millis() + 200;
		adaStrip.show();
	}
}

void LedStrip::updateIdle(){	
	if (millis() > waitingForTime){
		
		for (int i = 0; i < 6; ++i) {
			HsvColor hsv = { random() & 0xff, 255, 255 };
			RgbColor rgb = HsvToRgb(hsv);
			uint32_t color = Adafruit_NeoPixel::Color(rgb.r, rgb.g, rgb.b);

			for (int j = 0; j < 5; ++j){
				LedStrip::setLed(adaStrip, i, j, color);
			}

		}
		waitingForTime = millis() + 200;
		adaStrip.show();
	}
}

static uint32_t colors[6] = {

	Adafruit_NeoPixel::Color(0xff, 0x33, 0x00),
	Adafruit_NeoPixel::Color(0x00, 0xff, 0x33),
	Adafruit_NeoPixel::Color(0x33, 0x00, 0xff),

	Adafruit_NeoPixel::Color(0x33, 0xff, 0x00),
	Adafruit_NeoPixel::Color(0x00, 0x33, 0xff),
	Adafruit_NeoPixel::Color(0xff, 0x00, 0x33)

};

void LedStrip::setIndex(uint8_t i) {
	index = i;
}

void LedStrip::updateRotating() {
	
	if (millis() > waitingForTime){
		uint32_t color = colors[index];
	
		for (int i = 0; i < 30; ++i){
			adaStrip.setPixelColor(i, color);
		}

		waitingForTime = ~((uint32_t)0); // maximum

		adaStrip.show();

	}
}

void LedStrip::updatePouring() {

	if (millis() > waitingForTime){
		unsigned long delta = max(10, remaining / 100);

		waitingForTime  = millis() + delta;
		if (remaining >= delta){
			remaining -= delta;
		}
		else{
			remaining = 0;
		}

		unsigned long percent = (100 * remaining) / duration;

		uint8_t values[5] = {
			percent > 100 ? 255 : (percent < 80 ? 0 : (255 * (percent-80))/20),
			percent >  80 ? 255 : (percent < 60 ? 0 : (255 * (percent-60))/20),
			percent >  60 ? 255 : (percent < 40 ? 0 : (255 * (percent-40))/20),
			percent >  40 ? 255 : (percent < 20 ? 0 : (255 * (percent-20))/20),
			255,//percent >  20 ? 255 : (percent <  0 ? 0 : (255 * (percent- 0))/20),
		};

		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 6; ++j){
				HsvColor hsv = { random() & 0xff, 255, values[4 - i] };
				RgbColor rgb = HsvToRgb(hsv);

				LedStrip::setLed(adaStrip, j, i, Adafruit_NeoPixel::Color( rgb.r, rgb.g, rgb.b ) );
			}
		}

		adaStrip.show();
	}
	if (remaining == 0){
		setPattern(Idle);
	}
}


#if 0
void LedStrip::starwipe(){
	if (millis() > waitingForTime){

		uint8_t colors[5];
		colors[0] = 255 - sin_tab[(start + 0) % 256];
		colors[1] = 255 - sin_tab[(start + 70) % 256];
		colors[2] = 255 - sin_tab[(start + 140) % 256];
		colors[3] = 255 - sin_tab[(start + 210) % 256];
		colors[4] = 255 - sin_tab[(start + 280) % 256];
		start += 15;
		waitingForTime = millis() + 30;

		for (int i = 0; i < 5; ++i) {
			uint32_t color = Adafruit_NeoPixel::Color(colors[i], colors[i], colors[i]);
			for (int j = 0; j < 6; ++j){
				LedStrip::setLed(adaStrip, j, i, color);
			}
		}

		adaStrip.show();

	}
}
#endif