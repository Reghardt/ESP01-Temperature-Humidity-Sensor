#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_AM2320.h"
#include <RTClib.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIRE Wire


Adafruit_AM2320 am2320 = Adafruit_AM2320();
RTC_DS3231 rtc;


Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &WIRE);


void setup() {

  Wire.begin(2,0);
  Serial.begin(9600);
  am2320.begin();

  

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  if (! rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) 
  {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();



}

void loop() {


  float temp = am2320.readTemperature();
  float humid = am2320.readHumidity();
  DateTime now = rtc.now();

  // lcd.setCursor(0,0);
  // lcd.print("Temp: " + String(temp) + "C   ");
  // lcd.setCursor(0,1);
  // lcd.print("Humidity: " + String(humid) + " %   ");
  // lcd.setCursor(0,2);
  // lcd.print(String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()) + " - " + String(now.hour()) + ":" + String(now.minute()) + "  ");

  Serial.print(String(temp) + " ");
  Serial.println(humid);

  // text display tests
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0,0);
  display.setTextColor(SSD1306_WHITE);
  display.print(temp, 1);
  display.setTextSize(2);
  display.print("C ");
  display.setCursor(90, 16);
  display.setTextSize(2);
  if(humid >= 99.0f)
  {
    display.print(99);
  }
  else
  {
    display.print(humid, 0);
  }
  
  display.print("%");

  display.setCursor(0, 24);
  display.setTextSize(1);
  display.print(String(now.month()) + "/" + String(now.day()) + " - " + String(now.hour()) + ":" + String(now.minute()));


  display.display(); // actually display all of the above

  delay(10000);                                                                           
}

