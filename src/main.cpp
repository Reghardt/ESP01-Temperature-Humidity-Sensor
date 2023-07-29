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
// #include <SPI.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>
// #include "Adafruit_AM2320.h"

#include "./pages/pages.h"


#define CONNECTION_TIME_MS 5000
#define SEND_INTERVAL_MS 10000

// String prepareHtmlPage();
String createURL(String temp, String humidity);
void handle_NotFound();
void handle_Home();
void handle_Connect();

/* Put your SSID & Password */
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

unsigned long previousMillis=0;


IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);
ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_Home);
  server.on("/connect", handle_Connect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() 
{
  server.handleClient();

  unsigned long currentMillis = millis();
  if((unsigned long)(currentMillis - previousMillis) >= SEND_INTERVAL_MS) 
  {
    if (WiFiMulti.run(CONNECTION_TIME_MS) == WL_CONNECTED) 
    {
      Serial.print("WiFi connected: ");
      Serial.print(WiFi.SSID());
      Serial.print(" ");
      Serial.println(WiFi.localIP());

        WiFiClientSecure client;
        client.setInsecure();
        
        HTTPClient https;
        String url = createURL("10", "30");
        Serial.println("Requesting " + url);
        if (https.begin(client, url)) {
          int httpCode = https.GET();
          Serial.println("============== Response code: " + String(httpCode));
          if (httpCode > 0) {
            Serial.println(https.getString());
          }
          https.end();
        } else {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
    }
    else {
      Serial.println("WiFi not connected!");
    }
    previousMillis = currentMillis;
  }
}

String createURL(String temp, String humidity){
  return "https://cloudy-seal-62.deno.dev/api/reading?tmp=" + temp + "&hmd=" + humidity;
}


void handle_Connect(){

  server.hasArg();
  
  WiFiMulti.cleanAPlist();
  WiFiMulti.addAP("GN10", "123456789x");

  if(WiFiMulti.run(CONNECTION_TIME_MS) == WL_CONNECTED) {
    Serial.print("WiFi connected: ");
    Serial.print(WiFi.SSID());
    Serial.print(" ");
    Serial.println(WiFi.localIP());
    delay(500);
    server.send(200, "text/plain", "Sucessfully Connected!");
    delay(500);
  } else {
    Serial.println("WiFi not connected!");
    delay(500);
    server.send(200, "text/plain", "Could not connect...");
    delay(500);
    WiFiMulti.cleanAPlist();
  }  
}

void handle_Home(){
  server.send(200, "text/html", connect_html);
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


// String prepareHtmlPage()
// {
//   String htmlPage;
//   htmlPage.reserve(1024);               // prevent ram fragmentation
//   htmlPage = F("HTTP/1.1 200 OK\r\n"
//                "Content-Type: text/html\r\n"
//                "Connection: close\r\n"  // the connection will be closed after completion of the response
//                "Refresh: 5\r\n"         // refresh the page automatically every 5 sec
//                "\r\n"
//                "<!DOCTYPE HTML>"
//                "<html>"
//                "Analog input:  ");
//   htmlPage += analogRead(A0);
//   htmlPage += F("</html>"
//                 "\r\n");
//   return htmlPage;
// }