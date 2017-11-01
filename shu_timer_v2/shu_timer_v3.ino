/*
  Shu Timer version 3
  
  A Timer app with for Shu-chan to limit his TV/Gaming time. 
  Runs on littleBits + Arduino. Version 3 added a support for Google Assistant.
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
const int CTRL_BTN = 0; // control button input at D0
const int CLOUDBIT_IN = A1; // cloudBit input at A0
const int SPEAKER_OUT = 9; // blinker LED and speaker output at D5
const int COUNTER_LED = 1; // counter 7-seg LED output at D1
const int COUNTER_RESET = 5; // counter reset port at D9
const int CLOUDBIT_OUT = 10; // cloudBit output at D10

const int MAX_TIMER_VAL = 99 * 60; // max amount of timer in seconds
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

// states for the counter
int ST_STOP = 0; // counting stopped
int ST_STARTING = 1; // counting will start soon
int ST_COUNTING = 2; // counting
int countState = ST_STOP;
unsigned long countStartingAt = 0;

unsigned long lastTick = 0; // timestamp of last tick time
int timerValue = 0; // current timer value
int timerValuePrev = 0; // previous timer value

// states for the button
int ctrlBtnPrev = LOW; // previous button value
unsigned long ctrlBtnReleasedAt = 0; // last tick when button released

// states for cloudBit
int cloudBitInputPrev = 0;

// states for playing the melody
int melodyCount = 0; // remaining count of melody play
int noteIndex = 0; // current note index
unsigned long nextToneTime = 0; // time to play next tone
unsigned long nextNoToneTime = 0; // time to stop play tone

/*
 * Setup and main loop
 */

// setup
void setup() {
  Serial.begin(9600);
  pinMode(CTRL_BTN, INPUT);
  pinMode(CLOUDBIT_IN, INPUT);
  pinMode(SPEAKER_OUT, OUTPUT);
  pinMode(COUNTER_LED, OUTPUT);
  pinMode(COUNTER_RESET, OUTPUT);
  pinMode(CLOUDBIT_OUT, OUTPUT);
  reset();
}

void reset() {
  timerValue = 0;
  lastTick = 0;
  ctrlBtnPrev = LOW;
  ctrlBtnReleasedAt = 0;
  timerValuePrev = 0;
  melodyCount = 0;
  countState = ST_STOP;
  countStartingAt = 0;
  cloudBitInputPrev = 0;
  resetMelody();
  resetCounter();
}

// main loop
void loop() {
  delay(10);
  ticking();
  checkCtrlButton();
  checkCloudBitInput();
  playNextTone();
}

/*
 * Counter management
 */

// decrease timerValue every one sec if timer is not stopped
void ticking() {
  if (millis() - lastTick > 1000) {
    lastTick = millis();
    counting();
  }
}

// cound down and update the LED every 1 sec
void counting() {

  // skip if it's not counting or playing the melody
  if (countState != ST_COUNTING || melodyCount > 0) return;

  // count down
  if (timerValue > 0) {
    timerValue--;
  }
  updateLED();

  // play click
  if (melodyCount== 0) {
    digitalWrite(SPEAKER_OUT, (timerValue % 2));
  }
  
  // check if it's time to alarm
  if (timerValue == 0 && timerValuePrev == 1) {
    digitalWrite(CLOUDBIT_OUT, HIGH);
    delay(100);
    digitalWrite(CLOUDBIT_OUT, LOW);
    melodyCount = MAX_MELODY_COUNT; // start playing the melody
    countState == ST_STOP;
  }
  timerValuePrev = timerValue;
}

void updateLED() {
  
  // update LED
  resetCounter();
  int n = timerValue;
  if (timerValue > 60) {
    n = round(timerValue / 60.0); // in minutes
  }
  for (int i = 0; i < n; i++) {
    digitalWrite(COUNTER_LED, HIGH);
    delay(1);
    digitalWrite(COUNTER_LED, LOW);
    delay(1);
  }
}

void resetCounter() {
  digitalWrite(COUNTER_RESET, HIGH);
  delay(5);
  digitalWrite(COUNTER_RESET, LOW);  
}

/*
 * Control Button
 */

// if ctrl button is pressed, increase timerValue by START_VAL
void checkCtrlButton() {

  // check button status
  int ctrlBtn = digitalRead(CTRL_BTN);

  // reset all when button pressed for 1 sec
  if (ctrlBtn == LOW) {
    ctrlBtnReleasedAt = millis();
  }
  if (millis() - ctrlBtnReleasedAt > 1000) {
    reset();
    return;
  }  

  // change ST_STARTING -> ST_COUNTING after 3 sec
  if (countState == ST_STARTING && (millis() - countStartingAt) > 3000) {
    countState = ST_COUNTING;
  }

  // skip if button not clicked
  if (ctrlBtn != HIGH || ctrlBtnPrev != LOW) {
    ctrlBtnPrev = ctrlBtn;
    return;
  }
    
  // button clicked
  if (countState == ST_STOP) {
    if (timerValue == 0) {
      countState = ST_STARTING;
      countStartingAt = millis();        
      increaseTimerValue();
    } else {
      countState = ST_COUNTING;
    }
  } else if (countState == ST_STARTING) {
    countState = ST_STARTING;
    countStartingAt = millis();
    increaseTimerValue();
  } else if (countState == ST_COUNTING) {
    countState = ST_STOP;
  }
  ctrlBtnPrev = ctrlBtn;
}

void increaseTimerValue() {
  if (timerValue == 0) {
    timerValue = 3000;
  } else if (timerValue == 3000) {
    timerValue = 600;
  } else if (timerValue == 600) {
    timerValue = 300;
  } else if (timerValue == 300) {
    timerValue = 180;
  }
  updateLED();
}

// check input from cloudBit to play melody
void checkCloudBitInput() {
  int cloudBitInput = analogRead(CLOUDBIT_IN);
//  Serial.println(cloudBitInput);
  if (cloudBitInput - cloudBitInputPrev > 100) {
    timerValue = 3000;
    updateLED();
    countState = ST_COUNTING;
  }
  cloudBitInputPrev = cloudBitInput;
}

/*
 * Playing the melody
 */

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
