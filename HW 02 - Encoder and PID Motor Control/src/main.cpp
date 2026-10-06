#include <Wire.h>
#include <Adafruit_MotorShield.h>

Adafruit_MotorShield motorShield = Adafruit_MotorShield();
Adafruit_DCMotor *motor = motorShield.getMotor(1);

#define ENCODER_PIN 5

volatile int pulses = 0;

float targetRPM = 200;
float actualRPM = 0;

float Kp = 1.0;
float Ki = 0.2;
float Kd = 0.05;

float error = 0;
float lastError = 0;
float integral = 0;

int motorSpeed = 0;

const int pulsesPerRevolution = 8;

unsigned long lastTime = 0;

void IRAM_ATTR countPulse() {
  pulses++;
}

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_PIN, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(ENCODER_PIN),
    countPulse,
    RISING
  );

  motorShield.begin();

  motor->setSpeed(0);
  motor->run(FORWARD);

  Serial.println("PID Motor Control");
  Serial.println("Enter desired RPM:");
}

void loop() {

  if (Serial.available()) {
    targetRPM = Serial.parseFloat();

    while (Serial.available())
      Serial.read();

    Serial.print("New Target RPM: ");
    Serial.println(targetRPM);
  }

  if (millis() - lastTime >= 500) {

    noInterrupts();
    int pulseCount = pulses;
    pulses = 0;
    interrupts();

    actualRPM =
      (pulseCount * 120.0) /
      pulsesPerRevolution;

    error = targetRPM - actualRPM;

    integral += error * 0.5;

    float derivative =
      (error - lastError) / 0.5;

    motorSpeed =
      Kp * error +
      Ki * integral +
      Kd * derivative +
      motorSpeed;

    motorSpeed =
      constrain(motorSpeed, 0, 255);

    motor->setSpeed(motorSpeed);
    motor->run(FORWARD);

    lastError = error;
    lastTime = millis();

    Serial.print("Target: ");
    Serial.print(targetRPM);

    Serial.print(" RPM   Actual: ");
    Serial.print(actualRPM);

    Serial.print(" RPM   Motor: ");
    Serial.println(motorSpeed);
  }
}