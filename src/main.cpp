#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>


#define NETWORK_NAME "GN10"
#define NETWORK_PASS "123456789x"

void handleRoot();

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to");
  Serial.println(NETWORK_NAME);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  // MDNS.addService() //What does this do?

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  server.handleClient();
}

void handleRoot()
{
  server.send(200, "text/html", "Hello World from ESP!");
}