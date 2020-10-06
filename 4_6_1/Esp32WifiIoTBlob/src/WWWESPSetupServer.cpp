#include "WWWESPSetupServer.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////

int LogicalToPhysicalPin(int pin);

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
void WWWESPSetupServer::initTime()
{
  time_t epochTime;

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  while (true)
  {
    epochTime = time(NULL);

    if (epochTime < MIN_EPOCH)
    {
      ERROR_MSG("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
      delay(2000);
    }
    else
    {
      DEBUG_MSG("Fetched NTP epoch time is: %ld \r\n", epochTime);
      break;
    }
  }
}


bool WWWESPSetupServer::restoreConfig()
{
  //WiFi.beginSmartConfig(); //Wait for magic package - only ESP32
  if (Config::WIFI_SSID.length() > 0)
  {
    wifi_ssid = Config::WIFI_SSID;
    wifi_password = Config::WIFI_PASSWD;
    //Serial.println(wifi_ssid.c_str());
    //Serial.println(wifi_password.c_str());
    //ESP32 Arduino Bug
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    delay(1000);
    WiFi.disconnect();
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    return true;
  }
  else
  {
    return false;
  }
}

bool WWWESPSetupServer::checkConnection()
{
  int count = 0;
  DEBUG_MSG(Config::WIFI_SSID_CSTR);
  while (count < 50)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      hasWifi = true;
      DEBUG_MSG("CONNECTED");
      DEBUG_MSG("HasWiFi");
      DEBUG_MSG(WiFi.localIP().toString().c_str());

      initTime();
      DEBUG_MSG("Time - done");
#ifdef ESP8266
      wifissl.setCACert((const uint8_t*)certificates,sizeof(certificates));
#else
      wifissl.setCACert((const char*)certificates);
#endif

      DEBUG_MSG("SSL - done");
      return (true);
    }
    Serial.print('.');
    delay(1000);
    count++;
  }
  hasWifi = false;
  return false;
}

//Nasty, but ESP.reset() doesnt't reset fully WiFi
void (*addressZero)() = 0;

void WWWESPSetupServer::startWebServer()
{
  if (settingMode)
  {
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    //http://192.168.41.1/settings
    webServer.on("/settings", [this]() {
      String s = "<h1>Settings</h1><p>Please select WiFi network, enter password, enter IoT Hub connection details and click <b>Submit<b> button.</p>";
      s += "<form method=\"GET\" action=\"set\"><br><b>WiFi connection:</b><br><label>SSID: </label><select name=\"WIFI_SSID\">";
      s += ssidList;
      s += "</select>";
      s += "<br><label>Password:</label><input name=\"WIFI_PASSWD\" length=64 type=\"password\"><br><b>Iot Hub connection:</b><br><label>Device:</label><input name=\"DEVICE_NAME\" length=64 type=\"text\"><br><label>Key:</label><input name=\"DEVICE_KEY\" length=100 type=\"password\"><br><label>Hub:</label><input name=\"IOT_HUB_NAME\" length=64 type=\"text\"><br><label>Cnn:</label><input name=\"CONNECTION_STRING\" length=200 type=\"password\">";
      s += "<br><b>Others:</b>";
      // s += "<br><label>INTERVAL_MS_STR (ms,10000):</label><input name=\"INTERVAL_MS_STR\" length=10 type=\"text\">";
      // s += "<br><label>INTERVAL_DEEP_SLEEP_S_STR (s, 180)</label><input name=\"INTERVAL_DEEP_SLEEP_S_STR\" length=10 type=\"text\">";
      // s += "<br><label>DEEP_SLEEP_STR (0,1):</label><input name=\"DEEP_SLEEP_STR\" length=1 type=\"text\">";
      // s += "<br><label>NORMAL_SLEEP_STR (0,1):</label><input name=\"NORMAL_SLEEP_STR\" length=1 type=\"text\">";
      // s += "<br><label>WD_TIMEOUT_STR (s,90):</label><input name=\"WD_TIMEOUT_STR\" length=10 type=\"text\">";


      s += "<br><input type=\"submit\"></form>";
      s += "<p>Or for advanced users, http://192.168.4.1/set?WIFI_SSID=<i>p1</i>&WIFI_PASSWD=<i>p2</i>&DEVICE_NAME=<i>p2</i>&DEVICE_KEY=<i>p2</i>&IOT_HUB_NAME=<i>p2..... + others</i></p>";
      //http://192.168.4.1/set?WIFI_SSID=5GBEMOWO&WIFI_PASSWD=MESHbgh123QW%23&DEVICE_NAME=ioth8266r01&DEVICE_KEY=emDiLK9SiWnIfl0iDBHVrpELlTw3W4F7PsySXeuFqyg%3D&IOT_HUB_NAME=pltkdpepliot2016S1
      //http://192.168.4.1/set?WIFI_SSID=5GBEMOWO&WIFI_PASSWD=MESHbgh123QW%23&DEVICE_NAME=ioth8266r01&DEVICE_KEY=emDiLK9SiWnIfl0iDBHVrpELlTw3W4F7PsySXeuFqyg%3D&IOT_HUB_NAME=pltkdpepliot2016S1
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/set", [this]() {
      Config::WIFI_SSID = urlDecode(webServer.arg("WIFI_SSID"));
      Config::WIFI_PASSWD = urlDecode(webServer.arg("WIFI_PASSWD"));
      Config::DEVICE_NAME = urlDecode(webServer.arg("DEVICE_NAME"));
      Config::DEVICE_KEY = urlDecode(webServer.arg("DEVICE_KEY"));
      Config::IOT_HUB_NAME = urlDecode(webServer.arg("IOT_HUB_NAME"));
      Config::CONNECTION_STRING = urlDecode(webServer.arg("CONNECTION_STRING"));
      Config::SaveConfig();

      String s = "<h1>Setup complete.</h1><p>device " + Config::DEVICE_NAME + " will be connected to \"";
      s += Config::WIFI_SSID;
      s += "\" after the restart in 3 second.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
      //addressZero();
    });
    webServer.onNotFound([this]() {
      Serial.println("NOT FOUND");
      String s = "<h1>AP mode, 90s watchdog</h1><p><a href=\"/settings\">Settings</a></p>";
      webServer.send(200, "text/html", makePage("AP (Access Point) mode", s));
    });
  }
  else
  {
    webServer.on("/", [this]() {
      String s = "<h1>STA (Client) mode, 90s watchdog</h1><p>Connected to WiFi. <a href=\"/reset\">Reset settings</a></p>";
      webServer.send(200, "text/html", makePage("STA (Client) mode", s));
    });
    webServer.on("/reset", [this]() {
      Config::Clear();
      String s = "<h1>Settings was reset.</h1><p>Device will restart in 3 seconds.</p><p>Please, turn off (and turn on) manually if needed</p>";
      webServer.send(200, "text/html", makePage("Reset settings", s));
      delay(2000);
      // WiFi.disconnect(true,true);
      // WiFi.mode(WIFI_OFF);
      delay(1000);
      ESP.restart();
    });
  }
  //Common
  webServer.on("/GPIOSET",[this](){
      String pin = urlDecode(webServer.arg("pin"));
      String state = urlDecode(webServer.arg("state"));
      int iPin = pin.toInt();
      int iState = state.toInt();
      if (iState!=0) iState = 1;
      int pinPhysical = iPin; //LogicalToPhysicalPin(iPin);

      digitalWrite(pinPhysical,iState);
      char buf[100];
      snprintf(buf,sizeof(buf),"<h1>GPIO - SET</h1><p>Logical: %d, Physical: %d = %d</p>",iPin, pinPhysical ,iState);
      webServer.send(200, "text/html", makePage("GPIO - SET", buf));
  });
  webServer.on("/current", [this]() {
    String s = "<h1>Settings</h1>";
    s+= "<p>" + Config::WIFI_SSID + "</p>";
    s+= "<p>" + Config::WIFI_PASSWD + "</p>";
    s+= "<p>" + Config::DEVICE_NAME + "</p>";
    s+= "<p>" + Config::DEVICE_KEY + "</p>";
    s+= "<p>" + Config::IOT_HUB_NAME + "</p>";
    webServer.send(200, "text/html", makePage("STA (Client) mode", s));
  });
  webServer.begin();
}

void WWWESPSetupServer::setupMode()
{
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  for (int i = 0; i < n; ++i)
  {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  WiFi.mode(WIFI_AP);
  startWebServer();
}

String WWWESPSetupServer::makePage(String title, String contents)
{
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

String WWWESPSetupServer::urlDecode(String input)
{
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}

WWWESPSetupServer::WWWESPSetupServer():apIP(IPAddress(192, 168, 4, 1)) {
}

void WWWESPSetupServer::Init() {
  Config::LoadConfig();
  delay(10);
  if (restoreConfig())
  {
    if (checkConnection())
    {
      settingMode = false;
      startWebServer();
    }
  }  
  if (!hasWifi)
  {
    settingMode = true;
    setupMode();
  }  
}

void WWWESPSetupServer::HandleClient() {
  webServer.handleClient();
}

bool WWWESPSetupServer::IsSettingsMode() {
  return settingMode;
}
bool WWWESPSetupServer::HasWifi() {
  return hasWifi;

}