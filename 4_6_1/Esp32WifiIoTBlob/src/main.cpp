#include <Arduino.h>
#include "WWWESPSetupServer.h"
#include "log.h"
#include "IoTBlobHelper.h"



WWWESPSetupServer www;
IoTBlobHelper blob;
void setup() {
  Serial.begin(115200);
  #ifdef DEBUG_ESP_PORT
  Serial.setDebugOutput(true);
  #endif
  delay(5000);
  DEBUG_MSG("START");
  DEBUG_MSG("LoadConfig form EEPROM");
  Config::LoadConfig();


  DEBUG_MSG("Init WiFi / WebServer");
  www.Init();
  if (www.HasWifi()) {
    //Init devices
    DEBUG_MSG("Wifi, try to upload to azure");
    blob.UploadBlob("20201006.txt","ABC"); //ioth8266r01
  } else {
    ERROR_MSG("WiFi needs to be configured!");
  }

}

void loopIoTHub() {
  // if (!client.connected()) {
  //   reconnect();
  // }
  // client.loop();

}

void loop() {
  if (www.IsSettingsMode())
  {
  }
  else
  {
    loopIoTHub();
  }
  www.HandleClient();
}
