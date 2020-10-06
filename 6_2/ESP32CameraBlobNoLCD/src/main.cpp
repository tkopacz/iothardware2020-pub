#include <Arduino.h>
#include "esp_task_wdt.h"
#include "IoTBlobHelper.h"

#include <time.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#include "esp_camera.h"

//Camera
// Return the minimum of two values a and b
#define minimum(a, b) (((a) < (b)) ? (a) : (b))

#define UPLOAD_TIMESTAMP 10000

#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 4
#define SIOD_GPIO_NUM 18
#define SIOC_GPIO_NUM 23

#define Y9_GPIO_NUM 36
#define Y8_GPIO_NUM 37
#define Y7_GPIO_NUM 38
#define Y6_GPIO_NUM 39
#define Y5_GPIO_NUM 35
#define Y4_GPIO_NUM 26
#define Y3_GPIO_NUM 13
#define Y2_GPIO_NUM 34
#define VSYNC_GPIO_NUM 5
#define HREF_GPIO_NUM 27
#define PCLK_GPIO_NUM 25

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

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // put your setup code here, to run once:
  WiFi.begin(".......................", "................................");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    log_i(".");
  }

  log_i("\r\nConnected to wifi");
  initTime();

  //Test
  // IoTBlobHelper h;
  // uint8_t b[] = {1, 2, 3};
  // h.UploadBlob("t2", b, 3);

  //Camera and LCD
  char buff[256];

  delay(1000);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    log_e("Camera init failed with error 0x%x", err);
    while (1)
      ;
  }

  //drop down frame size for higher initial frame rate
  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
}

///
int messageCount = 0;
IoTBlobHelper h;
void loop()
{
  char buf[30];
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb)
  {
    log_e("Camera capture failed");
  }
  else
  {
    uint32_t now = millis();
    snprintf(buf,sizeof(buf),"%d.jpg",now);

    h.UploadBlob(buf,(uint8_t *)fb->buf, fb->len,"image/jpeg");
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  //vTaskDelay(1000 / portTICK_PERIOD_MS);
  log_i("FreeMem: %d", ESP.getFreeHeap());
  //vTaskDelay(10000 / portTICK_PERIOD_MS);
}