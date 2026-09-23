#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  Serial.begin(9600);
  delay(1000);

  int result = myFunction(2, 3);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLDOWN);
  pinMode(2, INPUT_PULLDOWN);
}

long prevSampleTime = 0;
long timeBetweenSamplesMs = 100;
void loop() {
  long currentTime = millis();
  if (currentTime > prevSampleTime + timeBetweenSamplesMs) {
    Serial.print("D0, D1, D2, A0: ");
    Serial.print(digitalRead(0));
    Serial.print(digitalRead(1));
    Serial.print(digitalRead(2));
    Serial.print("   ");
    Serial.println(analogRead(A0));
  }
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}