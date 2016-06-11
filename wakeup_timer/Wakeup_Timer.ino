/*
 * Wakeup Timer by kazunori279
 */

// consts
const int IFTTT_IN = 0;
const int SENSOR_IN = A0;
const int SWITCH_IN = A1;
const int OSC_OUT = 5;
const int BUZZ_OUT = 9;

// melody notes
#define _0 0
#define _C 61
#define _D 68
#define _E 76
#define _F 83
#define _G 90
#define _A 98
#define _B 105
#define _c 112

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

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  pinMode(IFTTT_IN, INPUT);
  pinMode(SENSOR_IN, INPUT);
  pinMode(SWITCH_IN, INPUT);
  pinMode(OSC_OUT, OUTPUT);
  pinMode(BUZZ_OUT, OUTPUT);
  analogWrite(OSC_OUT, 0);
  analogWrite(BUZZ_OUT, 0);
}

// the main event loop
void loop() {

  // wait until switched on
  printDebug(10);
  while(digitalRead(SWITCH_IN) == LOW) delay(100);

  // wait until receiving IFTTT trigger
  printDebug(20);
  while(digitalRead(SWITCH_IN) == HIGH && digitalRead(IFTTT_IN) == LOW) delay(100);

  // play songs
  printDebug(30);
  playSongs();
  printDebug(40);
  playBuzzer();
}

void playBuzzer() {
  for (int i = 0; i < 300 && isEnabled(); i++) {
    int d = random(1, 7);
    for (int j = 100; j < 255; j++) {
      analogWrite(BUZZ_OUT, j);
      delay(d);
    }
    for (int j = 255; j > 100; j--) {
      analogWrite(BUZZ_OUT, j);
      delay(d);
    }
  }
  analogWrite(BUZZ_OUT, 0);
}

void playSongs() {
  playSong(500, 1);
  playSong(450, 1);
  playSong(400, 1);
  playSong(350, 1);
  playSong(300, 1);
  playSong(250, 1);
  playSong(200, 2);
  playSong(150, 2);
  playSong(100, 3);
  playSong(75, 5);
  playSong(50, 5);
  playSong(40, 5);
  playSong(30, 5);
}

void playSong(int interval, int repeatation) {
  for (int i = 0; i < repeatation && isEnabled(); i++) {
    for (int j = 0; j < NOTES_COUNT && isEnabled(); j++) {
      playNote(pooh_melody[j], duration[j] * interval);
    }      
  }
}

void playNote(int note, int duration) {
  analogWrite(OSC_OUT, note);
  delay(duration);
  analogWrite(OSC_OUT, 0);
  delay(duration);
}

boolean isEnabled() {
  return (digitalRead(SWITCH_IN) == HIGH) && (digitalRead(SENSOR_IN) == LOW);
}

void printDebug(int code) {
  Serial.println("code: " + String(code) + "  SWT: " + String(digitalRead(SWITCH_IN)) + 
    "  IFT: " + String(digitalRead(IFTTT_IN)) + "  SEN: " + String(digitalRead(SENSOR_IN)));  
}
