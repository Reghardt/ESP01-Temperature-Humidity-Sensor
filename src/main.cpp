#include <Arduino.h>
// #include <SPI.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>
// #include "Adafruit_AM2320.h"


// #define SCREEN_I2C_ADDRESS 0x3c
// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 64 // OLED display height, in pixels
// #define OLED_RESET -1   //   QT-PY / XIAO
// Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Adafruit_AM2320 am2320 = Adafruit_AM2320();

// void setup()   {
//   Wire.begin(2,0);
//   Serial.begin(9600);

//   am2320.begin();

//   delay(250); // wait for the OLED to power up
//   display.begin(SCREEN_I2C_ADDRESS, true); // Address 0x3C default
//   display.display(); //Adafruit splash screen
//   delay(2000); //wait for splash creen
//   display.setTextColor(SH110X_WHITE); //needed for displaying text later on
// }

// void loop() {

//   float temp = am2320.readTemperature();
//   float humid = am2320.readHumidity();

//   display.clearDisplay();
//   display.setCursor(0, 0);
//   display.setTextSize(4);
//   display.print(temp, 1);
//   display.setTextSize(3);
//   display.println('C');

//   display.setCursor(0, 40);
//   display.setTextSize(3);
//   humid >= 99 ? display.print(99) : display.print(humid, 0);
//   display.print("%");

//   display.display();

//   delay(10000);
// }

// #include "ESP8266WiFi.h"
// #include "ESP8266WiFiMulti.h"
// #include "ESP8266HTTPClient.h"
// #include "WiFiClientSecure.h"

// String url = "https://cloudy-seal-62.deno.dev/api/reading?data=10";



// void setup()
// {
//   Serial.begin(9600);

//   WiFi.begin("GN10", "123456789x");

//   while (WiFi.status() != WL_CONNECTED) 
//   {
//     delay(500);
//     Serial.print("*");
//   }
  
//   pinMode(LED_BUILTIN, OUTPUT);
// }

// void loop()
// {

//   if (WiFi.status() == WL_CONNECTED) {
//     WiFiClientSecure client;
//     client.setInsecure();
    
//     HTTPClient https;
//     Serial.println("Requesting " + url);
//     if (https.begin(client, url)) {
//       int httpCode = https.GET();
//       Serial.println("============== Response code: " + String(httpCode));
//       if (httpCode > 0) {
//         Serial.println(https.getString());
//       }
//       https.end();
//     } else {
//       Serial.printf("[HTTPS] Unable to connect\n");
//     }
//   }
//   delay(5000);
//   Serial.println("done");
  // client.connect()

  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(1000);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(1000);

// }

// #include <ESP8266WiFiMulti.h>

// void setup()
// {
  // Serial.begin(115200);
  // Serial.println();

  // WiFi.begin("GN10", "123456789x");



  // Serial.print("Connecting");
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println();

  // Serial.print("Connected, IP address: ");
  // Serial.println(WiFi.localIP());
// }

// void loop() {}


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


#define CONNECTION_TIME_MS 5000
#define SEND_INTERVAL_MS 10000

#define SCREEN_I2C_ADDRESS 0x3c
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_AM2320 am2320 = Adafruit_AM2320();

// String prepareHtmlPage();
String createURL(String temp, String humidity);
void handle_NotFound();
void handle_Setup();
void handle_Connect();

void sendReading();
void updateScreen(float temperature, float humidity);

boolean createWiFiConnection();

/* Put your SSID & Password */
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

unsigned long previousMillis=0;

float temp = 0.0f;
float humid = 0.0f;

int refreshes = 0;
bool wifiConnected = false;
int nrOfReadings = 0;

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);
ESP8266WiFiMulti WiFiMulti;

void setup() {

  Wire.begin(2,0);
  Serial.begin(115200);

  am2320.begin();

  Serial.println("Starting...");

  delay(250); // wait for the OLED to power up
  display.begin(SCREEN_I2C_ADDRESS, true); // Address 0x3C default
  display.display(); //Adafruit splash screen
  delay(2000); //wait for splash creen
  display.setTextColor(SH110X_WHITE); //needed for displaying text later on

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.persistent(true);
  delay(100);

  server.on("/", handle_Setup);
  server.on("/connect", HTTP_POST, handle_Connect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  WiFiMulti.addAP("GN10", "123456789x");

  updateScreen(am2320.readTemperature(), am2320.readHumidity()); //read after splash screen
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
      sendReading();
      refreshes = 0;
    }

    if(wifiConnected == false)
    {
      Serial.println("Attempting connection");
      createWiFiConnection();
    }

    updateScreen(temp, humid);



    refreshes++;
    previousMillis = currentMillis;
  }
}

void updateScreen(float temperature, float humidity)
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
  display.print("R:" + String(99));

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

void sendReading()
{
  if (createWiFiConnection()) 
  {
    WiFiClientSecure client;
    HTTPClient https;

    client.setInsecure();
    
    String url = createURL("10", "30");
    Serial.println("Requesting " + url);
    if (https.begin(client, url)) 
    {
      int httpCode = https.GET();
      Serial.println("Response code: " + String(httpCode));
      if (httpCode > 0) {
        Serial.println(https.getString());
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
    WiFiMulti.cleanAPlist();
    WiFiMulti.addAP(server.arg("hotspot_name").c_str(), server.arg("hotspot_pass") == NULL ? "" : server.arg("hotspot_pass").c_str());

    if(createWiFiConnection()) {
      delay(500);
      server.send(200, "text/html", connectionSuccess_html);
      delay(500);
    } else {
      Serial.println("WiFi could not connect..");

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
