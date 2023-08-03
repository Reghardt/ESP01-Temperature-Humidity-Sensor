#include <Arduino.h>
#include <ESP8266HTTPClient.h> // used for sending http(s) requests: sending temp + humid data to a server
#include <ESP8266WebServer.h> // serves pages and handles client requests like setting the wifi name and pass
// #include "WiFiClientSecure.h"
// #include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "Adafruit_AM2320.h"
#include "./pages/pages.h"
#include "wireless/wireless.h"
#include "env.h"

#define CONNECTION_TIME_MS 5000
#define SEND_INTERVAL_MS 1800000 //every 30 mins - roughly as the system does not have a real time clock for accurate readings
#define READ_INTERVAL_MS 10000 // every 10 seconds
#define SCREEN_I2C_ADDRESS 0x3c // Address of 0x3C by default
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);
EEPROMClass eeprom;
Wireless wireless;
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AM2320 am2320 = Adafruit_AM2320();

String createJSON(String api_key, float temperature, float humidity);
void handle_NotFound();
void handle_Setup();
void handle_Credentials();
String sendReading(float temperature, float humidity);
void updateScreen(float temperature, float humidity, String nrOfReadings, bool wifiConnected);

const char* ssid = "NodeMCU"; 
const char* password = "12345678";

float temperature = 0.0f;
float humidity = 0.0f;
String nrOfReadingsSentToday = "0";

unsigned long previousSendMillis = 0;
unsigned long previousReadMillis = 0;

void setup() {
  Wire.begin(2,0);
  Serial.begin(115200);
  eeprom.begin(sizeof(WiFiCredentials));
  wireless.begin(&eeprom, ssid, password, local_ip, gateway, subnet);
  am2320.begin();
  delay(250); // wait for the OLED to power up
  display.begin(SCREEN_I2C_ADDRESS, true); 
  display.setTextColor(SH110X_WHITE); //needed for displaying text later on

  //setup server endpoints
  server.on("/", handle_Setup);
  server.on("/credentials", HTTP_POST, handle_Credentials);
  server.onNotFound(handle_NotFound);
  server.begin();

  Serial.println("HTTP server started");

  temperature = am2320.readTemperature();
  humidity = am2320.readHumidity();
  updateScreen(am2320.readTemperature(), am2320.readHumidity(), nrOfReadingsSentToday, wireless.connection(true)); //display readings without trying to connect to wifi so there is now "screen on" delay. Will display "No Wifi" at first
  
  wireless.loadCredentials(); //load wifi credentials from EEPROM
  updateScreen(am2320.readTemperature(), am2320.readHumidity(), nrOfReadingsSentToday, wireless.connection()); // update screen again but this time it can take its time trying to connect to the wifi as a recent temp + humid reading is displayed on the screen. The screen wont be black
}

void loop() 
{
  server.handleClient();
  unsigned long currentMillis = millis();

  if((unsigned long)(currentMillis - previousReadMillis) >= READ_INTERVAL_MS) //read temp + humid every x seconds
  {
    temperature = am2320.readTemperature();
    humidity = am2320.readHumidity();
    Serial.println("temp: " + String(temperature) + " humid: " + String(humidity));

    if((unsigned long)(currentMillis - previousSendMillis) >= SEND_INTERVAL_MS) // send temp + humid reading every y seconds
    {
      nrOfReadingsSentToday = sendReading(temperature, humidity);
      previousSendMillis = currentMillis;
    }

    updateScreen(temperature, humidity, nrOfReadingsSentToday, wireless.connection()); //check if connection exists on every reading. This will cause the connection state to update on the screen. ie WiFi OK -> No Wifi and vice versa
    previousReadMillis = currentMillis;
  }
}

void updateScreen(float temperature, float humidity, String nrOfReadings, bool wifiConnected)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(4);
  display.print(temperature, 1);
  display.setTextSize(3);
  display.println('C');

  display.setCursor(0, 40);
  display.setTextSize(3);
  humidity >= 99 ? display.print(99) : display.print(humidity, 0);
  display.print("%");

  display.setCursor(98, 46);
  display.setTextSize(1);
  display.print("R:" + nrOfReadings);

  display.setCursor(80, 54);
  display.setTextSize(1);
  if(wifiConnected)
  {
    display.print("WiFi OK");
  }
  else
  {
    display.print("No WiFi");
  }
  display.display();
}

String createJSON(String api_key, float temperature, float humidity){
  return "{\"api_key\":\"" + api_key + "\",\"temp\":\"" + String(temperature) + "\",\"humidity\":\"" + String(humidity) + "\"}";
}

String sendReading(float temperature, float humidity)
{
  String response = "0";
  if (wireless.connection()) 
  {
    WiFiClientSecure client;
    HTTPClient https;

    client.setInsecure();
    
    Serial.println("Requesting " + url);
    if (https.begin(client, url)) 
    {
      
      https.addHeader("Content-Type", "application/json");
      int httpCode = https.POST(createJSON(api_key, temperature, humidity));
      Serial.println("Response code: " + String(httpCode));
      if (httpCode > 0) {
        response = https.getString();
      }
      https.end();
    } 
    else 
    {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  return response;
}

void handle_Credentials(){
  if(server.hasArg("hotspot_name") == false || server.arg("hotspot_name") == NULL)
  {
    Serial.println("No hotspot name");
    delay(500);
    server.send(200, "text/html", connectionError_html);
    delay(500);
  }
  else if(server.hasArg("hotspot_pass") == false)
  {
    Serial.println("No hotspot pass");
    delay(500);
    server.send(200, "text/html", connectionError_html);
    delay(500);
  }
  else
  {
    Serial.println("credentials received OK");
    wireless.saveCredentials(server.arg("hotspot_name"), server.arg("hotspot_pass") == NULL ? "" : server.arg("hotspot_pass"));
    wireless.loadCredentials();

    if(wireless.connection()) {
      delay(500);
      server.send(200, "text/html", connectionSuccess_html);
      updateScreen(temperature, humidity, nrOfReadingsSentToday, wireless.connection(true));
      delay(500);
    } else {
      wireless.clearCredentials();
      delay(500);
      server.send(200, "text/html", connectionError_html);
      updateScreen(temperature, humidity, nrOfReadingsSentToday, wireless.connection(true));
      delay(500);
    }
  } 
}

void handle_Setup(){
  server.send(200, "text/html", setup_html);
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}







