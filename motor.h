#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>
#include <stdint.h>

#define MICROS_MAX 4200000000

class StepperMotor {

  private:
	//control pins
	uint8_t stepPin;
	uint8_t dirPin, enablePin, resetPin;
	uint32_t prevTime, currTime;

	void busyWait(uint32_t waitTime);

  public:

	enum Direction{ Clockwise, CounterClockwise };
	enum StepType{ Full, Half, Quarter, Eighth, Sixteenth };
	enum StepSpeed{ Slow, Medium, Fast };

	const static uint32_t StepsPerRotation = 5373;

	StepperMotor(uint8_t _stepPin, uint8_t _dirPin,
				uint8_t _enablePin, uint8_t _resetPin );
	void Initialize();
	bool SetMotor(StepSpeed speed, uint16_t numSteps, StepType sType, Direction dir, uint8_t limit_pin=-1, uint8_t count=1, bool ignore_starting_on_switch=true);

};

#endif
