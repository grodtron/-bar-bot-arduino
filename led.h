	#ifndef LED_H
#define LED_H
#include <Adafruit_NeoPixel.h>

enum LEDIdleMode{Spiral, Eruption, Snake, UrMother};
enum LEDDisplayMode{Ramp, Solid, Blink};
enum LEDColor{Rainbow, White, Bottle, Off};

class LedStrip {
public:
	enum PatternType {
		Pouring, Rotating, Idle, Danger
	};

private:

	uint8_t pin;
	Adafruit_NeoPixel adaStrip;
	
	static void setLed(Adafruit_NeoPixel & adaStrip, uint8_t column, uint8_t led, uint32_t color);

	PatternType pattern;

	void updateRotating();
	void updatePouring();
	void updateIdle();
	void updateDanger();

	unsigned long waitingForTime;
	uint8_t start;
	unsigned long duration;
	unsigned long remaining;
	uint8_t index;

  public:

	void setIndex(uint8_t i);
	void setPattern(LedStrip::PatternType pattern, unsigned long duration=0);

	LedStrip(uint8_t _pin);

	void update();


};

#endif