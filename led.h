	#ifndef LED_H
#define LED_H
#include <Adafruit_NeoPixel.h>

enum LEDIdleMode{Spiral, Eruption, Snake, UrMother};
enum LEDDisplayMode{Ramp, Solid, Blink};
enum LEDColor{Rainbow, White, Bottle, Off};

class LedStrip {
public:
	enum PatternType {
		Pouring, Rotating, Idle
	};

private:

	uint8_t pin;
	Adafruit_NeoPixel adaStrip;
	
	static void setLed(Adafruit_NeoPixel & adaStrip, uint8_t column, uint8_t led, uint32_t color);

	PatternType pattern;

	void updateRotating();
	void updatePouring();
	void updateIdle();

	unsigned long waitingForTime;
	uint8_t start;

  public:

	void setPattern(LedStrip::PatternType pattern);

	LedStrip(uint8_t _pin);

	void update();


};

#endif