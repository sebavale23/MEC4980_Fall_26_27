#include <Arduino.h>
#include <math.h>
#include <Adafruit_BNO08x.h>
#include <AceButton.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_GFX.h>
using namespace ace_button;
 
void setReports();
#define BNO08X_RESET -1
int pinD0 = 0;
int pinD1 = 1;
int pinD2 = 2;
 
AceButton buttonD0(pinD0);
AceButton buttonD1(pinD1);
AceButton buttonD2(pinD2);
 
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
 
enum ScreenMode {
  SCREEN_STEPS, // 0
  SCREEN_DISTANCE, // 1
  SCREEN_STRIDE, // 2
  SCREEN_RAW, // 3
  SCREEN_COUNT // 4
};
ScreenMode curScreen = SCREEN_STEPS;
 
int steps = 0;
float strideLength = 0.75;
float x = 0;
float y = 0;
float z = 0;
float magnitude = 0;
float stepThreshold = 10.8;
 
bool stepReady = true;
 
unsigned long lastStepTime = 0;
 
void ChangeScreen(AceButton *button, uint8_t eventType, uint8_t buttonState) {
 
  // Print out a message for all events.
  Serial.print(F("handleEvent(): eventType: "));
  Serial.print(AceButton::eventName(eventType));
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
  if(eventType == (uint8_t)AceButton::kEventPressed) {
 
    if(button->getPin() == pinD0) {
      curScreen = (ScreenMode)((curScreen + 1) % SCREEN_COUNT);
    }
 
    if(button->getPin() == pinD1) {
      if(curScreen == SCREEN_STRIDE) {
        strideLength += 0.01;
 
        if(strideLength > 2.00) {
          strideLength = 2.00;
      }
    }
  }
 
    if(button->getPin() == pinD2) {
      if(curScreen == SCREEN_STRIDE) {
        strideLength -= 0.01;
 
        if(strideLength < 0.20) {
          strideLength = 0.20;
        }
      }
    }
  }
}
 
Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;
 
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens
 
  pinMode(pinD0, INPUT_PULLUP);
  buttonD0.init(pinD0, HIGH);
 
  pinMode(pinD1, INPUT_PULLDOWN);
  buttonD1.init(pinD1, LOW);
 
  pinMode(pinD2, INPUT_PULLDOWN);
  buttonD2.init(pinD2, LOW);
 
  ButtonConfig* buttonConfigD0 = buttonD0.getButtonConfig();
  buttonConfigD0->setEventHandler(ChangeScreen);
 
  ButtonConfig* buttonConfigD1 = buttonD1.getButtonConfig();
  buttonConfigD1->setEventHandler(ChangeScreen);
 
  ButtonConfig* buttonConfigD2 = buttonD2.getButtonConfig();
  buttonConfigD2->setEventHandler(ChangeScreen);
 
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
 
  display.init(135, 240);
  display.setRotation(3);
  display.fillScreen(ST77XX_BLACK);
  display.setTextColor(ST77XX_WHITE);
  display.setTextSize(1);
 
  Serial.println("Adafruit BNO08x test!");
 
    // Try to initialize!
  if (!bno08x.begin_I2C()) {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte
    // UART buffer! if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
 
  Serial.println("BNO08x Found!");
 
  setReports();
}
 
void loop() {
  buttonD0.check();
  buttonD1.check();
  buttonD2.check();
  delay(10);
 
  if (bno08x.wasReset()) {
    Serial.println("Sensor was reset");
    setReports();
  }
 
  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }
 
  x = sensorValue.un.accelerometer.x;
  y = sensorValue.un.accelerometer.y;
  z = sensorValue.un.accelerometer.z;
 
  magnitude = sqrt( x * x + y * y + z * z );
 
  if (magnitude > stepThreshold && stepReady == true && millis() - lastStepTime > 300)
  {
    steps++;
    stepReady = false;
    lastStepTime = millis();
    Serial.print("Step Count: ");
    Serial.println(steps);
  }
 
  if (magnitude < 10.3) {
    stepReady = true;
  }
 
  Serial.print("X: ");
  Serial.print(x, 2);
 
  Serial.print("  Y: ");
  Serial.print(y, 2);
 
  Serial.print("  Z: ");
  Serial.print(z, 2);
 
  Serial.print("  Magnitude: ");
  Serial.print(magnitude, 2);
 
  Serial.print("  Steps: ");
  Serial.println(steps);
 
 
  display.fillScreen(ST77XX_BLACK);
  display.setCursor(10, 10);
  display.setTextColor(ST77XX_WHITE);
  display.setTextSize(1);
 
  if (curScreen == SCREEN_STEPS) {
   display.setTextSize(2);
    display.println("Steps:");
    display.setTextSize(2);
    display.println(steps);
    display.setTextSize(1);
    display.println("");
    display.println("D0 = Next Screen");
  }
 
  else if (curScreen == SCREEN_DISTANCE) {
 
    float distance = steps * strideLength;
 
    display.setTextSize(2);
    display.println("Distance:");
    display.setTextSize(2);
    display.print(distance, 2);
    display.println(" m");
 
    display.setTextSize(1);
    display.println("");
    display.println("D0 = Next Screen");
  }
 
  else if (curScreen == SCREEN_STRIDE) {
     display.setTextSize(2);
    display.println("Stride Length:");
    display.setTextSize(2);
    display.print(strideLength, 2);
    display.println(" m");
 
    display.setTextSize(1);
    display.println("");
    display.println("D1 = Increase");
    display.println("D2 = Decrease");
    display.println("D0 = Next Screen");
  }
 
  else if (curScreen == SCREEN_RAW) {
    display.setTextSize(2);
    display.println("Acceleration:");
    display.setTextSize(1);
    display.println("");
 
    display.print("X: ");
    display.println(x, 2);
 
    display.print("Y: ");
    display.println(y, 2);
 
    display.print("Z: ");
    display.println(z, 2);
 
    display.println("");
    display.println("D0 = Next Screen");
  }
 
  delay(100);
}
 
void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  } else {
    Serial.println("Set accelerometer report... success!");
  }
}



void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  } else {
    Serial.println("Set accelerometer report... success!");