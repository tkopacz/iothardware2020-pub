#pragma once
#include <Arduino.h>

class Config {
    public:
        static void LoadConfig();
        static void SaveConfig();
        static void Clear();
        static String WIFI_SSID;
        static String WIFI_PASSWD;
        static String DEVICE_NAME;
        static String DEVICE_KEY;
        static String IOT_HUB_NAME;

        static char MQTT_SERVER[100];
        static char USER_NAME[100];
        static char MQTTReceiveTopic[100];
        static char MQTTSendTopic[100];
        static char FileUploadURI[200];
        static char NotificationURI[200];
};