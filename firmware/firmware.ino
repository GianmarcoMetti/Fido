#define CAMERA_MODEL_XIAO_ESP32S3
#include <driver/i2s.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "esp_camera.h"
#include "camera_pins.h"
#include "mulaw.h"

//
// BLE
//

static BLEUUID serviceUUID("19B10000-E8F2-537E-4F6C-D104768A1214");
static BLEUUID audioCharUUID("19B10001-E8F2-537E-4F6C-D104768A1214");
static BLEUUID audioCodecUUID("19B10002-E8F2-537E-4F6C-D104768A1214");
static BLEUUID photoCharUUID("19B10005-E8F2-537E-4F6C-D104768A1214");

BLECharacteristic *audio;
BLECharacteristic *photo;
bool connected = false;

class ServerHandler : public BLEServerCallbacks {
  void onConnect(BLEServer *server) {
    connected = true;
    Serial.println("Connected");
  }

  void onDisconnect(BLEServer *server) {
    connected = false;
    Serial.println("Disconnected");
    BLEDevice::startAdvertising();
  }
};

class MessageHandler : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param) {
    // Currently unused
  }
};

void configure_ble() {
  BLEDevice::init("FidoCamera");
  BLEServer *server = BLEDevice::createServer();
  BLEService *service = server->createService(serviceUUID);

  // Audio service
  audio = service->createCharacteristic(
    audioCharUUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  BLE2902 *ccc = new BLE2902();
  ccc->setNotifications(true);
  audio->addDescriptor(ccc);

  // Photo service
  photo = service->createCharacteristic(
    photoCharUUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  ccc = new BLE2902();
  ccc->setNotifications(true);
  photo->addDescriptor(ccc);

  // Codec service
  BLECharacteristic *codec = service->createCharacteristic(
    audioCodecUUID,
    BLECharacteristic::PROPERTY_READ
  );
  uint8_t codecId = 11; // MuLaw 8mhz
  codec->setValue(&codecId, 1);

  // Service
  server->setCallbacks(new ServerHandler());
  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(service->getUUID());
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x0);
  advertising->setMinPreferred(0x1F);
  BLEDevice::startAdvertising();
}

camera_fb_t *fb;

bool take_photo() {
  // Release buffer
  if (fb) {
    Serial.println("Release FB");
    esp_camera_fb_return(fb);
  }

  // Take a photo
  Serial.println("Taking photo...");
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to get camera frame buffer");
    return false;
  }

  Serial.printf("Photo taken! Size: %d bytes\n", fb->len);

  // Log the first few bytes of the image data for debugging
  for (int i = 0; i < 10 && i < fb->len; ++i) {
    Serial.printf("%02x ", fb->buf[i]);
  }
  Serial.println();

  return true;
}

static uint8_t *s_compressed_frame_2 = nullptr;
static size_t compressed_buffer_size = 400 + 3; /* header 

/*
// Microphone


#define VOLUME_GAIN 2

static size_t recording_buffer_size = 400;
static size_t compressed_buffer_size = 400 + 3; /* header 
static uint8_t *s_recording_buffer = nullptr;
static uint8_t *s_compressed_frame = nullptr;
static uint8_t *s_compressed_frame_2 = nullptr;

void configure_microphone() {
  // I2S configuration
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 8000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0
  };

  // I2S pin configuration
  i2s_pin_config_t pin_config = {
      .bck_io_num = 42,
      .ws_io_num = 41,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = -1
  };

  // Install and start I2S driver
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);

  // Allocate buffers
  s_recording_buffer = (uint8_t *) ps_calloc(recording_buffer_size, sizeof(uint8_t));
  s_compressed_frame = (uint8_t *) ps_calloc(compressed_buffer_size, sizeof(uint8_t));
  s_compressed_frame_2 = (uint8_t *) ps_calloc(compressed_buffer_size, sizeof(uint8_t));
}

size_t read_microphone() {
  size_t bytes_recorded;
  i2s_read(I2S_NUM_0, (void*) s_recording_buffer, recording_buffer_size, &bytes_recorded, portMAX_DELAY);
  return bytes_recorded;
}

*/


//
// Camera
//

void configure_camera() {
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
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.fb_count = 1;

  // High quality (psram)
  // config.jpeg_quality = 10;
  // config.fb_count = 2;
  // config.grab_mode = CAMERA_GRAB_LATEST;

  // Low quality (and in local ram)
  config.jpeg_quality = 10;
  config.frame_size = FRAMESIZE_SVGA;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  // config.fb_location = CAMERA_FB_IN_DRAM;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
//
// Main
//

void setup() {
  Serial.begin(921600);
  // SD.begin(21);
  Serial.println("Setup");
  Serial.println("Starting BLE...");
  configure_ble();
  s_compressed_frame_2 = (uint8_t *) ps_calloc(compressed_buffer_size, sizeof(uint8_t));
/*  Serial.println("Starting Microphone...");
  configure_microphone(); */
  Serial.println("Starting Camera...");
  configure_camera();
  Serial.println("OK");
}

uint16_t frame_count = 0;
unsigned long lastCaptureTime = 0;
size_t sent_photo_bytes = 0;
size_t sent_photo_frames = 0;
bool need_send_photo = false;

void loop() {
/*  // Read from mic
  size_t bytes_recorded = read_microphone(); 

  // Push to BLE
  if (bytes_recorded > 0 && connected) {
    size_t out_buffer_size = bytes_recorded / 2 + 3;
    for (size_t i = 0; i < bytes_recorded; i += 2) {
      int16_t sample = ((s_recording_buffer[i + 1] << 8) | s_recording_buffer[i]) << VOLUME_GAIN;
      s_compressed_frame[i / 2 + 3] = linear2ulaw(sample);
    }
    s_compressed_frame[0] = frame_count & 0xFF;
    s_compressed_frame[1] = (frame_count >> 8) & 0xFF;
    s_compressed_frame[2] = 0;
    audio->setValue(s_compressed_frame, out_buffer_size);
    audio->notify();
    frame_count++;
  } */

  // Take a photo
  unsigned long now = millis();
  if ((now - lastCaptureTime) >= 5000 && !need_send_photo && connected) {
    if (take_photo()) {
      need_send_photo = true;
      sent_photo_bytes = 0;
      sent_photo_frames = 0;
      lastCaptureTime = now;
    }
  }

  // Push to BLE
  if (need_send_photo) {
    size_t remaining = fb->len - sent_photo_bytes;
    if (remaining > 0) {
      // Populate buffer
      s_compressed_frame_2[0] = sent_photo_frames & 0xFF;
      s_compressed_frame_2[1] = (sent_photo_frames >> 8) & 0xFF;
      size_t bytes_to_copy = remaining;
      if (bytes_to_copy > 200) {
        bytes_to_copy = 200;
      }
      memcpy(&s_compressed_frame_2[2], &fb->buf[sent_photo_bytes], bytes_to_copy);

      // Push to BLE
      Serial.printf("Sending photo chunk %d, size: %d bytes\n", sent_photo_frames, bytes_to_copy);
      photo->setValue(s_compressed_frame_2, bytes_to_copy + 2);
      photo->notify();
      sent_photo_bytes += bytes_to_copy;
      sent_photo_frames++;
    } else {
      // End flag
      s_compressed_frame_2[0] = 0xFF;
      s_compressed_frame_2[1] = 0xFF;
      photo->setValue(s_compressed_frame_2, 2);
      photo->notify();

      Serial.println("Photo sent");
      need_send_photo = false;
    }
  }

  // Delay
  delay(4);
}
