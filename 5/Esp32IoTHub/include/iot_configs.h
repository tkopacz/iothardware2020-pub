#ifndef IOT_CONFIGS_H
#define IOT_CONFIGS_H

/**
 * WiFi setup
 */
#define IOT_CONFIG_WIFI_SSID            "..................."
#define IOT_CONFIG_WIFI_PASSWORD        "................"

/**
 * IoT Hub Device Connection String setup
 * Find your Device Connection String by going to your Azure portal, creating (or navigating to) an IoT Hub, 
 * navigating to IoT Devices tab on the left, and creating (or selecting an existing) IoT Device. 
 * Then click on the named Device ID, and you will have able to copy the Primary or Secondary Device Connection String to this sample.
 */
#define DEVICE_CONNECTION_STRING    "HostName=pltkdpepliot2016S1.azure-devices.net;DeviceId=ioth2020esp32;SharedAccessKey=................................."

// The protocol you wish to use should be uncommented
//
#define USE_MQTT
#define SAMPLE_MQTT
//#define USE_HTTP

#endif /* IOT_CONFIGS_H */