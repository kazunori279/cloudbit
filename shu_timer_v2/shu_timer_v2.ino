/*
  Shu Timer version 2
  
  A Timer app with for Shu-chan to limit his TV/Gaming time. 
  Runs on littleBits + Arduino. Version 2 added melody alarm function.
 */

// melody notes
#define _0 0
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
const int START_TIMER_VAL = 20 * 60; // the unit amount for starting timer in seconds
const int MAX_TIMER_VAL = 99 * 60; // max amount of timer in seconds
const int ANALOG_HIGH_THRETHOLD = 100; // threshold to determine an analog value is HIGH
const int MAX_MELODY_COUNT = 1; // how many times melody will be played at alarming

// notes of "winny the pooh"
const int pooh_melody[] = 
 {_B, _A, _G, _F, _D, _C, _D, _C, 
  _A, _B, _A, _G, _F, _G, _A,
  _C, _B, _A, _G, _F, _G, _c, _A, _F,
  _C, _D, _E, _F, _E, _D, _C, _F, _0};

// durations of the melody
const int duration[] = 
 {1, 1, 1, 2, 1, 2, 1, 2,
  1, 1, 1, 1, 2, 1, 4,
  1, 1, 1, 1, 2, 1, 2, 1, 2,
  1, 1, 1, 1, 1, 1, 1, 4, 8};
const int NOTES_COUNT = 33; // total number of notes
const int NOTE_DUR = 200; // duration of each note
const float NOTE_DUTY = 0.7; // duty ratio of each note

// states
int timerValue = 0; // current timer value
unsigned long lastTick = 0; // timestamp of last tick time
boolean isStopped = false; // is timer stopped?
int startBtnPrev = LOW; // previous start button value
int stopBtnPrev = 0; // previous stop button value
int timerValuePrev = 0; // previous timer value
boolean isCloudBitInputHigh = false; // is cloudBit input HIGH?
int melodyCount = 0; // remaining count of melody play
int noteIndex = 0; // current note index
unsigned long nextToneTime = 0; // time to play next tone
unsigned long nextNoToneTime = 0; // time to stop play tone

// setup
void setup() {
  pinMode(START_BTN, INPUT);
  pinMode(CLOUDBIT_IN, INPUT);
  pinMode(STOP_BTN, INPUT);
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
  checkTimerTimeout();
  checkCloudBitInput();
  playClick();
  playNextTone();
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
  if (stopBtn > ANALOG_HIGH_THRETHOLD && stopBtnPrev == 0) {
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
  isCloudBitInputHigh = false;
  melodyCount = 0;
  resetMelody();
}

// if start button is pressed, increase timerValue by START_VAL
void checkStartButton() {
  int startBtn = digitalRead(START_BTN);
  if (startBtn == HIGH && startBtnPrev == LOW) {
    if (isStopped) {
      isStopped = false;
    } else {
      timerValue = min(timerValue + START_TIMER_VAL, MAX_TIMER_VAL);
    }
  }
  startBtnPrev = startBtn;
  displayCounter();
}

// output to counter 7seg LED
void displayCounter() {
  // output only when it's not playing melody (to avoid internal timer conflict)
  if (melodyCount == 0) {
    analogWrite(COUNTER_LED, map(timerValue, 0, 5999, 0, 255)); // maps 6000 sec to 100 minutes on littleBits LED
  }
}

// output to cloudBit and play melody when timer counting is finished
void checkTimerTimeout() {
  if (timerValue == 0 && timerValuePrev == 1) {
    analogWrite(CLOUDBIT_OUT, 255);
    delay(100);
    analogWrite(CLOUDBIT_OUT, 0);
    startPlayingMelody();
  }
}

// check input from cloudBit to play melody
void checkCloudBitInput() {
  int cloudBitInput = analogRead(CLOUDBIT_IN);
  if (cloudBitInput >= ANALOG_HIGH_THRETHOLD && !isCloudBitInputHigh) {
    isCloudBitInputHigh = true;
    startPlayingMelody();
  }
  if (cloudBitInput < ANALOG_HIGH_THRETHOLD) {
    isCloudBitInputHigh = false;
  }
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
  if (timerValue % 2 == 0) {
    blinkerValue = LOW;
  } else {
    blinkerValue = HIGH;
  }
  digitalWrite(SPEAKER_OUT, blinkerValue);
}

// start playing melody for MAX_MELODY_COUNT times
void startPlayingMelody() {
  melodyCount = MAX_MELODY_COUNT;
  analogWrite(COUNTER_LED, 0); // to avoid conflict with tone()
}

// play next tone if melodyCount > 0
void playNextTone() {  
  
  // if it is not playing melody, return
  if (melodyCount == 0) return;
  
  // if next tone time has reached, play next note
  if (millis() > nextToneTime) {
    int nextTone = pooh_melody[noteIndex];
    if (nextTone != _0) {
      tone(SPEAKER_OUT, nextTone);
    } else {
      noTone(SPEAKER_OUT);
    }
    nextToneTime = millis() + (duration[noteIndex] * NOTE_DUR);
    nextNoToneTime = millis() + (duration[noteIndex] * NOTE_DUR * NOTE_DUTY);
    noteIndex++;
  }

  // if next no tone time has reached, stop tone
  if (millis() > nextNoToneTime) {
    noTone(SPEAKER_OUT);

    // if all notes has been played, stop the melody and decrease melodyCount.
    if (noteIndex == NOTES_COUNT) {
      resetMelody();
      melodyCount--;
    }
  }
}

// reset states for playing melody
void resetMelody() {
  
  // reset variables for tone generation
  noteIndex = 0;
  nextToneTime = 0;
  nextNoToneTime = 0;

  // reset internal timer after using tone()/noTone()
  // see http://discuss.littlebits.cc/t/analogwrite-on-d5-doesnt-work-after-using-tone/7833
  noTone(SPEAKER_OUT);
  TCCR3A = 1;
  TCCR3B = 3;  
}

