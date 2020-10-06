#include "config.h"

String Config::DEVICE_KEY ="...........";
String Config::DEVICE_NAME="................" ;
String Config::IOT_HUB_NAME = "pltkdpepliot2016S1";
String Config::WIFI_PASSWD="..............." ;
String Config::WIFI_SSID="..................";
char Config::MQTT_SERVER[100];
char Config::USER_NAME[100];
char Config::MQTTReceiveTopic[100];
char Config::MQTTSendTopic[100];
char Config::FileUploadURI[200];
char Config::NotificationURI[200];
void Config::LoadConfig() {
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

    

}

void Config::SaveConfig() {
}

void Config::Clear() {
}

