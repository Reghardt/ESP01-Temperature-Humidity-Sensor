#include "wifiCredentials.h"

void saveCredentials(String ssid, String password) {
    WiFiCredentials newCredentials = {WIFI_CRED_CHECK_VALUE, "", ""};
    ssid.toCharArray(newCredentials.ssid, SSID_SIZE);
    password.toCharArray(newCredentials.password, PASSWORD_SIZE);
    EEPROM.put(0, newCredentials);
    EEPROM.commit();
}

void loadCredentials(WiFiCredentials& credentials) {
    EEPROM.get(0, credentials);
}

void deleteCredentials() {
    WiFiCredentials newCredentials = {0, "", ""};
    EEPROM.put(0, newCredentials);
    EEPROM.commit();
}