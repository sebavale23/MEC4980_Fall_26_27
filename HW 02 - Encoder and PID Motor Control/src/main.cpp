#include <Arduino.h>
#include <PID_v1.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

Adafruit_ST7789 tft =
  Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// H-bridge motor pins
const int motorPin1 = 9;
const int motorPin2 = 10;

// Photoresistor
const int encoderPin = A0;

// Buttons
const int buttonD0 = 0;
const int buttonD1 = 1;
const int buttonD2 = 2;

// Four open gaps in the encoder disk
const int pulsesPerRevolution = 4;

// Photoresistor values
int lightReading = 0;
int lightThreshold = 2000;
int lightHysteresis = 300;
bool gapDetected = false;

unsigned long lastPulseTime = 0;
unsigned long previousDisplayTime = 0;
unsigned long previousButtonTime = 0;

const unsigned long debounceTime = 200;

// Motor variables
double targetRPM = 60.0;
double motorRPM = 0.0;
double motorPWM = 0.0;

// Starting PID values
double Kp = 0.7;
double Ki = 0.30;
double Kd = 0.0;

PID motorPID(
  &motorRPM,
  &motorPWM,
  &targetRPM,
  Kp,
  Ki,
  Kd,
  DIRECT
);

bool motorRunning = false;

int previousD0 = HIGH;
int previousD1 = LOW;
int previousD2 = LOW;

void readEncoder();
void checkButtons();
void runMotor();
void stopMotor();
void displayScreen();

void setup() {
  Serial.begin(115200);

  // Turn on TFT display
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);

  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  tft.init(135, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  // Motor pins
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, 0);

  // Photoresistor
  pinMode(encoderPin, INPUT);

  // Buttons
  pinMode(buttonD0, INPUT_PULLUP);
  pinMode(buttonD1, INPUT_PULLDOWN);
  pinMode(buttonD2, INPUT_PULLDOWN);

  // PID setup
  motorPID.SetOutputLimits(0, 255);
  motorPID.SetSampleTime(200);
  motorPID.SetMode(MANUAL);

  displayScreen();
}

void loop() {
  readEncoder();
  checkButtons();

  if (motorRunning) {
    motorPID.Compute();
    runMotor();
  } else {
    stopMotor();
  }

  if (millis() - previousDisplayTime >= 500) {
    previousDisplayTime = millis();

    displayScreen();

    Serial.print("Target: ");
    Serial.print(targetRPM);

    Serial.print(" RPM: ");
    Serial.print(motorRPM);

    Serial.print(" PWM: ");
    Serial.print(motorPWM);

    Serial.print(" Light: ");
    Serial.println(lightReading);
  }
}

void readEncoder() {
  lightReading = analogRead(encoderPin);

  // Natural light reaches the photoresistor through a gap
  if (!gapDetected &&
      lightReading > lightThreshold + lightHysteresis) {

    gapDetected = true;

    unsigned long currentPulseTime = micros();

    if (lastPulseTime != 0) {
      unsigned long pulsePeriod =
        currentPulseTime - lastPulseTime;

      double newRPM =
        60000000.0 /
        (pulsePeriod * pulsesPerRevolution);

      // Smooth the RPM measurement
      motorRPM = 0.8 * motorRPM + 0.2 * newRPM;
    }

    lastPulseTime = currentPulseTime;
  }

  // A fin blocks the natural light
  if (gapDetected &&
      lightReading < lightThreshold - lightHysteresis) {

    gapDetected = false;
  }

  // No pulse for one second means the motor stopped
  if (lastPulseTime != 0 && micros() - lastPulseTime > 1000000) {
    motorRPM = 0;
  }
}

void checkButtons() {
  unsigned long currentTime = millis();

  int currentD0 = digitalRead(buttonD0);
  int currentD1 = digitalRead(buttonD1);
  int currentD2 = digitalRead(buttonD2);

  // D0 starts or stops the motor
  if (currentD0 == LOW &&
      previousD0 == HIGH &&
      currentTime - previousButtonTime > debounceTime) {

    previousButtonTime = currentTime;
    motorRunning = !motorRunning;

    if (motorRunning) {
      motorPWM = 0;
      motorPID.SetMode(AUTOMATIC);
    } else {
      motorPID.SetMode(MANUAL);
      motorPWM = 0;
    }
  }

  // D1 increases the target speed
  if (currentD1 == HIGH &&
      previousD1 == LOW &&
      currentTime - previousButtonTime > debounceTime) {

    previousButtonTime = currentTime;
    targetRPM += 10;

    if (targetRPM > 140) {
      targetRPM = 140;
    }
  }

  // D2 decreases the target speed
  if (currentD2 == HIGH &&
      previousD2 == LOW &&
      currentTime - previousButtonTime > debounceTime) {

    previousButtonTime = currentTime;
    targetRPM -= 10;

    if (targetRPM < 50) {
      targetRPM = 50;
    }
  }

  previousD0 = currentD0;
  previousD1 = currentD1;
  previousD2 = currentD2;
}

void runMotor() {
  int pwmValue = constrain((int)motorPWM, 0, 255);

  analogWrite(motorPin1, pwmValue);
  analogWrite(motorPin2, 0);
}

void stopMotor() {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, 0);
}

void displayScreen() {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);

  tft.setCursor(10, 5);
  tft.println("Motor Speed");

  tft.setCursor(10, 30);
  tft.print("Set: ");
  tft.print(targetRPM, 0);

  tft.setCursor(10, 53);
  tft.print("RPM: ");
  tft.print(motorRPM, 1);

  tft.setCursor(10, 76);
  tft.print("PWM: ");
  tft.print(motorPWM, 0);

  tft.setTextSize(1);

  tft.setCursor(10, 100);
  tft.print("Light sensor: ");
  tft.print(lightReading);

  tft.setCursor(10, 112);

  if (motorRunning) {
    tft.setTextColor(ST77XX_GREEN);
    tft.print("Motor running");
  } else {
    tft.setTextColor(ST77XX_RED);
    tft.print("Motor stopped");
  }

  tft.setCursor(10, 125);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("D0 Start  D1 +  D2 -");
}