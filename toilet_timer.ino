int timer = 0;

void setup() {
  pinMode(A1, INPUT);  
  pinMode(0, INPUT);
  pinMode(9, OUTPUT);
}

void loop() {

  // wait for 0.01s
  delay(10);

  // check the IR sensor
  int a1 = analogRead(A1);
  if (a1 > 500) {
    timer = 0;
  }
  timer++;

  // after 5 sec, beep
  if (timer >= 500) {
    digitalWrite(9, HIGH);
    timer = 500;
  } else {
    digitalWrite(9, LOW);
  }

}
