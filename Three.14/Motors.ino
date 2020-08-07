/*
 * Three.14
 * 
 * Motors.ino - holds driving/motor functions
 * 
 * Copyright © 2019 Del Rudolph, <del@darthrudolph.com> https://www.darthrudolph.com/
 * 
 */

void doWalkies(){
  if(theMotor == 0){
    // driving the left motor
    // if a full step is made
    if(encoders.getCountsLeft() >= theSteps[0]){
      motors.setLeftSpeed(0);
      encoders.getCountsAndResetLeft();
      stepCount++;
      theMotor = 1;
    }else{
      motors.setLeftSpeed(theSpeed);
    }

  }else if(theMotor == 1){
    // driving the right motor
    // if a full step is made
    if(abs(encoders.getCountsRight()) >= theSteps[0]){
      motors.setRightSpeed(0);
      encoders.getCountsAndResetRight();
      stepCount++;
      theMotor = 0;
    }else{
      motors.setRightSpeed(-theSpeed);
    }

  }else{
    // should never happen
    allStop();
    theMotor = 0;
    theState = 255;
  }
}

void doSpin(int16_t whichStep){
  int16_t stepToDo;
  
  if(whichStep < theStepsCount){
    stepToDo = theSteps[whichStep];
  }else{
    stepToDo = whichStep;
  }

  if((encoders.getCountsLeft() < stepToDo) and 
     (abs(encoders.getCountsRight()) < stepToDo)
    ){
    motors.setSpeeds(theSpeed, -theSpeed);
  }else{
    allStop();
    // let the Pi know it's been done
    theState = 0;
  }
}

void runMotorsForTicks(int16_t doTicks){
  if((abs(encoders.getCountsLeft()) < doTicks) and 
     (abs(encoders.getCountsRight()) < doTicks)
    ){
    if(doTicks < 0){
      motors.setSpeeds(-theSpeed, -theSpeed);
    }else{
      motors.setSpeeds(theSpeed, theSpeed);
    }
  }else{
    allStop();
    theState = 0;
  }
}

void allStop(){
  motors.setSpeeds(0,0);
  encoders.getCountsAndResetRight();
  encoders.getCountsAndResetLeft();
}
