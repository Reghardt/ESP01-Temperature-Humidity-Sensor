#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "Adafruit_AM2320.h"
#include <RTClib.h>


#define SCREEN_I2C_ADDRESS 0x3c
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AM2320 am2320 = Adafruit_AM2320();
RTC_DS3231 rtc;

void startRTC();

void setup()   {
  Wire.begin(2,0);
  Serial.begin(9600);

  if (! rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) 
  {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  am2320.begin();

  delay(250); // wait for the OLED to power up
  display.begin(SCREEN_I2C_ADDRESS, true); // Address 0x3C default
  display.display(); //Adafruit splash screen
  delay(2000); //wait for splash creen
  display.setTextColor(SH110X_WHITE); //needed for displaying text later on
}

void loop() {

  float temp = am2320.readTemperature();
  float humid = am2320.readHumidity();
  DateTime now = rtc.now();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(4);
  display.print(temp, 1);
  display.setTextSize(3);
  display.println('C');

  display.setCursor(0, 40);
  display.setTextSize(3);
  humid >= 99 ? display.print(99) : display.print(humid, 0);
  display.print("%");

  display.display();

  delay(10000);
}
