/*
  Shu Timer
  
  A Timer app with for Shu-chan to limit his TV/Gaming time.
  Runs on littleBits + Arduino.
 */

// consts
const int START_BTN = 0; // timer start button input at D0
const int STOP_BTN = A0; // timer stop button input at A0
const int RESET_BTN = A1; // timer reset button input at A1
const int BLINKER_LED = 1; // blinker LED output at D1
const int COUNTER_LED = 5; // counter 7-seg LED output at D5
const int CLOUDBIT = 9; // cloudBit output at D9

const int START_VAL = 20 * 60; // the unit amount for starting timer in seconds
const int MAX_VAL = 99 * 60; // max amount of timer in seconds

// states
int timerValue = 0;
unsigned long lastTick = 0;
boolean isStopped = false;
int startBtnPrev = LOW;
int stopBtnPrev = 0;
int timerValuePrev = 0;

// setup
void setup() {
  pinMode(BLINKER_LED, OUTPUT);
  pinMode(COUNTER_LED, OUTPUT);
  pinMode(CLOUDBIT, OUTPUT);
  analogWrite(CLOUDBIT, 0);
}

// main loop
void loop() {
  delay(100);
  ticking();
  checkResetButton();
  checkStartButton();
  checkStopButton();
  outputCloudBit();
  displayBlinker();
  displayCounter();
}

// decrease timerValue every one sec if timer is not stopped
void ticking() {
  if (lastTick == 0 || millis() - lastTick > 1000) {
    lastTick = millis();
    timerValuePrev = timerValue;
    if (timerValue > 0 && !isStopped) {
      timerValue--;
    }
  }
}

// if reset button is pressed, clear all the states
void checkResetButton() {
  if (digitalRead(RESET_BTN) == HIGH) {
    lastTick = 0;
    timerValue = 0;
    timerValuePrev = 0;
    isStopped = false;
  }
}

// if start button is pressed, increase timerValue by START_VAL
void checkStartButton() {
  boolean startBtn = digitalRead(START_BTN);
  if (startBtn == HIGH && startBtnPrev == LOW) {
    timerValue = min(timerValue + START_VAL, MAX_VAL);
  }
  startBtnPrev = startBtn;
}

// if stop button is pressed, flip isStopped flag
void checkStopButton() {
  int stopBtn = analogRead(STOP_BTN);
  if (stopBtn > 90 && stopBtnPrev == 0 && timerValue > 0) {
    isStopped = !isStopped;
  }
  stopBtnPrev = stopBtn;
}

// output to blinker LED
void displayBlinker() {

  // if timer is not working, turn off
  if (timerValue == 0) {
      digitalWrite(BLINKER_LED, LOW);
      return;
  }

  // if timer is stopped, turn on
  if (isStopped) {
      digitalWrite(BLINKER_LED, HIGH);
      return;
  }

  // if timer is ticking, blink it
  int blinkerValue = LOW;
  if(timerValue % 2 == 0) {
    blinkerValue = LOW;
  } else {
    blinkerValue = HIGH;
  }
  digitalWrite(BLINKER_LED, blinkerValue);
}

// output to counter 7seg LED
void displayCounter() {
  int counterValue = counterValue = map(timerValue, 0, 5999, 0, 255); // maps 6000 sec to 100 minutes on littleBits LED
  analogWrite(COUNTER_LED, counterValue);
}

// output to cloutBit when timer counting is finished
void outputCloudBit() {
  if (timerValue == 0 && timerValuePrev == 1) {
    analogWrite(CLOUDBIT, 255);
    delay(500);
    analogWrite(CLOUDBIT, 0);
  }
}
