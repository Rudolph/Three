/*
 * Three.14
 * 
 * Copyright © 2019-2020 Del Rudolph, <del@darthrudolph.com> https://www.darthrudolph.com/
 * 
 */

#include <Romi32U4.h>
#include <PololuRPiSlave.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

void playSounds(uint8_t ceasePlaying = 0);

#define rWhisker 12     // pin 12, button on Three's right side, white wire
#define lWhisker 5      // pin 5, button on Three's left side, brown wire

#define theEye A2       // LDR eye, on Analog 2

#define ledPin 11       // pin holding the RGB LED ("NeoPixel")
#define ledCount 1      // how many LEDs connected

Adafruit_NeoPixel moodLight = Adafruit_NeoPixel(ledCount, ledPin, NEO_GRB + NEO_KHZ800);

// how many encoder ticks for a given rotational step
const uint16_t theSteps[] = {
  3065,     // one full 'bot circumference, or one of Three's "footsteps"
  1533,     // half step       1532.734 -- *2=3066    off by 1
  766,      // quarter step    766.367  -- *4=3064    off by 1
  383,      // eighth step     383.184  -- *8=3064    off by 1
  307,      // tenth step      306.547  -- *10=3070   off by 5
  255,      // twelfth step    255.456  -- *12=3060   off by 5
  192       // sixteenth step  191.592  -- *16=3072   off by 7
};

// how many entries in the above array
const uint8_t theStepsCount = 7;

struct Data
{
  // RGB values and brightness from the Pi
  // 4 bytes
  uint8_t red, green, blue, bright;
  
  // theCommand and commandValue from the Pi
  // 3 bytes
  uint8_t theCommand;       // 1 byte
  int16_t commandValue;    // 2 bytes

  // individual motor speeds from the Pi for use when theCommand == 2
  // 4 bytes
  int16_t leftMotor, rightMotor;

  // to the Pi
  // 2 bytes
  uint16_t batteryMillivolts;

  //uint16_t analog[6]; // 12 bytes 
  // replaced by lastBump and the LDR eye's readings and the values of 
  // A0, A3, A4 analog pins to the Pi
  // (A1 is batteryMillivolts, A2 is the theEye)
  // 12 bytes
  uint32_t lastBump;        // 4 bytes
  uint16_t eyeLight;        // 2 bytes
  uint16_t analogPins[3];   // 6 bytes

  // from the Pi
  // 15 bytes
  bool playNotes;
  char notes[14];

  // to the Pi
  // 4 bytes
  int16_t leftEncoder, rightEncoder;

  // to the Pi
  // 2 bytes
  uint8_t stepCount, theState;
};

// set up the things
PololuRPiSlave<struct Data,5> stem;
PololuBuzzer buzzer;
Romi32U4Motors motors;
Romi32U4Encoders encoders;
Romi32U4ButtonA buttonA;
Romi32U4ButtonC buttonC;
Pushbutton leftWhisker(lWhisker);         // feelers are named after the side of the 'bot they're
Pushbutton rightWhisker(rWhisker);        // on, not the motor they protect

// some vars to hold things
uint8_t theState = 0;       // current state of the state machine
uint8_t theCommand = 0;     // command from the Pi
uint8_t stepCount = 0;      // how many steps have been taken without being reset
uint8_t theMotor = 0;       // 0 = Left motor, 1 = Right motor

int16_t theSpeed = 100;     // starting motor speed

uint32_t lastBump = 0;      // millis since the last time a feeler was triggered

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  randomSeed(analogRead(theEye));

  // Set up the I2C slave at address 42.
  stem.init(42);

  while(1){
    // wait for a feeler to trigger
    if((leftWhisker.getSingleDebouncedRelease()) or (rightWhisker.getSingleDebouncedRelease())){
      // Play startup sound.
      buzzer.play("v10>>g16>>>c16");
      delay(3000);
      break;
    }
  }

  lastBump == millis();
  moodLight.begin();
  moodLight.setBrightness(50);
  moodLight.show();
  Serial.println("one, two... Three!");
}

void loop() {
  doHeartBeat();
  doWhiskers();
  
  // Call updateBuffer() before using the buffer, to get the latest
  // data including recent master writes.
  stem.updateBuffer();

  // see if the Pi has sent a new command
  if(theCommand != stem.buffer.theCommand){
    Serial.println("theCommand != theCommand");
    theState = stem.buffer.theCommand;
    theCommand = stem.buffer.theCommand;
  }

  // the state machine that runs the 'bot
  switch(theState){
    case 0:
      // just started or idle
      // wait for the Pi to issue a command
      //theState = 1;     // FIXME - remove this when Pi is ready to go. The stem should
                        // not be able to make these kinds of decisions on its own

      break;

    case 1:
      // walking
      doWalkies();

      break;

    case 2:
      // rotate in place one of theSteps[0-6] or, if commandValue > theStepsCount,
      // rotate for commandValue encoder counts
      doSpin(stem.buffer.commandValue);

      break;

    case 3:
      // set general motor speed
      theSpeed = constrain(stem.buffer.commandValue, 75, 300);

      // let the Pi know it's been done
      theState = 0;

      break;

    case 4:
      // direct motor drive from the Pi
      // FIXME utilize encoders to ensure both motors are turning the same amount
      motors.setSpeeds(stem.buffer.leftMotor, stem.buffer.rightMotor);

      // let the Pi know it's been done
      theState = 0;

      break;

    case 5:
      // drive  for commandValue encoder ticks at theSpeed
      runMotorsForTicks(stem.buffer.commandValue);
      
      break;

    case 6:
      // 
      
      break;

    default:
      // generic error
      allStop();
      
      break;
  }

  playSounds();
  
  showMood(stem.buffer.red, stem.buffer.green, stem.buffer.blue, stem.buffer.bright);

  // check if ButtonA has been pressed
  // if so, override theState to tell the Pi to shut down
  if((buttonA.getSingleDebouncedRelease()) or (buttonC.getSingleDebouncedRelease())){
    theState = 100;
  }

  // apprise the Pi of the current state of things
  stem.buffer.theState = theState;
  stem.buffer.batteryMillivolts = readBatteryMillivolts();
  stem.buffer.lastBump = lastBump;
  stem.buffer.eyeLight = analogRead(theEye);  // A2
  stem.buffer.analogPins[0] = analogRead(A0);
  stem.buffer.analogPins[1] = analogRead(A3);
  stem.buffer.analogPins[2] = analogRead(A4);
  stem.buffer.leftEncoder = encoders.getCountsLeft();
  stem.buffer.rightEncoder = encoders.getCountsRight();
  stem.buffer.stepCount = stepCount;

  // When you are done WRITING, call finalizeWrites() to make modified
  // data available to I2C master.
  stem.finalizeWrites();
}
