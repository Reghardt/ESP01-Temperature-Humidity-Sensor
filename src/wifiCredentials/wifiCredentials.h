#ifndef WIFI_CREDENTIALS_H
#define WIFI_CREDENTIALS_H

#include <Arduino.h>
#include <EEPROM.h>

#define SSID_SIZE 48
#define PASSWORD_SIZE 48
#define WIFI_CRED_CHECK_VALUE 1234

struct WiFiCredentials {
  int checkValue; //value used to see if the struct exists in eeprom. If checkValue returns junk we know it does not, if it returns the expected value we know it exists. 1234 used for exists (arbitrary). any other value means it does not
  char ssid[SSID_SIZE];
  char password[PASSWORD_SIZE];
};

void saveCredentials(String ssid, String password);
void loadCredentials(WiFiCredentials& credentials);
void deleteCredentials();

#endif