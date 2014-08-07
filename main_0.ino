#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "motor.h"
#include "led.h"

#define MSGSIZE 32
#define BOTTLESTEP_AMT 3333 //todo: change with calibration


enum CommunicationType{PouringRequest = 0, IdleRequest = 1, JogRequest = 2, CommandAckRequest = 30, FaultRequest = 31};

enum GlobalResponseCodes{UnknownRequest=254, InternalError=255};

//fault request responses
enum Status{Idle = 0, BottleAlignment = 1, HookAlignment = 2, Pouring = 3};

//pouring mode
enum DispenseType{Shot = 0, FreePour = 1, NoPour = 2};
enum Rotation{CW = 0, CCW = 1, NoRotation = 2};
enum MotorName{ Spire = 0, Hook = 1 };

StepperMotor::Direction hookPullDirection;
StepperMotor::Direction hookReleaseDirection;

uint32_t hookPullAmount;

bool commandFlag;
bool requestFlag;
bool badChecksum;
uint8_t timeRemaining;
uint8_t msgBuffer[MSGSIZE];
CommunicationType currentCommand;
CommunicationType currentRequest;

Status status;

StepperMotor* spireStepper;
StepperMotor* hookStepper;
LedStrip* strip;

//pin assignments
uint8_t hookEnable = 2;
uint8_t hookReset = 3;
uint8_t hookStep = 4;
uint8_t hookDir = 5;

uint8_t spireEnable = 6;
uint8_t spireReset = 7;
uint8_t spireStep = 8;
uint8_t spireDir = 9;

uint8_t hookLimitPin  = A1;
uint8_t spireLimitPin = A2;

uint8_t ledCtrlPin = 10;

const int i2c_addr_pin = A3;
static const uint32_t SHOT_POUR_WAIT_TIME_MICROS = 500 * 1000; // half a second

uint8_t interrupt_rx_csum = 0xff;

void setup(){
  commandFlag   = false;
  requestFlag   = false;
  badChecksum   = true;
  status        = Idle;
  timeRemaining = 0;

  // stepper motor initialization
  hookStepper = new StepperMotor(hookStep, hookDir, hookEnable, hookReset);
  hookStepper->Initialize();

  spireStepper = new StepperMotor(spireStep, spireDir, spireEnable, spireReset);
  spireStepper->Initialize();

  pinMode(i2c_addr_pin, INPUT);

  //I2C initialization
  // Set i2c address based on which slot we're in
  if (digitalRead(i2c_addr_pin)){
	  Wire.begin(0x11);
	  hookPullDirection = StepperMotor::CounterClockwise;
	  hookReleaseDirection = StepperMotor::Clockwise;
	  hookPullAmount = 1090;
  }
  else{
	  Wire.begin(0x22);
	  hookPullDirection = StepperMotor::Clockwise;
	  hookReleaseDirection = StepperMotor::CounterClockwise;
	  hookPullAmount = 1070;

  }

  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  pinMode(spireLimitPin, INPUT);
  pinMode(hookLimitPin,  INPUT);
  digitalWrite(spireLimitPin, HIGH);
  digitalWrite(hookLimitPin, HIGH);

  //LED initialization
  strip = new LedStrip(ledCtrlPin);

  }

void loop(){
	
	static int8_t index = 0;

	strip->update();

	if(commandFlag){
		//parse according to communication type
		switch(currentCommand){
		case PouringRequest: 
			{
			uint8_t x=1;
			uint8_t cmpCheckSum=0;
			uint8_t numBottles, dispenseType, pourAmount, rotation, ledMode, ledColor, checkSum;

			numBottles = msgBuffer[x++];
			dispenseType = msgBuffer[x++];
			pourAmount = msgBuffer[x++];
			rotation = msgBuffer[x++];
			ledMode = msgBuffer[x++];
			ledColor = msgBuffer[x++];
			checkSum = msgBuffer[x++];


			uint8_t csm = numBottles ^ dispenseType ^ pourAmount ^ rotation ^ ledMode ^ ledColor;

			strip->setPattern(LedStrip::Rotating);

			// Safety: Zero the hook (this should have no effect since the hook should always start zeroed.)
			hookStepper->SetMotor(
				StepperMotor::Slow,
				StepperMotor::StepsPerRotation / 6,
				hookReleaseDirection,
				hookLimitPin,
				1,
				false);

			// Spin to the right bottle.
			if (numBottles > 0 && (rotation == CW || rotation == CCW)){
				if (rotation == CW) {
					index = (index + numBottles) % 6;
				} else {
					index = (index - numBottles) % 6;
				}
				
				strip->setIndex(index);
				spireStepper->SetMotor(
						StepperMotor::MediumRamp,
						((((StepperMotor::StepsPerRotation * numBottles)/6) *5)/2)/* + 500*/, // 500 overshoot - 5/2 is gear ratio
						rotation==CW ? StepperMotor::Clockwise : StepperMotor::CounterClockwise,
						spireLimitPin,
						numBottles);
			}

			if (pourAmount > 0){
				if (dispenseType == Shot){

					for (int i = 0; i < pourAmount; ++i){
						hookStepper->SetMotor(
							StepperMotor::Slow,
							hookPullAmount,
							hookPullDirection);

						strip->setPattern(LedStrip::Pouring, 3000); // hook steps are at 1kHz -> each step is 1 ms

						StepperMotor::busyWaitMillis(3000); // update lights during this movement.

						strip->setPattern(LedStrip::Rotating);

						hookStepper->SetMotor(
							StepperMotor::Slow,
							hookPullAmount + 100,
							hookReleaseDirection,
							hookLimitPin,
							1);
						StepperMotor::busyWaitMillis(200);
					}
				}
				if (dispenseType == FreePour){

					hookStepper->SetMotor(
						StepperMotor::Slow,
						hookPullAmount,
						hookPullDirection);

					strip->setPattern(LedStrip::Pouring, 2000 * pourAmount ); // hook steps are at 1kHz -> each step is 1 ms

					// TODO - configure this value (or send it from the rpi?)
					StepperMotor::busyWaitMillis(2000 * pourAmount);

					strip->setPattern(LedStrip::Rotating);

					hookStepper->SetMotor(
						StepperMotor::Slow,
						hookPullAmount + 100,
						hookReleaseDirection,
						hookLimitPin,
						1);
				}
			}

			strip->setPattern(LedStrip::Idle);

			status = Idle;
			break;
			}
		case JogRequest: 
			{
			uint8_t x=1;
			bool badFlag;
			uint8_t cmpCheckSum=0;
			uint8_t motor, direction, nSteps, checkSum;
			motor = msgBuffer[x++];
			direction = msgBuffer[x++];
			nSteps = msgBuffer[x++];
			checkSum = msgBuffer[x++];

			StepperMotor::Direction motorDir;

			badFlag = false;

			switch (direction){
			case CW:
				motorDir = StepperMotor::Clockwise;
				break;
			case CCW:
				motorDir = StepperMotor::CounterClockwise;
				break;
			default:
				badFlag = true;
				break;
			}

			strip->setPattern(LedStrip::Rotating);
			if (!badFlag){
				switch (motor){
				case Spire:
					spireStepper->SetMotor(
						StepperMotor::Slow,
						nSteps * 5,
						motorDir);
					break;
				case Hook:
					hookStepper->SetMotor(
						StepperMotor::Slow,
						nSteps * 5,
						motorDir);
					break;
				}
			}
			strip->setPattern(LedStrip::Idle);

			status = Idle;
			break;
			}
		case IdleRequest: 
			break;
		case CommandAckRequest:
			break;
		case FaultRequest:
			break;
		default: 
			break;
		}

		commandFlag = false;
	}
	else{
	}
    delay(20);

}

void receiveEvent(int numBytes){
	if ( Wire.available() ){
		int command;
		command = Wire.read();

		if (command < CommandAckRequest){
			currentCommand = (CommunicationType)command;
			msgBuffer[0] = command;

			uint8_t computedCsum = command;
			uint8_t i = 1;
			while ( Wire.available() && (i < MSGSIZE) ){
				computedCsum ^= (msgBuffer[i] = Wire.read());
				i++;
			}

			// In computing the checksum we end up xor'ing the computed checksum with the
			// transmitted checksum. Therefore if the transmitted checksum is correct, we
			// should have zero as the result.
			if (computedCsum){
				badChecksum = true;
			}
			else{
				// Only set the command flag in the case of a good checksum
				commandFlag = true;
				badChecksum = false;

				if (currentCommand == PouringRequest || currentCommand == JogRequest){
					status = Pouring; // set this right away incase we get a status request from the raspberry right after
				}
			}
		}
		else {
			currentRequest = (CommunicationType)command;
			requestFlag = true;
		}

	}

	// MUST completely consume this buffer every time or else we will receive no subsequent
	// RX events and our state will essentially be locked forever. (see Wire.cpp:253)
	while (Wire.available()) Wire.read();

}

void requestEvent(){
	if(requestFlag){ //redundant check
		requestFlag = false;

		switch(currentRequest){
		case CommandAckRequest:
			if(badChecksum){
				Wire.write(0);
				badChecksum = false;
			}
			else{
				Wire.write(8); // TODO - do we care?
			}
			break;
		case FaultRequest:
			Wire.write(status);
			break;
		default:
			// Unknown request... client should retry after delay, either that
			// or there are bigger problems.
			Wire.write(UnknownRequest);
			break;
		}	
	}
	else{ // Client should retry request after a delay
		Wire.write(InternalError);
	}
}






