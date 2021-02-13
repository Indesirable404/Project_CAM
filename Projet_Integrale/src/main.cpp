#include <Arduino.h>

// Change according to your model
// The models available are
//   - CAMERA_MODEL_WROVER_KIT
//   - CAMERA_MODEL_ESP_EYE
//   - CAMERA_MODEL_M5STACK_PSRAM
//   - CAMERA_MODEL_M5STACK_WIDE
//   - CAMERA_MODEL_AI_THINKER
#define CAMERA_MODEL_AI_THINKER

#include <FS.h>
#include <SPIFFS.h>
#include "Downscale.h"
#include "MotionDetection.h"
#include "Cross.h"
#include "Strategy.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "esp_http_server.h"

//Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// This project was tested with the AI Thinker Model, M5STACK PSRAM Model and M5STACK WITHOUT PSRAM
#define CAMERA_MODEL_AI_THINKER //Nous utilisons une CAMERA_MODEL_AI_THINKER.
  #define PWDN_GPIO_NUM     32 //Nous definissons les pins de la carte.
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

  #define Flashlight         4

httpd_handle_t stream_httpd = NULL;

// set the resolution of the source image and the resolution of the downscaled image for the motion detection
#define SOURCE_WIDTH 320
#define SOURCE_HEIGHT 240
#define CHANNELS 1
#define DEST_WIDTH 32
#define DEST_HEIGHT 24
#define BLOCK_VARIATION_THRESHOLD 0.3
#define MOTION_THRESHOLD 0.2

// we're using the Eloquent::Vision namespace a lot!
using namespace Eloquent::Vision;
using namespace Eloquent::Vision::ImageProcessing;
using namespace Eloquent::Vision::ImageProcessing::Downscale;
using namespace Eloquent::Vision::ImageProcessing::DownscaleStrategies;

/*
   This method only stream one JPEG image - Cette méthode ne publie qu'une seule image JPEG
   Compatible with/avec Jeedom / NextDom / Domoticz
*/
static esp_err_t capture_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t fb_len = 0;
  int64_t fr_start = esp_timer_get_time();

  res = httpd_resp_set_type(req, "image/jpeg");
  if (res == ESP_OK)
  {
    res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=image.jpg"); //capture
  }
  if (res == ESP_OK)
  {
    ESP_LOGI(TAG, "Take a picture");
    digitalWrite(Flashlight, HIGH);
    //while(1){
    fr_start = esp_timer_get_time();
    fb = esp_camera_fb_get();
    if (!fb)
    {
      ESP_LOGE(TAG, "Camera capture failed");
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    else
    {
      fb_len = fb->len;
      res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

      esp_camera_fb_return(fb);
      // Uncomment if you want to know the bit rate - décommentez pour connaître le débit
      //int64_t fr_end = esp_timer_get_time();
      //ESP_LOGD(TAG, "JPG: %uKB %ums", (uint32_t)(fb_len / 1024), (uint32_t)((fr_end - fr_start) / 1000));
      digitalWrite(Flashlight, LOW);
      return res;
    }
    //}
  }
}

camera_fb_t* capture_camera()
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t fb_len = 0;
  
    ESP_LOGI(TAG, "Take a picture");

    fb = esp_camera_fb_get();
    if (!fb)
    {
      ESP_LOGE(TAG, "Camera capture failed");
    }
    else
    {
      return fb;
      ESP_LOGE(TAG, "Camera capture successed");
    }
}



// the buffer to store the downscaled version of the image
uint8_t resized[DEST_HEIGHT][DEST_WIDTH];
// the downscaler algorithm
// for more details see https://eloquentarduino.github.io/2020/05/easier-faster-pure-video-esp32-cam-motion-detection
Cross<SOURCE_WIDTH, SOURCE_HEIGHT, DEST_WIDTH, DEST_HEIGHT> crossStrategy;
// the downscaler container
Downscaler<SOURCE_WIDTH, SOURCE_HEIGHT, CHANNELS, DEST_WIDTH, DEST_HEIGHT> downscaler(&crossStrategy);
// the motion detection algorithm
MotionDetection<DEST_WIDTH, DEST_HEIGHT> motion;

void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);
    motion.setBlockVariationThreshold(BLOCK_VARIATION_THRESHOLD);
}

void loop() {
    // capture image
    camera_fb_t *fb = capture_camera();

    // resize image and detect motion
    downscaler.downscale(fb->buf, resized);
    motion.update(resized);
    motion.detect();

    if (motion.ratio() > MOTION_THRESHOLD) {

        Serial.println("Motion detected");
        
        // here we want to save the image to disk
    }
}