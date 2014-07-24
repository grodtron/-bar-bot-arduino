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

StepperMotor::Direction hookPullDirection    = StepperMotor::CounterClockwise;
StepperMotor::Direction hookReleaseDirection = StepperMotor::Clockwise;

uint32_t hookPullAmount = 1550;

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
//LedStrip* strip;

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

  Serial.begin(9600);           
  Serial.println("Connected");

  pinMode(i2c_addr_pin, INPUT);

  //I2C initialization
  // Set i2c address based on which slot we're in
  if (digitalRead(i2c_addr_pin)){
	  Wire.begin(0x11);
  }
  else{
	  Wire.begin(0x22);
  }

  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  pinMode(spireLimitPin, INPUT);
  pinMode(hookLimitPin,  INPUT);
  digitalWrite(spireLimitPin, HIGH);
  digitalWrite(hookLimitPin, HIGH);

  //LED initialization
  //strip = new LedStrip(ledCtrlPin);
  //strip -> Initialize();
  }

void loop(){
	
	if(commandFlag){
		Serial.println("In handling read...");
		
		//parse according to communication type
		switch(currentCommand){
		case PouringRequest: 
			{
			Serial.println("In PouringRequest..."); 
			uint8_t x=1;
			uint8_t cmpCheckSum=0;
			uint8_t numBottles, dispenseType, pourAmount, rotation, ledMode, ledColor, checkSum;
			numBottles = msgBuffer[x++];
					Serial.print("bottleNum: "); 
					Serial.println(numBottles);
			dispenseType = msgBuffer[x++];
					Serial.print("dispenseType: "); 
					Serial.println(dispenseType);
			pourAmount = msgBuffer[x++];
					Serial.print("pourAmount: "); 
					Serial.println(pourAmount);
			rotation = msgBuffer[x++];
					Serial.print("rotation: ");
					Serial.println(rotation);
			ledMode = msgBuffer[x++];
					Serial.print("ledMode: "); 
					Serial.println(ledMode);
			ledColor = msgBuffer[x++];
					Serial.print("ledColor: "); 
					Serial.println(ledColor);
			checkSum = msgBuffer[x++];
					Serial.print("checkSum: "); 
					Serial.println(checkSum);

			uint8_t csm = numBottles ^ dispenseType ^ pourAmount ^ rotation ^ ledMode ^ ledColor;

			Serial.print("Computed checksum: ");
			Serial.println(csm);
			Serial.print("Received checksum: ");
			Serial.println(checkSum);
			Serial.print("badChecksum flag = ");
			Serial.println(badChecksum);

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
						delay(3500);
						hookStepper->SetMotor(
							StepperMotor::Slow,
							hookPullAmount + 100,
							hookReleaseDirection,
							hookLimitPin,
							1);
						delay(200);
					}
				}
				if (dispenseType == FreePour){
					hookStepper->SetMotor(
						StepperMotor::Slow,
						hookPullAmount,
						hookPullDirection);

					// TODO - configure this value (or send it from the rpi?)
					delay(3500*pourAmount);
					
					hookStepper->SetMotor(
						StepperMotor::Slow,
						hookPullAmount + 100,
						hookReleaseDirection,
						hookLimitPin,
						1);
				}
			}



			status = Idle;
			break;
			}
		case JogRequest: 
			{
			Serial.println("In JogRequest..."); 
			uint8_t x=1;
			bool badFlag;
			uint8_t cmpCheckSum=0;
			uint8_t motor, direction, nSteps, checkSum;
			motor = msgBuffer[x++];
					Serial.print("motor: "); 
					Serial.println(motor);
			direction = msgBuffer[x++];
					Serial.print("direction: "); 
					Serial.println(direction);
			nSteps = msgBuffer[x++];
					Serial.print("nSteps: "); 
					Serial.println(nSteps);
			checkSum = msgBuffer[x++];
					Serial.print("checkSum: "); 
					Serial.println(checkSum);

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

			status = Idle;
			break;
			}
		case IdleRequest: 
			Serial.println("In IdleRequest..."); 
			break;
		case CommandAckRequest:
			Serial.println("In CommandAckRequest..."); 
			break;
		case FaultRequest:
			Serial.println("In FaultRequest..."); 
			break;
		default: 
			Serial.println("In default..."); 
			break;
		}

		commandFlag = false;
	}
	else{
	}

	delay(20);
}

void receiveEvent(int numBytes){
	int command;
	if (Wire.available()){
		command = Wire.read();

		// Ignore a command if we haven't dealt with the previous one yet.
		if (command < CommandAckRequest && !commandFlag){
			currentCommand = (CommunicationType)command;
			msgBuffer[0] = command;

			uint8_t computedCsum = command;
			uint8_t i = 1;
			while (Wire.available()){
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
		// Ignore a command if we haven't dealt with the previous one yet
		else if (command >= CommandAckRequest && !requestFlag){
			currentRequest = (CommunicationType)command;
			requestFlag = true;
		}

		else{
			// CONFUSION
		}
	}
}

void requestEvent(){

	if(requestFlag){ //redundant check

		switch(currentRequest){
		case CommandAckRequest:
			if(badChecksum){
				Wire.write(0);
			}
			else{
				timeRemaining = 255; // testing (no overflow please!)
				Wire.write(timeRemaining); // TODO - do we care?
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
		requestFlag = false;
	}
	else{ // Client should retry request after a delay
		Wire.write(InternalError);
	}
}






