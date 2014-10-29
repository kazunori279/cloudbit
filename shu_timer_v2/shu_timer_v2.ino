/*
  Shu Timer version 2
  
  A Timer app with for Shu-chan to limit his TV/Gaming time. 
  Runs on littleBits + Arduino. Version 2 added melody alarm function.
 */

// melody notes
#define _C 262
#define _D 294
#define _E 330
#define _F 349
#define _G 392
#define _A 440
#define _B 494
#define _c 523

// consts
const int START_BTN = 0; // timer start button input at D0
const int CLOUDBIT_IN = A0; // cloudBit input at A0
const int STOP_BTN = A1; // timer stop button input at A1
const int SPEAKER_OUT = 1; // blinker LED and speaker output at D1
const int COUNTER_LED = 5; // counter 7-seg LED output at D5
const int CLOUDBIT_OUT = 9; // cloudBit output at D9

const int START_VAL = 20 * 60; // the unit amount for starting timer in seconds
const int MAX_VAL = 99 * 60; // max amount of timer in seconds

const int MAX_MELODY_COUNT = 1; // how many times melody will be played at alarming

// notes of "winny the pooh"
const int pooh_melody[] = 
 {_B, _A, _G, _F, _D, _C, _D, _C, 
  _A, _B, _A, _G, _F, _G, _A,
  _C, _B, _A, _G, _F, _G, _c, _A, _F,
  _C, _D, _E, _F, _E, _D, _C, _F};

// durations of the melody
const int duration[] = 
 {1, 1, 1, 2, 1, 2, 1, 2,
  1, 1, 1, 1, 2, 1, 4,
  1, 1, 1, 1, 2, 1, 2, 1, 2,
  1, 1, 1, 1, 1, 1, 1, 4};

// total number of notes
const int NOTES_COUNT = 32;

// duration of each note
const int NOTE_DUR = 200;

// states
int timerValue = 0; // current timer value
unsigned long lastTick = 0; // timestamp of last tick time
boolean isStopped = false; // is timer stopped?
int startBtnPrev = LOW; // previous start button value
int stopBtnPrev = 0; // previous stop button value
int timerValuePrev = 0; // previous timer value
int melodyCount = 0; // remaining count of melody play
int noteIndex = 0; // current note index
unsigned long nextToneTime = 0; // time to play next tone
unsigned long nextNoToneTime = 0; // time to stop play tone

// setup
void setup() {
  pinMode(SPEAKER_OUT, OUTPUT);
  pinMode(COUNTER_LED, OUTPUT);
  pinMode(CLOUDBIT_OUT, OUTPUT);
  analogWrite(CLOUDBIT_OUT, 0);
}

// main loop
void loop() {
  delay(10);
  ticking();
  checkStartButton();
  checkStopButton();
  checkTimeout();
  playClick();
  playMelody();
}

// decrease timerValue every one sec if timer is not stopped
void ticking() {
  if (millis() - lastTick > 1000) {
    lastTick = millis();
    timerValuePrev = timerValue;
    if (timerValue > 0 && !isStopped) {
      timerValue--;
    }
    displayCounter();
  }
}

// if stop button is pressed, stop the timer. If it's already stopped, reset the timer
void checkStopButton() {
  boolean stopBtn = analogRead(STOP_BTN);
  if (stopBtn > 200 && stopBtnPrev == 0) {
    if (isStopped || melodyCount > 0) {
      reset();
    } else {
      isStopped = true;
    }
  }
  stopBtnPrev = stopBtn;
}

void reset() {
  timerValue = 0;
  lastTick = 0;
  isStopped = false;
  startBtnPrev = LOW;
  stopBtnPrev = 0;
  timerValuePrev = 0;
  melodyCount = 0;
  noteIndex = 0;
  nextToneTime = 0;
  nextNoToneTime = 0;
  noTone(SPEAKER_OUT);
}

// if start button is pressed, increase timerValue by START_VAL
void checkStartButton() {
  boolean startBtn = digitalRead(START_BTN);
  if (startBtn == HIGH && startBtnPrev == LOW) {
    if (isStopped) {
      isStopped = false;
    } else {
      timerValue = min(timerValue + START_VAL, MAX_VAL);
    }
  }
  startBtnPrev = startBtn;
  displayCounter();
}

// output click sound to speaker
void playClick() {
  
  // if melody is playing, skip
  if (melodyCount > 0) return;

  // if timer is not working, turn off
  if (timerValue == 0) {
      digitalWrite(SPEAKER_OUT, LOW);
      return;
  }

  // if timer is stopped, turn on
  if (isStopped) {
      digitalWrite(SPEAKER_OUT, HIGH);
      return;
  }

  // if timer is ticking, blink it
  int blinkerValue = LOW;
  if(timerValue % 2 == 0) {
    blinkerValue = LOW;
  } else {
    blinkerValue = HIGH;
  }
  digitalWrite(SPEAKER_OUT, blinkerValue);
}

// output to counter 7seg LED
void displayCounter() {
  int counterValue = map(timerValue, 0, 5999, 0, 255); // maps 6000 sec to 100 minutes on littleBits LED
  analogWrite(COUNTER_LED, counterValue);
}

// output to cloutBit when timer counting is finished
void checkTimeout() {
  if (timerValue == 0 && timerValuePrev == 1) {
    analogWrite(CLOUDBIT_OUT, 255);
    delay(100);
    analogWrite(CLOUDBIT_OUT, 0);
    melodyCount = MAX_MELODY_COUNT;
  }
}

// play the melody if melodyCount > 0
void playMelody() {  
  
  // if melody is not playing, return
  if (melodyCount == 0) return;
  
  // if next tone time has reached, play next note
  if (millis() > nextToneTime) {
    tone(SPEAKER_OUT, pooh_melody[noteIndex]);
    nextToneTime = millis() + (duration[noteIndex] * NOTE_DUR);
    nextNoToneTime = millis() + (duration[noteIndex] * NOTE_DUR * 0.7);
    noteIndex++;
  }

  // if next no tone time has reached, stop tone
  if (millis() > nextNoToneTime) {
    noTone(SPEAKER_OUT);

    // if all notes has been played, decrease melodyCount.
    if (noteIndex == NOTES_COUNT) {
      noteIndex = 0;
      nextToneTime = 0;
      nextNoToneTime = 0;
      melodyCount--;
    }
  }
}

