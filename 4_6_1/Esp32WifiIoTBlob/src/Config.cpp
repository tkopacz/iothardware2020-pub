#include "config.h"
#include "ESPPreferences.h"
#include "log.h"
String Config::DEVICE_KEY;
String Config::DEVICE_NAME ;
String Config::IOT_HUB_NAME;
String Config::WIFI_PASSWD ;
String Config::WIFI_SSID  ;

char Config::MQTT_SERVER[100];
char Config::USER_NAME[100];
char Config::MQTTReceiveTopic[100];
char Config::MQTTSendTopic[100];
char Config::FileUploadURI[200];
char Config::NotificationURI[200];
char Config::DEVICE_NAME_CSTR[100];
char Config::DEVICE_KEY_CSTR[100];
char Config::WIFI_SSID_CSTR[100];
char Config::WIFI_PASSWD_CSTR[100];

String Config::CONNECTION_STRING; //Iot HUB
void Config::LoadConfig() {
    ESPPreferences pref;
#ifndef PRODUCTION
    pref.SetValue("WIFI_SSID","5GBEMOWO");
    pref.SetValue("WIFI_PASSWD",".....................");
    pref.SetValue("DEVICE_NAME","ioth8266r01");
    pref.SetValue("DEVICE_KEY",".................................");
    // pref.SetValue("DEVICE_NAME","ioth2020ESP32MQTT");
    // pref.SetValue("DEVICE_KEY","......................................");
    pref.SetValue("IOT_HUB_NAME","pltkdpepliot2016S1");
    pref.SetValue("CONNECTION_STRING","HostName=pltkdpepliot2016S1.azure-devices.net;DeviceId=ioth8266r01;SharedAccessKey=.........................");
    pref.WriteAll();
#endif
    //pref.Clear(); //TMP
    Config::DEVICE_KEY = pref.GetValue("DEVICE_KEY");
    Config::DEVICE_NAME = pref.GetValue("DEVICE_NAME");
    Config::IOT_HUB_NAME = pref.GetValue("IOT_HUB_NAME");
    Config::WIFI_PASSWD = pref.GetValue("WIFI_PASSWD");
    Config::WIFI_SSID = pref.GetValue("WIFI_SSID");
    Config::CONNECTION_STRING = pref.GetValue("CONNECTION_STRING");

    String str = Config::IOT_HUB_NAME + ".azure-devices.net";
    strncpy(Config::MQTT_SERVER,str.c_str(),sizeof(Config::MQTT_SERVER));
    str = Config::IOT_HUB_NAME + ".azure-devices.net/" + Config::DEVICE_NAME;
    strncpy(Config::USER_NAME,str.c_str(),sizeof(Config::USER_NAME));
    str = "devices/" + Config::DEVICE_NAME + "/messages/#";
    strncpy(Config::MQTTReceiveTopic,str.c_str(),sizeof(Config::MQTTReceiveTopic));
    str = "devices/"+ Config::DEVICE_NAME + "/messages/events/";;
    strncpy(Config::MQTTSendTopic,str.c_str(),sizeof(Config::MQTTSendTopic));
    str = "https://" + String(Config::MQTT_SERVER) + "/devices/" + Config::DEVICE_NAME + "/files?api-version=2018-06-30";
    strncpy(Config::FileUploadURI,str.c_str(),sizeof(Config::FileUploadURI));
    str = "https://" + String(Config::MQTT_SERVER) + "/devices/" + Config::DEVICE_NAME + "/files/notifications?api-version=2018-06-30";
    strncpy(Config::NotificationURI,str.c_str(),sizeof(Config::NotificationURI));

    strncpy(Config::DEVICE_NAME_CSTR,Config::DEVICE_NAME.c_str(),sizeof(Config::DEVICE_NAME_CSTR));
    strncpy(Config::DEVICE_KEY_CSTR,Config::DEVICE_KEY.c_str(),sizeof(Config::DEVICE_KEY_CSTR));

    strncpy(Config::WIFI_PASSWD_CSTR,Config::WIFI_PASSWD.c_str(),sizeof(Config::WIFI_PASSWD_CSTR));
    strncpy(Config::WIFI_SSID_CSTR,Config::WIFI_SSID.c_str(),sizeof(Config::WIFI_SSID_CSTR));
  
}



void Config::SaveConfig() {
    ESPPreferences pref;
    pref.SetValue("WIFI_SSID",Config::WIFI_SSID);
    pref.SetValue("WIFI_PASSWD",Config::WIFI_PASSWD);
    pref.SetValue("DEVICE_NAME",Config::DEVICE_NAME);
    pref.SetValue("DEVICE_KEY",Config::DEVICE_KEY);
    pref.SetValue("IOT_HUB_NAME",Config::IOT_HUB_NAME);
    pref.SetValue("CONNECTION_STRING",Config::CONNECTION_STRING);
    pref.WriteAll();
    LoadConfig();
}

void Config::Clear() {
    ESPPreferences pref;
    pref.Clear();
    pref.SetValue("WIFI_SSID","");
    pref.SetValue("WIFI_PASSWD","");
    pref.SetValue("DEVICE_NAME","");
    pref.SetValue("DEVICE_KEY","");
    pref.SetValue("IOT_HUB_NAME","");
    pref.SetValue("CONNECTION_STRING","");
    pref.WriteAll();
    LoadConfig();
}
