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

	static const int IoTHub_tokenExpirationPeriod = 360;

	String urlEncode(const String url);
	int decodeBase64(const String input, uint8_t* output, uint32_t outputLength);
	String encodeBase64(const uint8_t* input, int inputLength);
	String hashIt(const String data, uint8_t* key, size_t keyLength);
	void init();
public:
	SasKeyForIotHub();
	~SasKeyForIotHub();
	String get_iot_hub_sas_token();
};

