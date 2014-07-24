#ifndef LED_H
#define LED_H
#include <Adafruit_NeoPixel.h>

enum LEDIdleMode{Spiral, Eruption, Snake, UrMother};
enum LEDDisplayMode{Ramp, Solid, Blink};
enum LEDColor{Rainbow, White, Bottle, Off};

class LedStrip {
  private:

	uint8_t pin;
	Adafruit_NeoPixel adaStrip;
	
  public:
	LedStrip(uint8_t _pin);

	uint32_t UpdateTime;
	uint32_t BottleColors[6];
   	void Initialize();
	void SetIdle(uint8_t mode);
	void SetBottle(uint8_t bottleNum, uint8_t mode, uint8_t color);


};

#endif