#ifndef IOT_CONFIGS_H
#define IOT_CONFIGS_H

/**
 * WiFi setup
 */
#define IOT_CONFIG_WIFI_SSID            "5GBEMOWO"
#define IOT_CONFIG_WIFI_PASSWORD        "MESHbgh123QW#"

/**
 * IoT Hub Device Connection String setup
 * Find your Device Connection String by going to your Azure portal, creating (or navigating to) an IoT Hub, 
 * navigating to IoT Devices tab on the left, and creating (or selecting an existing) IoT Device. 
 * Then click on the named Device ID, and you will have able to copy the Primary or Secondary Device Connection String to this sample.
 */

//#define DEVICE_CONNECTION_STRING    "HostName=pltkdpepliot2016S1.azure-devices.net;DeviceId=ioth2020ESP32MQTT;SharedAccessKey=nD/PgMmBY3mA/8FXmGlLNK/zwQpTpvjrdoFHcOhRaIA="

//#define DEVICE_NAME "ioth2020ESP32MQTT"
//#define USER_NAME "pltkdpepliot2016S1.azure-devices.net/ioth2020ESP32MQTT"
//#define MQTT_SERVER "pltkdpepliot2016S1.azure-devices.net"

//#define MQTTSendTopic "devices/ioth2020ESP32MQTT/messages/events/"
//#define MQTTReceiveTopic  "devices/ioth2020ESP32MQTT/messages/#"
//#define MQTTReceiveTopic  "devices/ioth2020ESP32MQTT/messages/devicebound/#"


#endif /* IOT_CONFIGS_H */