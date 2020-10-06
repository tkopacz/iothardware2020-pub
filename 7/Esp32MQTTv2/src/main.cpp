#include <Arduino.h>
#include <Wifi.h>
#include <PubSubClient.h>

#include <time.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#include "iot_configs.h"

//SAS
#include "SasKeyForIotHub.h"
#include <sstream>
#include <time.h>
#include "config.h"

static const char ssid[] = IOT_CONFIG_WIFI_SSID;
static const char pass[] = IOT_CONFIG_WIFI_PASSWORD;




WiFiClient wifi;
WiFiClientSecure wifissl;
PubSubClient client(wifissl);

SasKeyForIotHub sas;

    char certificates[1283] = 
		/* Baltimore */
		"-----BEGIN CERTIFICATE-----\r\n"
		"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\r\n"
		"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\n"
		"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\r\n"
		"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\r\n"
		"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\r\n"
		"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\r\n"
		"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\r\n"
		"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\r\n"
		"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\r\n"
		"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\r\n"
		"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\r\n"
		"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\r\n"
		"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\r\n"
		"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\r\n"
		"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\r\n"
		"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\r\n"
		"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\r\n"
		"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\r\n"
		"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\r\n"
		"-----END CERTIFICATE-----\r\n";



#define MIN_EPOCH (40 * 365 * 24 * 3600)
static void initTime()
{
  time_t epochTime;

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  while (true)
  {
    epochTime = time(NULL);

    if (epochTime < MIN_EPOCH)
    {
      log_w("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
      delay(2000);
    }
    else
    {
      log_i("Fetched NTP epoch time is: %dl ", epochTime);
      log_i();
      break;
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char buf[100];
  log_i("Message arrived [%s]",topic);
  int toCopy = length;
  if (toCopy > sizeof(buf)-1) toCopy = sizeof(buf)-1;
  strncpy(buf,(char *)payload,toCopy);
  buf[toCopy] = 0;
  log_i("Msg: %s",buf);
}

void afterConnected() {
  int ret;
  log_i("Connected");
  ret = client.publish(Config::MQTTSendTopic,"{\"status\":1");
  log_i("client.publish: %d",ret);
  ret = client.subscribe(Config::MQTTReceiveTopic,1);
  log_i("client.subscribe: %d",ret);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    log_i("Attempting MQTT connection...");
    // Attempt to connect

    if (client.connect(Config::DEVICE_NAME.c_str(), Config::USER_NAME, sas.get_iot_hub_sas_token().c_str())) {
      afterConnected();
    } else {
      log_e("reconnect failed, rc=%d, retry in 5s",client.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Config::LoadConfig();
  WiFi.begin(                                                                                                                                                                                                                                                                                                                 "CBA_F1", "bgh12QW2");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    log_i(".");
  }

  log_i("\r\nConnected to wifi");
  initTime();
  wifissl.setCACert(certificates);
  log_i("Init MQTT CLient");

  log_i("Server: %s",Config::MQTT_SERVER);
  client.setServer(Config::MQTT_SERVER, 8883);
  log_i("Server: %s",Config::MQTT_SERVER);
  client.setCallback(callback);
  // if (client.connect(Config::DEVICE_NAME.c_str(), Config::USER_NAME().c_str(), sas.get_iot_hub_sas_token().c_str())) {
  //   afterConnected();
  // }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); //Well - 2 cores, could be task, but...
}