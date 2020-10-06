#include <Arduino.h>
#include "Esp.h"
#include "WiFi.h"
#include <stdio.h>
#include <stdlib.h>

//External Libbb
#include <ArduinoJson.h>


#include <AzureIoTHub.h>
#include "iot_configs.h"
#ifdef USE_MQTT
    #include "AzureIoTProtocol_MQTT.h"
    #include "iothubtransportmqtt.h"
#endif
#ifdef USE_HTTP
    #include "AzureIoTProtocol_HTTP.h"
    #include "iothubtransporthttp.h"
#endif

#include "esp_task_wdt.h"

#include "TelemetryJson.h"

//Hardware
#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library
#include "ClosedCube_HDC1080.h"
//HALL - build in
//Internal temp
#ifdef __cplusplus
  extern "C" {
#endif
    uint8_t temprature_sens_read();
#ifdef __cplusplus
  }
#endif

//Logging
static const char* TAG = "ESP322000IOTHUB";
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
void esp32_sample_init(const char* ssid, const char* password);

static const char ssid[] = IOT_CONFIG_WIFI_SSID;
static const char pass[] = IOT_CONFIG_WIFI_PASSWORD;
static IPAddress localIP;

static const char* connectionString = DEVICE_CONNECTION_STRING;
//-------
#define MESSAGE_MAX_LEN 200
#define DEVICE_ID "ESP322000IOTHUB"
volatile static bool hasIoTHub = false;
volatile static bool hasWifi = false;
static bool messageSending = true;
static uint64_t send_interval_ms;
int IntervalMs = 10000; //10000;

//-------
static long callbackCounter;
static long messageCount = 0;
IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
IOTHUB_MESSAGE_HANDLE msg;
int receiveContext = 0;
static char propText[1024];

//Main message
TelemetryJsonV2 t;
//Timer MUX
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


//-------------
static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
  int *counter = (int *)userContextCallback;
  const char *buffer;
  size_t size;
  MAP_HANDLE mapProperties;
  const char *messageId;
  const char *correlationId;
  const char *userDefinedContentType;
  const char *userDefinedContentEncoding;

  // Message properties
  if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
  {
    messageId = "<null>";
  }

  if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
  {
    correlationId = "<null>";
  }

  if ((userDefinedContentType = IoTHubMessage_GetContentTypeSystemProperty(message)) == NULL)
  {
    userDefinedContentType = "<null>";
  }

  if ((userDefinedContentEncoding = IoTHubMessage_GetContentEncodingSystemProperty(message)) == NULL)
  {
    userDefinedContentEncoding = "<null>";
  }

  // Message content
  if (IoTHubMessage_GetByteArray(message, (const unsigned char **)&buffer, &size) != IOTHUB_MESSAGE_OK)
  {
    LogError("unable to retrieve the message data\r\n");
  }
  else
  {
    LogInfo("Received Message [%d]\r\n Message ID: %s\r\n Correlation ID: %s\r\n Content-Type: %s\r\n Content-Encoding: %s\r\n Data: <<<%.*s>>> & Size=%d\r\n",
                 (*counter), messageId, correlationId, userDefinedContentType, userDefinedContentEncoding, (int)size, buffer, (int)size);
  }

  // Retrieve properties from the message
  mapProperties = IoTHubMessage_Properties(message);
  if (mapProperties != NULL)
  {
    const char *const *keys;
    const char *const *values;
    size_t propertyCount = 0;
    if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
    {
      if (propertyCount > 0)
      {
        size_t index;

        LogInfo(" Message Properties:\r\n");
        for (index = 0; index < propertyCount; index++)
        {
          LogInfo("\tKey: %s Value: %s\r\n", keys[index], values[index]);
        }
        LogInfo("\r\n");
      }
    }
  }

  /* Some device specific action code goes here... */
  (*counter)++;
  return IOTHUBMESSAGE_ACCEPTED;
}

static int DeviceMethodCallback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response, size_t *resp_size, void *userContextCallback)
{
  (void)userContextCallback;

  LogInfo("\r\nDevice Method called\r\n");
  LogInfo("Device Method name:    %s\r\n", method_name);
  LogInfo("Device Method payload: %.*s\r\n", (int)size, (const char *)payload);

  int status = 200;
  char *RESPONSE_STRING = "{ \"Response\": \"OK\" }";

  if (strcmp(method_name, "start") == 0)
  {
    messageSending = true;
  }
  else if (strcmp(method_name, "stop") == 0)
  {
    messageSending = false;
  }
  else if (strcmp(method_name, "delay") == 0)
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    // Test if parsing succeeds.
    if (error)
    {
      LogError("deserializeJson() failed: %s", error.c_str());
      RESPONSE_STRING = "{ \"Response\": \"deserializeJson() failed\" }";
      status = 500;
    }
    else
    {
      IntervalMs = doc["ms"];
      LogInfo("IntervalMs:%d", IntervalMs);
    }
  }
  else if (strcmp(method_name, "ledon") == 0)
  {
    //digitalWrite(M5_LED, LOW);
  }
  else if (strcmp(method_name, "ledoff") == 0)
  {
    //digitalWrite(M5_LED, HIGH);
  }
  else
  {
    LogInfo("No method %s found", method_name);
    RESPONSE_STRING = "{ \"Response\": \"No method found\" }";
    status = 404;
  }

  LogInfo("\r\nResponse status: %d\r\n", status);
  LogInfo("Response payload: %s\r\n\r\n", RESPONSE_STRING);

  *resp_size = strlen(RESPONSE_STRING);
  if ((*response = (unsigned char *)malloc(*resp_size)) == NULL)
  {
    status = -1;
  }
  else
  {
    (void)memcpy(*response, RESPONSE_STRING, *resp_size);
  }
  return status;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
  //Setup watchdog
  //esp_task_wdt_reset();
  //Passing address to IOTHUB_MESSAGE_HANDLE
  IOTHUB_MESSAGE_HANDLE *msg = (IOTHUB_MESSAGE_HANDLE *)userContextCallback;
  LogInfo("Confirmation %d result = %s\r\n", callbackCounter, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
  /* Some device specific action code goes here... */
  callbackCounter++;
  if (result != IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    //esp_restart();
  }

  //TK:Or caller
  IoTHubMessage_Destroy(*msg);
}

static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    (void)reason;
    (void)user_context;
    // This sample DOES NOT take into consideration network outages.
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        LogInfo("The device client is connected to iothub\r\n");
    }
    else
    {
        LogInfo("The device client has been disconnected\r\n");
    }
}

//-------
//Hardware
CCS811 ccs811(02); //VDD to 3V3, GND to GND, SDA to 21, SCL to 22, nWAKE to IO2 (2)
ClosedCube_HDC1080 hdc1080;


//--------------Timer
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  t.SetValue("random01",random(100));
  t.SetValue("random02",random(100));
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 
  if( errstat==CCS811_ERRSTAT_OK ) { 
    t.SetValue("eco2",eco2);
    t.SetValue("etvoc",etvoc);
  }
  t.SetValue("tempearture",hdc1080.readTemperature());
  t.SetValue("humidity",hdc1080.readHumidity());

  t.SetValue("hall",hallRead());
  t.SetValue("InternalTemp", temprature_sens_read());
  portEXIT_CRITICAL_ISR(&timerMux);
 
}
//-------------Other events 



void setup() {
  //esp_log_level_set(TAG,ESP_LOG_DEBUG);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.print("Begin ESP32, \r\n");

  esp32_sample_init(ssid,pass); //Serial, WIFI, TIME
  Serial.begin(115200);
  Serial.print("next, \r\n");

  //Setup WiFi
  hasWifi = true;
  localIP = WiFi.localIP();
  LogInfo("WiFi connected, IP address: %s",localIP.toString().c_str());

  randomSeed(analogRead(0));

  LogInfo(" > IoT Hub");

  LogInfo("Before platform_init\n");
  (void)IoTHub_Init();
 
  #ifdef USE_MQTT
      IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol = MQTT_Protocol;
  #endif
  #ifdef USE_HTTP
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol = HTTP_Protocol;
  #endif
  
  LogInfo("Before IoTHubClient_LL_CreateFromConnectionString\n");
  if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, protocol)) == NULL)
  {
    LogError("ERROR: iotHubClientHandle is NULL!\r\n");
  }
  else
  {
    // Set any option that are neccessary.
    // For available options please see the iothub_sdk_options.md documentation in the main C SDK
    // turn off diagnostic sampling
    int diag_off = 0;
    IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_DIAGNOSTIC_SAMPLING_PERCENTAGE, &diag_off);

#ifndef USE_HTTP
    // Example sdk status tracing for troubleshooting
    bool traceOn = true;
    IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_LOG_TRACE, &traceOn);
#endif // 

    // Setting the Trusted Certificate.
    IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_TRUSTED_CERT, certificates);

#if defined USE_MQTT
    //Setting the auto URL Encoder (recommended for MQTT). Please use this option unless
    //you are URL Encoding inputs yourself.
    //ONLY valid for use with MQTT
    // bool urlEncodeOn = true;
    // IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);

    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
    {
      LogError("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
    }

    if (IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, DeviceMethodCallback, &receiveContext) != IOTHUB_CLIENT_OK)
    {
      LogError("ERROR: IoTHubClient_LL_SetDeviceMethodCallback..........FAILED!\r\n");
    }

#endif // 
    // Setting connection status callback to get indication of connection to iothub
    (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(iotHubClientHandle, connection_status_callback, NULL);

  }
  hasIoTHub = true;
  // //Set Interrupt
  // timer = timerBegin(0, 80, true);
  // timerAttachInterrupt(timer, &onTimer, true);
  // ///////1000000 = 1s
  // timerAlarmWrite(timer, 10 * 1000000, true);
  // timerAlarmEnable(timer);
  
  delay(2000);
  send_interval_ms = millis();

  LogInfo("Hardware init\r\n");
  // Enable I2C
  Wire.begin(); 
  
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) LogError("setup: CCS811 begin FAILED");
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) LogError("setup: CCS811 start FAILED"); 
  hdc1080.begin(0x40);


  LogInfo(" End Setup \r\n");
}

void loopIoTHub()
{
  if (hasWifi && hasIoTHub)
  {
    if (messageSending && (int)(millis() - send_interval_ms) >= IntervalMs)
    {
      LogInfo(" loopIoTHub - begin \r\n");
      // Send data
      const char *messagePayload;
      t.SetValue("deviceId",DEVICE_ID);
      t.SetValue("messageId",messageCount);

      t.SetValue("random01",random(100));
      t.SetValue("random02",random(100));
      uint16_t eco2, etvoc, errstat, raw;
      ccs811.read(&eco2,&etvoc,&errstat,&raw); 
      if( errstat==CCS811_ERRSTAT_OK ) { 
        t.SetValue("eco2",eco2);
        t.SetValue("etvoc",etvoc);
      }
      t.SetValue("tempearture",hdc1080.readTemperature());
      t.SetValue("humidity",hdc1080.readHumidity());

      t.SetValue("hall",hallRead());

      LogInfo(" loopIoTHub - after SetValue \r\n");
      messagePayload = t.GetJson();

      LogInfo("Msg:%s",messagePayload);
      if ((msg = IoTHubMessage_CreateFromByteArray((const unsigned char *)messagePayload, strlen(messagePayload))) == NULL)
      {
        LogInfo("ERROR: iotHubMessageHandle is NULL!\r\n");
      }
      else
      {
        LogInfo(" loopIoTHub - setup message 1 \r\n");
        char buf[20];
        snprintf(buf,sizeof(buf),"%d",messageCount);
        (void)IoTHubMessage_SetMessageId(msg, buf);
        //(void)IoTHubMessage_SetCorrelationId(msg, "CORE_ID");
        (void)IoTHubMessage_SetContentTypeSystemProperty(msg, "application%2Fjson");
        (void)IoTHubMessage_SetContentEncodingSystemProperty(msg, "utf-8");

        MAP_HANDLE propMap = IoTHubMessage_Properties(msg);
        (void)sprintf_s(propText, sizeof(propText), (random(100)) ? "true" : "false");
        if (Map_AddOrUpdate(propMap, "valAlert", propText) != MAP_OK)
        {
          LogError("ERROR: Map_AddOrUpdate Failed!\r\n");
        }

        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, msg, SendConfirmationCallback, &msg) != IOTHUB_CLIENT_OK)
        {
          LogError("ERROR: IoTHubClient_SendEventAsync..........FAILED!\r\n");
        }
        messageCount++;
      }
      IOTHUB_CLIENT_STATUS status;
      IoTHubClient_LL_DoWork(iotHubClientHandle);
      ThreadAPI_Sleep(10);
      //Wait till
      while ((IoTHubClient_LL_GetSendStatus(iotHubClientHandle, &status) == IOTHUB_CLIENT_OK) && (status == IOTHUB_CLIENT_SEND_STATUS_BUSY))
      {
        IoTHubClient_LL_DoWork(iotHubClientHandle);
        ThreadAPI_Sleep(100);
      }
      //Callback is responsible for destroying message
      //IoTHubMessage_Destroy(msg);
      ThreadAPI_Sleep(10);
      send_interval_ms = millis();
      LogInfo(" loopIoTHub - end \r\n");
    }
    else
    {
    }
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    //esp_task_wdt_reset();
  }
}

void loop() {
  loopIoTHub();
}