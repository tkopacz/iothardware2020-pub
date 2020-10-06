#pragma once
#include <iostream>
#include <string>
//
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>


class SasKeyForIotHub
{
private:
	String CODES = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

	String IoTHub_name = "pltkdpepliot2016S1";//  # IoT Hub name
	String IoTHub_key = "....................";// # IoT Hub primary key
	String IoTHub_deviceId = "...............";// # IoT Hub device id
	String IoTHub_resourceURI = IoTHub_name + ".azure-devices.net";
	static const int IoTHub_tokenExpirationPeriod = 360;
	String IoTHub_apiVersion = "2018-06-30";

	String fileUploadRequestURI = "https://" + IoTHub_resourceURI + "/devices/" + IoTHub_deviceId + "/files?api-version=" + IoTHub_apiVersion;
	String notificationURI = "https://" + IoTHub_resourceURI + "/devices/" + IoTHub_deviceId + "/files/notifications?api-version=" + IoTHub_apiVersion;


	String urlEncode(const String url);
	int decodeBase64(const String input, uint8_t* output, uint32_t outputLength);
	String encodeBase64(const uint8_t* input, int inputLength);
	String hashIt(const String data, uint8_t* key, size_t keyLength);
	void init();
public:
	SasKeyForIotHub();
	SasKeyForIotHub(const String name, const String key, const String devicesId);
	~SasKeyForIotHub();
	String get_iot_hub_sas_token();
	String getFileUploadURI();
	String getNotificationURI();
};

