#pragma once
#include <Arduino.h>

#include "config.h"
#include "log.h"


#include <WiFiClient.h>
#include "WiFiClientSecure.h"

#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266mDNS.h>
    #include <ESP8266WebServer.h>  
    #include "ESP8266Preferences.h"
#else
    #include <WiFi.h>
    #include "WebServer.h"
#endif

class WWWESPSetupServer
{
private:
    IPAddress apIP;
    const char *apSSID = "WIFIESP";
#ifdef ESP8266
    ESP8266WebServer  webServer;
#else
    WebServer webServer;
#endif
    String wifi_ssid,wifi_password;
    String ssidList;
    bool hasWifi;
    bool settingMode;
    WiFiClientSecure wifissl;

    String urlDecode(String input);
    String makePage(String title, String contents);
    void setupMode();
    void startWebServer();
    bool checkConnection();
    bool restoreConfig();
    static void initTime();
public:
    WWWESPSetupServer();
    void Init();
    bool IsSettingsMode();
    bool HasWifi();
    void HandleClient();
};