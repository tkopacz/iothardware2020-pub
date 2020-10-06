#pragma once
#include <WString.h>
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

        static char DEVICE_NAME_CSTR[100];
        static char DEVICE_KEY_CSTR[100];
        static String CONNECTION_STRING;

        static char WIFI_SSID_CSTR[100];
        static char WIFI_PASSWD_CSTR[100];


};