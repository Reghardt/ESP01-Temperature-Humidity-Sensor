
#ifndef WIRELESS_H
#define WIRELESS_H

#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFiMulti.h>

#define SSID_SIZE 48
#define PASSWORD_SIZE 48
#define EXPECTED_VALUE 1234
#define CONNECTION_TIME_MS 5000

struct WiFiCredentials {
  int expectedValue; //value used to see if the struct exists in eeprom. If checkValue returns junk we know it does not, if it returns the expected value we know it exists. 1234 used for exists (arbitrary). any other value means it does not
  char ssid[SSID_SIZE];
  char password[PASSWORD_SIZE];
};

class Wireless
{
    private:
    EEPROMClass* eeprom;
    ESP8266WiFiMulti WiFiMulti;

    public:
    void begin(EEPROMClass* eeprom, const char *ssid, const char *password, IPAddress local_ip, IPAddress gateway, IPAddress subnet );
    
    bool connection(bool quick = false);
    void saveCredentials(String ssid, String password);
    bool loadCredentials();
    void clearCredentials();
};


#endif