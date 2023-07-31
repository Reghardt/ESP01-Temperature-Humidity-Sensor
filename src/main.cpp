#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include "WiFiClientSecure.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "Adafruit_AM2320.h"
#include "./pages/pages.h"
#include "wifiCredentials/wifiCredentials.h"


#define CONNECTION_TIME_MS 5000
#define SEND_INTERVAL_MS 10000

#define SCREEN_I2C_ADDRESS 0x3c
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO

WiFiCredentials credentials;

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_AM2320 am2320 = Adafruit_AM2320();

// String prepareHtmlPage();
String createURL(String temp, String humidity);
void handle_NotFound();
void handle_Setup();
void handle_Connect();

void sendReading(float temperature, float humidity);
void updateScreen(float temperature, float humidity, String nrOfReadings, bool wifiConnected);

boolean createWiFiConnection();

/* Put your SSID & Password */
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

unsigned long previousMillis=0;

float temp = 0.0f;
float humid = 0.0f;
String nrOfReadings = "0";
bool wifiConnected = false;


int refreshes = 0;

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);
ESP8266WiFiMulti WiFiMulti;

void setup() {

  Wire.begin(2,0);
  Serial.begin(115200);
  EEPROM.begin(sizeof(WiFiCredentials));

  am2320.begin();

  Serial.println("Starting...");

  delay(250); // wait for the OLED to power up
  display.begin(SCREEN_I2C_ADDRESS, true); // Address 0x3C default
  display.display(); //Adafruit splash screen
  delay(2000); //wait for splash creen
  display.setTextColor(SH110X_WHITE); //needed for displaying text later on

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.persistent(false);
  delay(100);

  server.on("/", handle_Setup);
  server.on("/connect", HTTP_POST, handle_Connect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  
  loadCredentials(credentials);
  if(credentials.checkValue == WIFI_CRED_CHECK_VALUE)
  {
    Serial.println("Credentials valid");
    WiFiMulti.addAP(credentials.ssid, credentials.password);
  }

  updateScreen(am2320.readTemperature(), am2320.readHumidity(), nrOfReadings, wifiConnected); //read after splash screen
}


void loop() 
{
  server.handleClient();

  unsigned long currentMillis = millis();
  if((unsigned long)(currentMillis - previousMillis) >= SEND_INTERVAL_MS) 
  {
    temp = am2320.readTemperature();
    humid = am2320.readHumidity();

    Serial.println("temp: " + String(temp) + " humid: " + String(humid));

    if(refreshes > 6)
    {
      sendReading(temp, humid);
      refreshes = 0;
    }

    if(wifiConnected == false)
    {
      Serial.println("Attempting connection");
      createWiFiConnection();
    }

    updateScreen(temp, humid, nrOfReadings, wifiConnected);



    refreshes++;
    previousMillis = currentMillis;
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

boolean createWiFiConnection()
{
  // Serial.println("Exists " + String(WiFiMulti.existsAP("GN10", "123456789x")));
  if (WiFiMulti.run(CONNECTION_TIME_MS) == WL_CONNECTED) 
  {
    delay(500);
    Serial.print("WiFi connected: ");
    Serial.print(WiFi.SSID());
    Serial.print(" ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
    return true;
  }
  else
  {
    Serial.println("WiFi could not connect.");
    wifiConnected = false;
    return false;
  }
}

void sendReading(float temperature, float humidity)
{
  if (createWiFiConnection()) 
  {
    WiFiClientSecure client;
    HTTPClient https;

    client.setInsecure();
    
    String url = createURL(String(temperature), String(humidity));
    Serial.println("Requesting " + url);
    if (https.begin(client, url)) 
    {
      int httpCode = https.GET();
      Serial.println("Response code: " + String(httpCode));
      if (httpCode > 0) {
        nrOfReadings = https.getString();
        Serial.println(nrOfReadings);
      }
      https.end();
    } 
    else 
    {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

}

String createURL(String temp, String humidity){
  return "https://cloudy-seal-62.deno.dev/api/reading?tmp=" + temp + "&hmd=" + humidity;
}


void handle_Connect(){
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
    //TODO create wrapper for wifiCredentials and Wifi multi
    WiFiMulti.cleanAPlist();
    WiFi.disconnect();
    saveCredentials(server.arg("hotspot_name"), server.arg("hotspot_pass") == NULL ? "" : server.arg("hotspot_pass"));
    WiFiMulti.addAP(server.arg("hotspot_name").c_str(), server.arg("hotspot_pass") == NULL ? "" : server.arg("hotspot_pass").c_str());

    if(createWiFiConnection()) {
      delay(500);
      server.send(200, "text/html", connectionSuccess_html);
      delay(500);
    } else {
      Serial.println("WiFi could not connect..");

      deleteCredentials();
      WiFiMulti.cleanAPlist();
      delay(500);
      server.send(200, "text/html", connectionError_html);
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







