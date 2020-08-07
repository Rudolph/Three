/*
 * Three.14
 * 
 * Sensors.ino - holds sensory/input functions
 * 
 * Copyright © 2019 Del Rudolph, <del@darthrudolph.com> https://www.darthrudolph.com/
 * 
 */

void doWhiskers(){
  if(leftWhisker.getSingleDebouncedPress() or rightWhisker.getSingleDebouncedPress()){
    doGrumble();
    lastBump = millis();
  }
  
  if(leftWhisker.isPressed() and rightWhisker.isPressed()){
    allStop();
    theState = 200;     // alert the Pi that both whiskers are triggered
    
  }else if(rightWhisker.isPressed() && theState == 1 && theMotor == 0){
    allStop();
    theMotor = 1;
    
  }else if(leftWhisker.isPressed() && theState == 1 && theMotor == 1){
    allStop();
    theMotor = 0;
    
  }else if((theState == 2) and (rightWhisker.isPressed() or leftWhisker.isPressed())){
    allStop();
    theState = 202;
  }
}
