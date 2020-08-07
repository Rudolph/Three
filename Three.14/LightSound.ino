/*
 * Three.14
 * 
 * LightSound.ino - holds functions for light and sound output
 * 
 * Copyright © 2019 Del Rudolph, <del@darthrudolph.com> https://www.darthrudolph.com/
 * 
 */

// make the red LED blink like a heartbeat
void doHeartBeat(){
  const uint16_t theRhythm[] = {100,100,100,700};
  static uint8_t rhythmIndex = 3;
  static uint32_t lastBeat = 0;
  static uint8_t ledOn = 1;

  if(millis() - lastBeat < theRhythm[rhythmIndex]){
    return;
  }else{
    rhythmIndex++;
    if(rhythmIndex >= 4) rhythmIndex = 0;
  
    lastBeat = millis();
  
    ledRed(ledOn);
    ledOn = !ledOn;
  }
}

void showMood(uint8_t red, uint8_t green, uint8_t blue, uint8_t bright){
  for(uint8_t i=0;i<ledCount;i++){
    moodLight.setPixelColor(i, red, green, blue);
  }
  moodLight.setBrightness(bright);
  moodLight.show();
}

void playSounds(uint8_t ceasePlaying){
  // Playing music involves both reading and writing, since we only
  // want to do it once.
  static bool startedPlaying = false;

  if(buzzer.isPlaying() && ceasePlaying){
    buzzer.stopPlaying();
    startedPlaying = false;
    stem.buffer.playNotes = false;
    return;
  }

  if(stem.buffer.playNotes && !startedPlaying){
    buzzer.play(stem.buffer.notes);
    startedPlaying = true;
  }else if (startedPlaying && !buzzer.isPlaying()){
    stem.buffer.playNotes = false;
    startedPlaying = false;
  }
}

void doGrumble(){
  playSounds(1);

  uint8_t toneCount = random(1, 4);

  for(uint8_t i=1;i<=toneCount;i++){
    uint8_t theFreq = random(40, 181);
    buzzer.playFrequency(theFreq, 150, 8);
    while(buzzer.isPlaying());
  }
  
}
