#include "wireless.h"




void Wireless::begin(EEPROMClass* eeprom, const char *ssid, const char *password, IPAddress local_ip, IPAddress gateway, IPAddress subnet )
{
    this->eeprom = eeprom;
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.persistent(false);
    delay(100);
}

bool Wireless::connection(bool quick)
{
    static bool isConnected = false;
    if(quick)
    {
        return isConnected;
    }

    if(WiFiMulti.run(CONNECTION_TIME_MS) == WL_CONNECTED)
    {
        Serial.print("WiFi connected: ");
        Serial.print(WiFi.SSID());
        Serial.print(" ");
        Serial.println(WiFi.localIP());
        isConnected = true;
        return true;
    }
    else
    {
        Serial.println("WiFi could not connect.");
        isConnected = false;
        return false;
    }
}

void Wireless::saveCredentials(String ssid, String password) {
    WiFiMulti.cleanAPlist();
    WiFi.disconnect();

    WiFiCredentials credentials = {EXPECTED_VALUE, "", ""};
    ssid.toCharArray(credentials.ssid, SSID_SIZE);
    password.toCharArray(credentials.password, PASSWORD_SIZE);

    eeprom->put(0, credentials);
    eeprom->commit();
}

bool Wireless::loadCredentials() {
    WiFiCredentials credentials;
    eeprom->get(0, credentials);

    Serial.println(credentials.expectedValue);
    Serial.println(credentials.ssid);
    Serial.println(credentials.password);

    if(credentials.expectedValue == EXPECTED_VALUE)
    {
        Serial.println("Credentials valid");
        WiFiMulti.addAP(credentials.ssid, credentials.password);
        return true;
    }
    else
    {
        Serial.println("Credentials invalid");
        return false;
    }
}

void Wireless::clearCredentials() {
    WiFiCredentials newCredentials = {0, "", ""};
    eeprom->put(0, newCredentials);
    eeprom->commit();

    WiFiMulti.cleanAPlist();
    WiFi.disconnect();
}