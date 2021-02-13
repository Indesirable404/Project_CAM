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
#include "EloquentVision.h"

// set the resolution of the source image and the resolution of the downscaled image for the motion detection
#define FRAME_SIZE FRAMESIZE_QVGA
#define SOURCE_WIDTH 320
#define SOURCE_HEIGHT 240
#define CHANNELS 1
#define DEST_WIDTH 32
#define DEST_HEIGHT 24
#define BLOCK_VARIATION_THRESHOLD 0.3
#define MOTION_THRESHOLD 0.2

// we're using the Eloquent::Vision namespace a lot!
using namespace Eloquent::Vision;
using namespace Eloquent::Vision::IO;
using namespace Eloquent::Vision::ImageProcessing;
using namespace Eloquent::Vision::ImageProcessing::Downscale;
using namespace Eloquent::Vision::ImageProcessing::DownscaleStrategies;

// an easy interface to capture images from the camera
ESP32Camera camera;
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
    camera.begin(FRAME_SIZE, PIXFORMAT_GRAYSCALE);
    motion.setBlockVariationThreshold(BLOCK_VARIATION_THRESHOLD);
}

void loop() {
    camera_fb_t *frame = camera.capture();

    // resize image and detect motion
    downscaler.downscale(frame->buf, resized);
    motion.update(resized);
    motion.detect();

    if (motion.ratio() > MOTION_THRESHOLD) {
        Serial.println("Motion detected");

        // here we want to save the image to disk
    }
}