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

  // LED initialization
  strip = new LedStrip(ledCtrlPin);
  strip -> Initialize();
}

void loop(){

	strip->update();

}






