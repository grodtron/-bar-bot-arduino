#include "motor.h"

#include "led.h"

extern LedStrip* strip;


StepperMotor::StepperMotor (uint8_t _stepPin, uint8_t _dirPin,
				uint8_t _enablePin, uint8_t _resetPin ): 
					stepPin(_stepPin), dirPin(_dirPin),
						enablePin(_enablePin), resetPin(_resetPin) {}

void StepperMotor::Initialize()
{
	//pin directions
	 pinMode(stepPin, OUTPUT);
	 pinMode(dirPin, OUTPUT);
	 pinMode(resetPin, OUTPUT);
	 pinMode(enablePin, OUTPUT);
 
	 //initial pin states
	 digitalWrite(enablePin, HIGH); // default not enabled
	 digitalWrite(resetPin, HIGH);
}

/*

Set the motor to go `numSteps` steps in direction `dir` with speed `speed`.

If `limit_pin` is greater than -1, then after each step, checks whether the pin reads a zero. If it does, then the
limit has been hit and the function stops moving the motor and returns true.

If `limit_count` is greater than one (which is the default value) then the motor will rotate until it has tripped and reset the
limit `limit_count` times.

In both cases return true if the limit was tripped the specified number of times before `numSteps` had been stepped, otherwise return
false

*/
bool StepperMotor::SetMotor(StepSpeed speed, uint16_t numSteps, Direction dir, uint8_t limit_pin, uint8_t limit_count, bool ignore_starting_on_switch){


	 digitalWrite(enablePin, LOW); // enable it

	//delay in microseconds between step transitions - half the period
	 uint32_t startHalfPeriod = 0;
	 uint32_t endHalfPeriod   = 0;

	 uint32_t halfPeriod;

	//set direction
	digitalWrite(dirPin, dir);

	//set (half) period amount depending on speed
	switch(speed){
	case Slow:
		startHalfPeriod = 1000; // 500 Hz
		endHalfPeriod   = 1000; // 500 Hz
		break;
	case Medium:
		startHalfPeriod = 500; // 1 kHz
		endHalfPeriod   = 500; // 1 kHz
		break;
	case MediumRamp:
		startHalfPeriod = 500; // 1 kHz
		endHalfPeriod   = 300; // TODO kHz
		break;
	default:
		startHalfPeriod = 500; // 1 kHz
		endHalfPeriod   = 500; // 1 kHz
	}
	
	// We only start out waiting for it if it's not already pressed.
	bool waiting_for_limit = ignore_starting_on_switch ? (digitalRead(limit_pin) != 0) : true;

	halfPeriod = startHalfPeriod;

	for(int i = 0; i < numSteps; i++){

		uint32_t skip_limit = 0;

		digitalWrite(stepPin, LOW);
		busyWait(halfPeriod);

		digitalWrite(stepPin, HIGH);
		busyWait(halfPeriod);

		uint8_t accelerationFactor = 13;

		// Ramp up the speed every four steps by decreasing the total period by one
		// microsecond. This should take us from 1 to 3 kHz in around one second.
		// TODO - this will require a bunch of tweaking for sure.
		//
		// NB it's also a non-linear acceleration, since as we increase the speed we
		// also increase the rate at which we accelerate.
		if (!(i%accelerationFactor) && (i < accelerationFactor*(startHalfPeriod - endHalfPeriod)) && (halfPeriod > endHalfPeriod)){
			--halfPeriod;
		}
		
		if (!(i%accelerationFactor) && (numSteps - i) <= accelerationFactor*(startHalfPeriod - endHalfPeriod)){
			++halfPeriod;
		}
		
		// If we actually care about limit switches
		if(limit_pin >= 0){

			// check the current value of the limit switch
			uint8_t val = digitalRead(limit_pin);
			if(val == 0){
				// If it's triggered and we're waiting for it
				if(waiting_for_limit){
					// decrease the count of how many times we still need
					// to see it
					--limit_count;
					if(limit_count == 0){
						// If we've hit it enough times, return true
						return true;
					}else{
						// otherwise, stop waiting for the limit switch to be triggered
						waiting_for_limit = false;
					}
				}
				// If the limit switch is not currently pressed
			}else{
				// If we were waiting for it to become un-pressed, mark
				// that we are now waiting for it to be pressed again
				if(!waiting_for_limit){
					waiting_for_limit = true;
				}
			}
		}
	}

    digitalWrite(enablePin, HIGH); // disable again

	return false;

}

void StepperMotor::busyWait(uint32_t waitTime){
	uint32_t elapsed = 0;
	uint32_t prevTime = micros();
		
	//strip->update();

	while(elapsed < waitTime){
		uint32_t currTime = micros();
			
		if(currTime < prevTime){
			//handle counter reset ~ every 70 minutes
			elapsed = currTime + (MICROS_MAX - prevTime); 
		}
		else if (currTime == prevTime){
			//unlikely
			elapsed = 0;
		}
		else{
			//normal case
			elapsed = currTime - prevTime;	
		}
	    //todo: add small delay here to avoid spin?
	}
}







