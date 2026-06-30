/**
 * Pixel Studio Signboard Firmware
 * Generated automatically by Pixel Studio Web App.
 * Hardware: ESP32-S3 N16R8, 80x16 WS2812B Matrix, Rotary Encoder
 */

#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#define MATRIX_PIN 3
#define MATRIX_WIDTH 80
#define MATRIX_HEIGHT 16
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)

// Rotary Encoder Pins
#define ENCODER_A 12
#define ENCODER_B 13
#define ENCODER_C 14 // Push button

// WiFi Settings
const char *WIFI_SSID = "Your WiFi Name";
const char *WIFI_PASS = "WiFi Password";

// Static IP Settings
#define STATIC_IP_ENABLED true
const char *STATIC_IP = "192.168.1.100";
const char *GATEWAY_IP = "192.168.1.1";
const char *SUBNET_MASK = "255.255.255.0";
const char *DNS_IP = "8.8.8.8";

// NTP & Timezone
const char *NTP_SERVER = "pool.ntp.org";
const char *TZ_INFO = "IST-5:30"; // Timezone rules (e.g. IST-5:30)

// OpenWeatherMap Settings
const char *OWM_KEY = "Your OpenWeatherMap Key";
const char *OWM_CITY = "Your City";
const char *OWM_COUNTRY = "Your Country Code";
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherInterval = 900000; // 15 mins

// Weather Cache
float weatherTemp = 0.0;
int weatherHumidity = 0;
String weatherDesc = "Clear";

// YouTube Settings
const char *YT_API_KEY = "Your YouTube API Key";
const char *YT_CHANNEL_ID = "Your YouTube Channel ID";
unsigned long lastYtUpdate = 0;
const unsigned long ytInterval = 3600000; // 1 hour
unsigned long ytRetryInterval = 300000;   // 5 mins on failure
bool ytLastUpdateSuccess = true;

// YouTube Cache
long ytSubscriberCount = 1980;

// FastLED/NeoPixel Frame Buffer
Adafruit_NeoPixel matrix =
    Adafruit_NeoPixel(NUM_LEDS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);
GFXcanvas1 textCanvas(MATRIX_WIDTH, MATRIX_HEIGHT);

// Live stream socket server & state variables
WiFiServer wsServer(81);
WiFiClient wsClient;
unsigned long lastLiveFrameTime = 0;
bool liveModeActive = false;

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER,
                     19800); // 5h 30m default offset in seconds

// Rotary Encoder State
volatile int encoderPos = 0;
int lastReportedPos = 0;
unsigned long lastDebounceTime = 0;
const int debounceDelay = 20;

// Button state
bool buttonPressed = false;
unsigned long buttonPressTime = 0;
int clickCount = 0;
const int doubleClickTime = 400;
unsigned long lastPhysicalInteractionTime = 0;

// Application Navigation States
enum AppState {
  STATE_SCENE_PLAY,
  STATE_TIMER_ADJUST,
  STATE_MENU_NAV,
  STATE_BRIGHTNESS_ADJUST
};
AppState currentState = STATE_SCENE_PLAY;
int menuCursor = 0;
int currentBrightness = (10 * 100) / 255;

// Preset Scenes count
const int NUM_SCENES = 26;
int currentSceneIndex = 0;

// Timer widget state
long countdownSeconds = 300; // 5 mins default
unsigned long lastTimerTick = 0;
bool timerRunning = false;

// Scrolling text variables
int scrollOffset_3_text_1782736739871 = 0;
unsigned long lastScrollTime_3_text_1782736739871 = 0;
int scrollOffset_4_text_1782791848509_616 = 0;
unsigned long lastScrollTime_4_text_1782791848509_616 = 0;
int scrollOffset_5_text_1781951751553 = 0;
unsigned long lastScrollTime_5_text_1781951751553 = 0;
int scrollOffset_6_text_1780903101051 = 0;
unsigned long lastScrollTime_6_text_1780903101051 = 0;
int scrollOffset_7_text_1780914747000 = 0;
unsigned long lastScrollTime_7_text_1780914747000 = 0;
int scrollOffset_8_text_1782374123305 = 0;
unsigned long lastScrollTime_8_text_1782374123305 = 0;
int scrollOffset_9_text_1780916508763 = 0;
unsigned long lastScrollTime_9_text_1780916508763 = 0;
int scrollOffset_11_text_1780915846538 = 0;
unsigned long lastScrollTime_11_text_1780915846538 = 0;
int scrollOffset_25_text_1782801231231 = 0;
unsigned long lastScrollTime_25_text_1782801231231 = 0;
int scrollOffset_25_text_1782801265079_577 = 0;
unsigned long lastScrollTime_25_text_1782801265079_577 = 0;

// Stickers data in Flash memory
// Sticker: Love Heart (8x8)
const uint32_t PROGMEM sticker_love_heart_heart_8[] = {
    0x00000000, 0xffef4444, 0xffef4444, 0x00000000, 0x00000000, 0xffef4444,
    0xffef4444, 0x00000000, 0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444,
    0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444,
    0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444,
    0x00000000, 0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444, 0xffef4444,
    0xffef4444, 0x00000000, 0x00000000, 0x00000000, 0xffef4444, 0xffef4444,
    0xffef4444, 0xffef4444, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffef4444, 0xffef4444, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000};

// Sticker: Retro Ghost (8x8)
const uint32_t PROGMEM sticker_retro_ghost_ghost_8[] = {
    0x00000000, 0x00000000, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7,
    0x00000000, 0x00000000, 0x00000000, 0xffa855f7, 0xffa855f7, 0xffa855f7,
    0xffa855f7, 0xffa855f7, 0xffa855f7, 0x00000000, 0xffa855f7, 0xffa855f7,
    0xffffffff, 0xffa855f7, 0xffa855f7, 0xffffffff, 0xffa855f7, 0xffa855f7,
    0xffa855f7, 0xffa855f7, 0xffffffff, 0xffa855f7, 0xffa855f7, 0xffffffff,
    0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7,
    0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7,
    0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7,
    0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7, 0xffa855f7,
    0xffa855f7, 0xffa855f7, 0xffa855f7, 0x00000000, 0xffa855f7, 0x00000000,
    0x00000000, 0xffa855f7, 0x00000000, 0xffa855f7};

// Sticker: Sunny Weather (8x8)
const uint32_t PROGMEM sticker_sunny_weather_sun_8[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff59e0b, 0x00000000,
    0xfff59e0b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff59e0b,
    0xfff59e0b, 0xfff59e0b, 0xfff59e0b, 0xfff59e0b, 0x00000000, 0x00000000,
    0xfff59e0b, 0xfff59e0b, 0xfff59e0b, 0xfff59e0b, 0xfff59e0b, 0xfff59e0b,
    0xfff59e0b, 0x00000000, 0x00000000, 0xfff59e0b, 0xfff59e0b, 0xfff59e0b,
    0xfff59e0b, 0xfff59e0b, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfff59e0b, 0x00000000, 0xfff59e0b, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000};

// Sticker: Cloudy weather (8x8)
const uint32_t PROGMEM sticker_cloudy_weather_cloud_8[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff38bdf8, 0xff38bdf8, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8,
    0xff38bdf8, 0x00000000, 0x00000000, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8,
    0xff38bdf8, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8,
    0xff38bdf8, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8, 0xff38bdf8,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000};

// Sticker: YouTube Count Sticker 1 (16x16)
const uint32_t PROGMEM sticker_youtube_count_sticker_1_sticker_1780894095375[] =
    {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffffffff, 0xffffffff, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffffffff, 0xffffffff,
     0xffffffff, 0xffffffff, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
     0xffffffff, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffffffff, 0xffffffff,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffffffff, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffffffff, 0x00000000, 0xffffffff,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
     0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
     0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
     0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000, 0x00000000, 0x00000000};

// Sticker: Mario Sticker 1 (16x16)
const uint32_t PROGMEM sticker_mario_sticker_1_shared_1781951601209[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff754a00, 0xff754a00, 0xff754a00, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a,
    0xff090d16, 0xfff5ce8a, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff754a00, 0xfff5ce8a, 0xff754a00,
    0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0xff090d16, 0xfff5ce8a,
    0xfff5ce8a, 0xfff5ce8a, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xff754a00, 0xfff5ce8a, 0xff754a00, 0xfff5ce8a, 0xfff5ce8a,
    0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0xff090d16, 0xfff5ce8a, 0xfff5ce8a,
    0xfff5ce8a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff754a00,
    0xff754a00, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a,
    0xff090d16, 0xff090d16, 0xff090d16, 0xff090d16, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff5ce8a,
    0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a,
    0xfff5ce8a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xff0004ff, 0xffff0000,
    0xffff0000, 0xffff0000, 0xff0004ff, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xff0004ff, 0xffff0000, 0xffff0000, 0xff0004ff,
    0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xff0004ff, 0xff0004ff, 0xff0004ff, 0xff0004ff, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfff5ce8a, 0xfff5ce8a, 0xffff0000, 0xff0004ff, 0xffffa200, 0xff0004ff,
    0xff0004ff, 0xffffa200, 0xff0004ff, 0xffff0000, 0xfff5ce8a, 0xfff5ce8a,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff5ce8a, 0xfff5ce8a,
    0xfff5ce8a, 0xff0004ff, 0xff0004ff, 0xff0004ff, 0xff0004ff, 0xff0004ff,
    0xff0004ff, 0xfff5ce8a, 0xfff5ce8a, 0xfff5ce8a, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xfff5ce8a, 0xfff5ce8a, 0xff0004ff, 0xff0004ff,
    0xff0004ff, 0xff0004ff, 0xff0004ff, 0xff0004ff, 0xff0004ff, 0xff0004ff,
    0xfff5ce8a, 0xfff5ce8a, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff0004ff, 0xff0004ff, 0xff0004ff, 0x00000000,
    0x00000000, 0xff0004ff, 0xff0004ff, 0xff0004ff, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff754a00,
    0xff754a00, 0xff754a00, 0xff754a00, 0x00000000, 0x00000000, 0xff754a00,
    0xff754a00, 0xff754a00, 0xff754a00, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff754a00, 0xff754a00, 0xff754a00, 0xff754a00,
    0xff754a00, 0x00000000, 0x00000000, 0xff754a00, 0xff754a00, 0xff754a00,
    0xff754a00, 0xff754a00, 0x00000000, 0x00000000};

// Sticker: Mario Sticker 2 (16x16)
const uint32_t PROGMEM sticker_mario_sticker_2_sticker_1781953405229[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfff59e0b, 0x00000000, 0xfff59e0b, 0x00000000, 0xfff59e0b, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff59e0b,
    0xfff59e0b, 0xfff59e0b, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xfffbff14, 0xfffbff14, 0xfffbff14, 0xfffbff14,
    0xfffbff14, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffbff14,
    0xfffbff14, 0xfffbff14, 0xfffbff14, 0xfffbff14, 0xfffbff14, 0xfffbff14,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xfffbff14, 0xfffbff14, 0xffff8aef, 0xfffbff14,
    0xfffbff14, 0xfffbff14, 0xfffbff1d, 0xfffbff14, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfffbff14, 0xfffbff14, 0xffff8aef, 0xff0062ff, 0xffff8aef, 0xfffbff14,
    0xffff93f0, 0xfffbff14, 0xfffbff14, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff8aef,
    0xffff8aef, 0xff0ea5e9, 0xffff8aef, 0xfffbff14, 0xff22c55e, 0xfffbff14,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffff8aef, 0xffff8aef, 0xffff8aef,
    0xffff8aef, 0xffff8aef, 0xfffbff14, 0xfffbff14, 0xfffbff14, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff8ef0, 0xffff8ef0, 0xffff8ef0, 0xffff8ef0,
    0xfffbff14, 0xfffbff14, 0xfffbff14, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xfffbff14, 0xfffbff14,
    0xfffbff14, 0xfffbff14, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xfffbff14, 0xfffbff14, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffff97f1, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff97f1,
    0xffff97f1, 0xfffbff14, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
    0xffff002b, 0xffff002b, 0xffff002b, 0xffff0000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff002b, 0xffff002b, 0xffff002b,
    0xffff002b, 0xffff0000, 0xffff0000, 0xffff002b, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
    0xffff002b, 0xffff002b, 0xffff002b, 0xffff002b, 0xffff002b, 0xffff002b,
    0xffff002b, 0xffff0000, 0xffff002b, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff002b, 0xffff002b,
    0xffff002b, 0xffff002b, 0xffff002b, 0xffff002b, 0xffff002b, 0xffff0000,
    0xffff0000, 0xffff002b, 0x00000000, 0x00000000};

// Sticker: On Call Sticker 1 (16x16)
const uint32_t PROGMEM sticker_on_call_sticker_1_sticker_1780917468946[] = {
    0x00000000, 0x00000000, 0x00000000, 0xff048604, 0xff048604, 0xff048604,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff048604, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff048604, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff048604, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff048604, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff048604, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff048604, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff048604, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff048604,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff048604, 0xff00ff00, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff048604, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff048604, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff048604,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff048604, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff048604, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff048604, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff048604, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xff048604, 0xff048604, 0xff048604, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff048604, 0xff00ff00, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff048604, 0x00000000, 0x00000000, 0xff048604, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff048604, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xff048604, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
    0xff048604, 0xff048604, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff048604, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff048604, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff048604,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff048604,
    0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff048604, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff048604, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
    0xff048604, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff048604, 0xff00ff00, 0xff00ff00,
    0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff048604, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff048604, 0xff048604, 0xff048604, 0xff048604,
    0xff048604, 0x00000000, 0x00000000, 0x00000000};

// Sticker: NectPCB Sticker 1 (16x16)
const uint32_t PROGMEM sticker_nectpcb_sticker_1_sticker_1780914666070[] = {
    0x00000000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000,
    0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000,
    0xffff0000, 0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
    0xffff0000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0x00000000, 0x00000000, 0xffff0000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000};

// Sticker: DFRobot Sticker 1 (15x16)
const uint32_t PROGMEM sticker_dfrobot_sticker_1_sticker_1780915744096[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffff5500, 0xffff5500, 0xffff5500, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffff5500, 0xffff5500, 0xffff5500,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffff5500, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffff5500, 0xffff5500, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff5500, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff5500, 0xffff5500,
    0xffff5500, 0xffff5500, 0x00000000, 0x00000000, 0x00000000, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0x00000000, 0x00000000,
    0x00000000, 0xffff5500, 0xffff5500, 0x00000000, 0x00000000, 0xffff5500,
    0x00000000, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffff5500, 0xffff5500, 0x00000000, 0xffff5500, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffff5500, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffff5500, 0xffff5500, 0xffff5500, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffff5500, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffffffff, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500, 0xffff5500,
    0xffff5500, 0xffff5500, 0xffffffff, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffff5500, 0xffff5500, 0xffff5500, 0xffffffff, 0xffffffff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};

// Sticker: Sword (16x16)
const uint32_t PROGMEM sticker_sword___sxmexct7_agjpefmar[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0ea5e9,
    0xff0ea5e9, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff0ea5e9, 0xff0ea5e9, 0xff0ea5e9, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0ea5e9,
    0xff0ea5e9, 0xff0ea5e9, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff0ea5e9, 0xff0ea5e9, 0xff0ea5e9, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0ea5e9,
    0xff0ea5e9, 0xff0ea5e9, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff0ea5e9, 0xff0ea5e9, 0xff0ea5e9, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff59e0b,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0ea5e9,
    0xff0ea5e9, 0xff0ea5e9, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xfff59e0b, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xff0ea5e9, 0xff0ea5e9, 0xff0ea5e9, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xfff59e0b, 0x00000000, 0x00000000, 0xff0ea5e9,
    0xff0ea5e9, 0xff0ea5e9, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xfff59e0b, 0xff06b6d4, 0xff0ea5e9, 0xff0ea5e9, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff59e0b,
    0xffff0000, 0xff06b6d4, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xfff59e0b, 0xfff59e0b, 0xfff59e0b, 0xfff59e0b,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff59e0b,
    0xfff59e0b, 0xfff59e0b, 0x00000000, 0x00000000, 0xfff59e0b, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xfff59e0b, 0xfff59e0b, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xfff59e0b, 0xfff59e0b, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000};

// Custom backgrounds in Flash memory
// Custom Background: BACKGROUND Layer (80x16)
const uint32_t PROGMEM bg_background_layer_bg_1782792030400[] = {
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
    0x00000000, 0x00000000};

// Custom Background: Solid Dark BG (80x16)
const uint32_t PROGMEM bg_solid_dark_bg_bg_1780842336867[] = {
    0x00000000, 0xffffff00, 0xffffff00, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffff00,
    0xffffff00, 0x00000000, 0xffffff00, 0xffffff00, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffff00, 0xffffff00, 0xffffff00, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffff00,
    0x00000000, 0x00000000, 0xffffff00, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffff00,
    0x00000000, 0x00000000, 0x00000000, 0xffffff00, 0xffffff00, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffff00, 0xffffff00, 0x00000000, 0xffffff00, 0xffffff00,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffff00, 0xffffff00,
    0xffffff00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffff00, 0x00000000, 0x00000000, 0xffffff00, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffff00, 0x00000000, 0x00000000, 0x00000000, 0xffffff00,
    0xffffff00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffffff00, 0xffffff00, 0x00000000,
    0xffffff00, 0xffffff00, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffff00, 0xffffff00, 0xffffff00, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffffff00, 0x00000000, 0x00000000,
    0xffffff00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffffff00, 0x00000000, 0x00000000,
    0x00000000, 0xffffff00, 0xffffff00, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffff00,
    0xffffff00, 0x00000000, 0xffffff00, 0xffffff00, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffff00, 0xffffff00, 0xffffff00, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffff00,
    0x00000000, 0x00000000, 0xffffff00, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffff00,
    0x00000000, 0x00000000};

// Custom animations in Flash memory

// Counter widget states
long counter_counter_1782374184679 = 123;

// Tiny 3x5 font table for text rendering at Size 0
const uint8_t PROGMEM tiny_font[96][3] = {
    {0x00, 0x00, 0x00}, // 32: Space
    {0x00, 0x1d, 0x00}, // 33: !
    {0x03, 0x00, 0x03}, // 34: "
    {0x0a, 0x1f, 0x0a}, // 35: #
    {0x12, 0x1f, 0x09}, // 36: $
    {0x13, 0x08, 0x19}, // 37: %
    {0x0a, 0x15, 0x0a}, // 38: &
    {0x00, 0x03, 0x00}, // 39: '
    {0x00, 0x0e, 0x11}, // 40: (
    {0x11, 0x0e, 0x00}, // 41: )
    {0x0a, 0x04, 0x0a}, // 42: *
    {0x04, 0x0e, 0x04}, // 43: +
    {0x00, 0x10, 0x08}, // 44: ,
    {0x04, 0x04, 0x04}, // 45: -
    {0x00, 0x10, 0x00}, // 46: .
    {0x10, 0x08, 0x04}, // 47: /
    {0x1f, 0x11, 0x1f}, // 48: 0
    {0x00, 0x1f, 0x00}, // 49: 1
    {0x1d, 0x15, 0x17}, // 50: 2
    {0x15, 0x15, 0x1f}, // 51: 3
    {0x07, 0x04, 0x1f}, // 52: 4
    {0x17, 0x15, 0x1d}, // 53: 5
    {0x1f, 0x15, 0x1d}, // 54: 6
    {0x01, 0x01, 0x1f}, // 55: 7
    {0x1f, 0x15, 0x1f}, // 56: 8
    {0x17, 0x15, 0x1f}, // 57: 9
    {0x00, 0x0a, 0x00}, // 58: :
    {0x00, 0x1a, 0x08}, // 59: ;
    {0x04, 0x0a, 0x11}, // 60: <
    {0x0a, 0x0a, 0x0a}, // 61: =
    {0x11, 0x0a, 0x04}, // 62: >
    {0x01, 0x15, 0x02}, // 63: ?
    {0x0e, 0x15, 0x0d}, // 64: @
    {0x1e, 0x05, 0x1e}, // 65: A
    {0x1f, 0x15, 0x0a}, // 66: B
    {0x0e, 0x11, 0x11}, // 67: C
    {0x1f, 0x11, 0x0e}, // 68: D
    {0x1f, 0x15, 0x11}, // 69: E
    {0x1f, 0x05, 0x01}, // 70: F
    {0x0e, 0x15, 0x1d}, // 71: G
    {0x1f, 0x04, 0x1f}, // 72: H
    {0x11, 0x1f, 0x11}, // 73: I
    {0x08, 0x10, 0x0f}, // 74: J
    {0x1f, 0x04, 0x1b}, // 75: K
    {0x1f, 0x10, 0x10}, // 76: L
    {0x1f, 0x02, 0x1f}, // 77: M
    {0x1f, 0x06, 0x1f}, // 78: N
    {0x0e, 0x11, 0x0e}, // 79: O
    {0x1f, 0x05, 0x02}, // 80: P
    {0x0e, 0x11, 0x1e}, // 81: Q
    {0x1f, 0x05, 0x1a}, // 82: R
    {0x12, 0x15, 0x09}, // 83: S
    {0x01, 0x1f, 0x01}, // 84: T
    {0x0f, 0x10, 0x0f}, // 85: U
    {0x07, 0x18, 0x07}, // 86: V
    {0x1f, 0x08, 0x1f}, // 87: W
    {0x1b, 0x04, 0x1b}, // 88: X
    {0x03, 0x1c, 0x03}, // 89: Y
    {0x19, 0x15, 0x13}, // 90: Z
    {0x00, 0x0e, 0x11}, // 91: [
    {0x10, 0x08, 0x04}, // 92:   {0x11, 0x0e, 0x00}, // 93: ]
    {0x02, 0x01, 0x02}, // 94: ^
    {0x10, 0x10, 0x10}, // 95: _
    {0x00, 0x03, 0x00}, // 96: `
    {0x1e, 0x05, 0x1e}, // 97: a
    {0x1f, 0x15, 0x0a}, // 98: b
    {0x0e, 0x11, 0x11}, // 99: c
    {0x1f, 0x11, 0x0e}, // 100: d
    {0x1f, 0x15, 0x11}, // 101: e
    {0x1f, 0x05, 0x01}, // 102: f
    {0x0e, 0x15, 0x1d}, // 103: g
    {0x1f, 0x04, 0x1f}, // 104: h
    {0x11, 0x1f, 0x11}, // 105: i
    {0x08, 0x10, 0x0f}, // 106: j
    {0x1f, 0x04, 0x1b}, // 107: k
    {0x1f, 0x10, 0x10}, // 108: l
    {0x1f, 0x02, 0x1f}, // 109: m
    {0x1f, 0x06, 0x1f}, // 110: n
    {0x0e, 0x11, 0x0e}, // 111: o
    {0x1f, 0x05, 0x02}, // 112: p
    {0x0e, 0x11, 0x1e}, // 113: q
    {0x1f, 0x05, 0x1a}, // 114: r
    {0x12, 0x15, 0x09}, // 115: s
    {0x01, 0x1f, 0x01}, // 116: t
    {0x0f, 0x10, 0x0f}, // 117: u
    {0x07, 0x18, 0x07}, // 118: v
    {0x1f, 0x08, 0x1f}, // 119: w
    {0x1b, 0x04, 0x1b}, // 120: x
    {0x03, 0x1c, 0x03}, // 121: y
    {0x19, 0x15, 0x13}, // 122: z
    {0x04, 0x0a, 0x04}, // 123: {
    {0x00, 0x1f, 0x00}, // 124: |
    {0x04, 0x0a, 0x04}, // 125: }
    {0x08, 0x04, 0x08}, // 126: ~
    {0x03, 0x03, 0x00}  // 127: degree
};

// Function Declarations
void handleEncoderInterrupt();
void checkEncoder();
void checkButton();
void updateWeather();
void updateYouTubeSubscribers();
void drawYouTubeSub(int x, int y, int w, int h, int textX, int textY,
                    uint32_t color, int size, bool shadow, uint32_t shadowColor,
                    String effect, int speed, bool bold = false,
                    String format = "short", bool rainbow = false);
void handleSerialCommands();
void handleWiFiFrames();
String getWebSocketAcceptKey(String clientKey);
bool readWebSocketFrame(WiFiClient &client, uint8_t *outBuffer,
                        int expectedLen);
String getSceneName(int idx);
void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, int opacity = 100);
void setPixel(int x, int y, uint32_t color, int opacity = 100);
void drawSolidBackground(uint32_t color, int opacity = 100);
void drawGradientBackground(uint32_t color1, uint32_t color2,
                            int opacity = 100);
void drawText(String txt, int x, int y, int w, int h, uint32_t color, int size,
              bool shadow, uint32_t shadowColor, String effect, int speed,
              bool bold = false, bool rainbow = false);
void drawText(String txt, int x, int y, uint32_t color, int size, bool shadow,
              uint32_t shadowColor, int scrollOffset, bool bold = false,
              bool rainbow = false);

void drawSticker(const uint32_t *sticker, int x, int y, int w, int h,
                 String motion, int speed);
void drawLocalTime(int x, int y, int w, int h, uint32_t color, int size,
                   String format, bool shadow = false, uint32_t shadowColor = 0,
                   String effect = "none", int speed = 4, bool bold = false,
                   bool rainbow = false);
void drawLocalDate(int x, int y, int w, int h, uint32_t color, int size,
                   String format, bool shadow = false, uint32_t shadowColor = 0,
                   String effect = "none", int speed = 4, bool bold = false,
                   bool rainbow = false);
void drawWeatherSticker(int sx, int sy, String condition);
void drawWeather(
    int x, int y, int w, int h, String param, int iconX = 2, int iconY = 0,
    int tempX = 22, int tempY = 0, int humiX = 58, int humiY = 0,
    int briefX = 22, int briefY = 8, String tempColorMode = "followBrief",
    uint32_t tempColor = 0xfffacc15, bool tempBold = false, int tempSize = 1,
    bool tempShadow = false, uint32_t tempShadowColor = 0,
    String humiColorMode = "followBrief", uint32_t humiColor = 0xff94a3b8,
    bool humiBold = false, int humiSize = 1, bool humiShadow = false,
    uint32_t humiShadowColor = 0, String briefColorMode = "followBrief",
    uint32_t briefColor = 0xffffffff, bool briefBold = true, int briefSize = 1,
    bool briefShadow = false, uint32_t briefShadowColor = 0);
void drawWeatherTemp(int x, int y, int w, int h, uint32_t color, int size,
                     bool shadow = false, uint32_t shadowColor = 0,
                     String effect = "none", int speed = 4, bool bold = false,
                     bool rainbow = false);
void drawWeatherHumi(int x, int y, int w, int h, uint32_t color, int size,
                     bool shadow = false, uint32_t shadowColor = 0,
                     String effect = "none", int speed = 4, bool bold = false,
                     bool rainbow = false);
void drawWeatherBrief(int x, int y, int w, int h, uint32_t color, int size,
                      bool shadow = false, uint32_t shadowColor = 0,
                      String effect = "none", int speed = 4, bool bold = false,
                      bool rainbow = false);
void drawClock(int x, int y, int w, int h, uint32_t color, int size,
               String timeOfDayOverride = "auto", bool shadow = true,
               uint32_t shadowColor = 0, bool bold = true, int bgX = 10,
               int bgY = 8, int dateX = 24, int dateY = 0, int timeX = 24,
               int timeY = 8, String dateFormat = "DD MMM",
               String timeFormat = "HH:MM AM/PM", uint32_t dateColor = 0xFFFFFF,
               uint32_t timeColor = 0xFFFFFF, bool dateBold = false,
               bool timeBold = true, int dateSize = 1, int timeSize = 1,
               bool dateShadow = true, bool timeShadow = true,
               uint32_t dateShadowColor = 0, uint32_t timeShadowColor = 0,
               String dateColorMode = "custom",
               String timeColorMode = "custom");
void drawTimer(int x, int y, int w, int h, uint32_t color, int size,
               bool shadow = false, uint32_t shadowColor = 0,
               String effect = "none", int speed = 4, bool bold = false,
               bool rainbow = false);
void drawCounter(long count, int x, int y, int w, int h, uint32_t color,
                 int size, bool shadow = false, uint32_t shadowColor = 0,
                 String effect = "none", int speed = 4, bool bold = false,
                 bool rainbow = false);
void drawPrebuiltAnimation(String animId, int x, int y, int opacity = 100,
                           int bgX = 10, int bgY = 8);
void drawBackgroundPixels(const uint32_t *bg, int x, int y, int w, int h,
                          int opacity = 100);
void drawCustomAnimation(const uint32_t *const *frames, int framesCount, int x,
                         int y, int w, int h, int frameRate, int opacity = 100);
void drawShape(String shapeType, int x, int y, int w, int h, uint32_t color,
               bool filled, uint32_t borderColor, int borderWidth,
               String motion = "none", int speed = 4, int cornerRadius = 0);

String getSceneName(int idx) {
  if (idx == NUM_SCENES)
    return "BRIGHTNESS";
  switch (idx) {
  case 0:
    return "CLOCK";
  case 1:
    return "WEATHER";
  case 2:
    return "YOUTUBE COUNT";
  case 3:
    return "BUSY";
  case 4:
    return "BOOT";
  case 5:
    return "MARIO";
  case 6:
    return "DFROBOT";
  case 7:
    return "NECTPCB";
  case 8:
    return "COUNTER";
  case 9:
    return "ON CALL";
  case 10:
    return "FOCUS TIMER";
  case 11:
    return "STAY OUT";
  case 12:
    return "A1";
  case 13:
    return "A2";
  case 14:
    return "A3";
  case 15:
    return "A4";
  case 16:
    return "A5";
  case 17:
    return "A6";
  case 18:
    return "A7";
  case 19:
    return "A8";
  case 20:
    return "A9";
  case 21:
    return "A10";
  case 22:
    return "A11";
  case 23:
    return "A12";
  case 24:
    return "A13";
  case 25:
    return "SCENE LAYOUT 26";
  default:
    return "UNKNOWN";
  }
}

bool currentSceneHasTimer() {
  switch (currentSceneIndex) {
  case 0:
    return false;
  case 1:
    return false;
  case 2:
    return false;
  case 3:
    return false;
  case 4:
    return false;
  case 5:
    return false;
  case 6:
    return false;
  case 7:
    return false;
  case 8:
    return false;
  case 9:
    return true;
  case 10:
    return true;
  case 11:
    return false;
  case 12:
    return false;
  case 13:
    return false;
  case 14:
    return false;
  case 15:
    return false;
  case 16:
    return false;
  case 17:
    return false;
  case 18:
    return false;
  case 19:
    return false;
  case 20:
    return false;
  case 21:
    return false;
  case 22:
    return false;
  case 23:
    return false;
  case 24:
    return false;
  case 25:
    return false;
  default:
    return false;
  }
}

void renderScene_0() {
  // Widget: CLOCK Layer (clock)
  drawClock(0, 0, 80, 16, 0xffffffff, 1, "auto", true, 0, true, 10, 10, 24, 1,
            24, 9, "DD MMM", "HH:MM AM/PM", 0xffffffff, 0xffffffff, false,
            false, 1, 1, false, false, 0, 0, "custom", "custom");
}

void renderScene_1() {
  // Widget: WEATHER Layer (weather)
  drawWeather(0, 0, 80, 16, "clear", 2, 0, 22, 1, 58, 1, 22, 9, "followBrief",
              0xfffacc15, false, 1, false, 0, "custom", 0xff006aff, false, 1,
              false, 0, "followBrief", 0xffffffff, false, 1, false, 0);
}

void renderScene_2() {
  // Widget: STICKER Layer (sticker)
  drawSticker(sticker_youtube_count_sticker_1_sticker_1780894095375, 1, 0, 16,
              16, "none", 4);
  // Widget: YOUTUBE-SUB Layer (youtube-sub)
  drawYouTubeSub(18, 1, 62, 16, 0, 0, 0xfff70808, 2, true, 0xffd9d9d9, "none",
                 4, false, "short", false);
}

void renderScene_3() {
  // Widget: BACKGROUND Layer (background)
  drawBackgroundPixels(bg_background_layer_bg_1782792030400, 0, 0, 80, 16, 100);
  // Widget: TEXT Layer (text)
  drawText("LIKE, SHARE, SUBSCRIBE", 2, 1, 76, 16, 0xffffffff, 2, true, 0,
           "left", 4, false, false);
}

void renderScene_4() {
  // Widget: TEXT Layer (text)
  drawText("PIXEL BAR", 0, 1, 80, 16, 0xffffffff, 2, true, 0xffababab, "left",
           4, false, true);
}

void renderScene_5() {
  // Widget: TEXT Layer (text)
  drawText("MARIO ", 0, 0, 80, 15, 0xff2bff00, 2, true, 0xffe7e4e4, "left", 4,
           false, false);
  // Widget: STICKER Layer (sticker)
  drawSticker(sticker_mario_sticker_1_shared_1781951601209, 1, 0, 16, 16,
              "wave", 2);
  // Widget: SHAPE Layer (shape)
  drawShape("hline", 0, 14, 80, 2, 0xff703c00, true, 0x00000000, 1, "none", 4,
            0);
  // Widget: STICKER Layer Copy (sticker)
  drawSticker(sticker_mario_sticker_2_sticker_1781953405229, 66, 0, 16, 16,
              "bounce", 2);
  // Widget: SHAPE Layer Copy (shape)
  drawShape("hline", 1, 12, 16, 2, 0xff04ff00, true, 0x00000000, 1, "none", 4,
            0);
}

void renderScene_6() {
  // Widget: STICKER Layer (sticker)
  drawSticker(sticker_dfrobot_sticker_1_sticker_1780915744096, 1, 0, 15, 16,
              "none", 4);
  // Widget: TEXT Layer (text)
  drawText("DFROBOT", 17, 1, 64, 15, 0xffff5e00, 2, true, 0xffd9d3d3, "left", 4,
           false, false);
}

void renderScene_7() {
  // Widget: STICKER Layer (sticker)
  drawSticker(sticker_nectpcb_sticker_1_sticker_1780914666070, 0, 0, 16, 16,
              "none", 4);
  // Widget: TEXT Layer (text)
  drawText("NEXT PCB", 17, 1, 63, 15, 0xffd60100, 2, true, 0xffe6e6e6, "left",
           4, false, false);
}

void renderScene_8() {
  // Widget: TEXT Layer (text)
  drawText("No.", 1, 1, 30, 16, 0xff0565ff, 2, true, 0xffffffff, "none", 4,
           false, false);
  // Widget: COUNTER Layer (counter)
  drawCounter(counter_counter_1782374184679, 34, 1, 47, 16, 0xff3b82f6, 2, true,
              0xffe8e8e8, "none", 4, true, false);
}

void renderScene_9() {
  // Widget: STICKER Layer (sticker)
  drawSticker(sticker_on_call_sticker_1_sticker_1780917468946, 2, 0, 16, 16,
              "none", 4);
  // Widget: TEXT Layer (text)
  drawText("ON CALL!", 21, 0, 70, 8, 0xffff0000, 1, true, 0xffc9c9c9, "none", 4,
           true, false);
  // Widget: TIMER Layer (timer)
  drawTimer(21, 9, 37, 8, 0xff3bf109, 1, false, 0xffe1e0e0, "none", 4, true,
            false);
}

void renderScene_10() {
  // Widget: TIMER Layer (timer)
  drawTimer(7, 1, 80, 18, 0xff3bf109, 2, true, 0xffe1e0e0, "none", 4, true,
            false);
}

void renderScene_11() {
  // Widget: Solid Dark BG (background)
  drawBackgroundPixels(bg_solid_dark_bg_bg_1780842336867, 0, 0, 80, 16, 100);
  // Widget: TEXT Layer (text)
  drawText("STAY OUT!", 3, 1, 74, 15, 0xffffea00, 2, true, 0xffe0dcdc, "left",
           3, false, false);
}

void renderScene_12() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("aurora", 0, 0, 100);
}

void renderScene_13() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("wavefront", 0, 0, 100);
}

void renderScene_14() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("kaleidoscope", 0, 0, 100);
}

void renderScene_15() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("vortex", 0, 0, 100);
}

void renderScene_16() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("spiral", 0, 0, 100);
}

void renderScene_17() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("fireflies", 0, 0, 100);
}

void renderScene_18() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("bounceballs", 0, 0, 100);
}

void renderScene_19() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("dnahelix", 0, 0, 100);
}

void renderScene_20() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("tunnel", 0, 0, 100);
}

void renderScene_21() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("combustion", 0, 0, 100);
}

void renderScene_22() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("starburst", 0, 0, 100);
}

void renderScene_23() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("metaballs", 0, 0, 100);
}

void renderScene_24() {
  // Widget: ANIMATION Layer (animation)
  drawPrebuiltAnimation("snake", 0, 0, 100);
}

void renderScene_25() {
  // Widget: STICKER Layer (sticker)
  drawSticker(sticker_dfrobot_sticker_1_sticker_1780915744096, 0, 0, 15, 16,
              "bounce", 4);
  // Widget: STICKER Layer Copy (sticker)
  drawSticker(sticker_nectpcb_sticker_1_sticker_1780914666070, 64, -1, 16, 16,
              "bounce", 4);
  // Widget: TEXT Layer (text)
  drawText("DFROBOT", 16, 1, 47, 8, 0xffff5900, 1, false, 0, "left", 4, false,
           false);
  // Widget: TEXT Layer Copy (text)
  drawText("NEXT PCB", 16, 9, 47, 8, 0xffff0000, 1, false, 0, "right", 4, false,
           false);
}

void setup() {
  Serial.setRxBufferSize(4096); // Increase serial buffer size to 4KB
  Serial.begin(921600);
  Serial.println("Pixel Studio Signboard Starting...");

  // Setup Led Strip
  matrix.begin();
  matrix.setBrightness((currentBrightness * 255) / 100);
  matrix.show();

  // Setup Encoder Pins
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_C, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), handleEncoderInterrupt,
                  CHANGE);

// WiFi Connection (displays shifting rainbow color PIXEL BAR text until it
// connects)
#if defined(STATIC_IP_ENABLED) && STATIC_IP_ENABLED
  {
    IPAddress local_IP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns;
    if (local_IP.fromString(STATIC_IP) && gateway.fromString(GATEWAY_IP) &&
        subnet.fromString(SUBNET_MASK) && dns.fromString(DNS_IP)) {
      if (!WiFi.config(local_IP, gateway, subnet, dns)) {
        Serial.println("Static IP Configuration Failed!");
      } else {
        Serial.println("Static IP Configured Successfully.");
      }
    }
  }
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int connectCounter = 0;
  while (WiFi.status() != WL_CONNECTED &&
         connectCounter < 30) { // check up to 15 seconds
    // Loop 10 times at 20 FPS (50ms delay) to check status every 500ms
    for (int f = 0; f < 10; f++) {
      if (WiFi.status() == WL_CONNECTED)
        break;
      matrix.clear();

      // Draw shifting rainbow color "PIXEL BAR" text (centered: x = 13, y = 4)
      textCanvas.fillScreen(0);
      textCanvas.setTextSize(1);
      textCanvas.setTextWrap(false);
      textCanvas.setTextColor(1);
      textCanvas.setCursor(13, 4);
      textCanvas.print("PIXEL BAR");

      unsigned long shift = millis() * 50;
      for (int cy = 0; cy < 16; cy++) {
        for (int cx = 0; cx < 80; cx++) {
          if (textCanvas.getPixel(cx, cy)) {
            uint32_t color = matrix.ColorHSV(
                (shift + map(cx, 13, 66, 0, 65535)) & 65535, 255, 255);
            setPixel(cx, cy, color);
          }
        }
      }
      matrix.show();
      delay(50);
    }
    if (WiFi.status() == WL_CONNECTED)
      break;
    Serial.print(".");
    connectCounter++;
  }

  matrix.clear();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    timeClient.begin();
    timeClient.update();
    updateWeather();
    updateYouTubeSubscribers();

    // Start Live WebSocket Server
    wsServer.begin();
  } else {
    Serial.println("\nWiFi Offline. Running in standalone clock mode.");
    drawText("PIXEL BAR", 13, 1, 0x6366f1, 1, false, 0, 0);
    // Centering "OFFLINE MODE" (12 chars * 4 = 48 pixels, x = 16, y = 10)
    drawText("OFFLINE MODE", 16, 10, 0xef4444, 0, false, 0, 0);
    matrix.show();
    delay(2000);
  }

  lastTimerTick = millis();
}

void loop() {
  // Check if live mode has timed out (2 seconds of inactivity)
  if (liveModeActive && millis() - lastLiveFrameTime > 2000) {
    liveModeActive = false;
  }

  // 1. Process encoder and button states
  checkEncoder();
  checkButton();

  // 2. Read live data update signals from serial connection
  handleSerialCommands();

  // 3. Keep local time refreshed
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    handleWiFiFrames();

    // Update weather hourly
    if (millis() - lastWeatherUpdate > weatherInterval ||
        lastWeatherUpdate == 0) {
      updateWeather();
    }

    // Update YouTube subscriber count hourly or retry on failure
    unsigned long currentYtInterval =
        ytLastUpdateSuccess ? ytInterval : ytRetryInterval;
    if (millis() - lastYtUpdate > currentYtInterval || lastYtUpdate == 0) {
      updateYouTubeSubscribers();
    }
  }

  // 4. Handle Timer decrement logic
  if (timerRunning && millis() - lastTimerTick >= 1000) {
    if (countdownSeconds > 0) {
      countdownSeconds--;
    } else {
      timerRunning = false;
    }
    lastTimerTick = millis();
  }

  // 5. Update scrolling text counters
  if (currentSceneIndex == 0) {
  }
  if (currentSceneIndex == 1) {
  }
  if (currentSceneIndex == 2) {
  }
  if (currentSceneIndex == 3) {
    if (millis() - lastScrollTime_3_text_1782736739871 > 80) {
      scrollOffset_3_text_1782736739871--;
      if (scrollOffset_3_text_1782736739871 < -200)
        scrollOffset_3_text_1782736739871 = 80;
      lastScrollTime_3_text_1782736739871 = millis();
    }
  }
  if (currentSceneIndex == 4) {
    if (millis() - lastScrollTime_4_text_1782791848509_616 > 80) {
      scrollOffset_4_text_1782791848509_616--;
      if (scrollOffset_4_text_1782791848509_616 < -200)
        scrollOffset_4_text_1782791848509_616 = 80;
      lastScrollTime_4_text_1782791848509_616 = millis();
    }
  }
  if (currentSceneIndex == 5) {
    if (millis() - lastScrollTime_5_text_1781951751553 > 80) {
      scrollOffset_5_text_1781951751553--;
      if (scrollOffset_5_text_1781951751553 < -200)
        scrollOffset_5_text_1781951751553 = 80;
      lastScrollTime_5_text_1781951751553 = millis();
    }
  }
  if (currentSceneIndex == 6) {
    if (millis() - lastScrollTime_6_text_1780903101051 > 80) {
      scrollOffset_6_text_1780903101051--;
      if (scrollOffset_6_text_1780903101051 < -200)
        scrollOffset_6_text_1780903101051 = 80;
      lastScrollTime_6_text_1780903101051 = millis();
    }
  }
  if (currentSceneIndex == 7) {
    if (millis() - lastScrollTime_7_text_1780914747000 > 80) {
      scrollOffset_7_text_1780914747000--;
      if (scrollOffset_7_text_1780914747000 < -200)
        scrollOffset_7_text_1780914747000 = 80;
      lastScrollTime_7_text_1780914747000 = millis();
    }
  }
  if (currentSceneIndex == 8) {
    if (millis() - lastScrollTime_8_text_1782374123305 > 80) {
      scrollOffset_8_text_1782374123305 = 0;
      lastScrollTime_8_text_1782374123305 = millis();
    }
  }
  if (currentSceneIndex == 9) {
    if (millis() - lastScrollTime_9_text_1780916508763 > 80) {
      scrollOffset_9_text_1780916508763 = 0;
      lastScrollTime_9_text_1780916508763 = millis();
    }
  }
  if (currentSceneIndex == 10) {
  }
  if (currentSceneIndex == 11) {
    if (millis() - lastScrollTime_11_text_1780915846538 > 90) {
      scrollOffset_11_text_1780915846538--;
      if (scrollOffset_11_text_1780915846538 < -200)
        scrollOffset_11_text_1780915846538 = 80;
      lastScrollTime_11_text_1780915846538 = millis();
    }
  }
  if (currentSceneIndex == 12) {
  }
  if (currentSceneIndex == 13) {
  }
  if (currentSceneIndex == 14) {
  }
  if (currentSceneIndex == 15) {
  }
  if (currentSceneIndex == 16) {
  }
  if (currentSceneIndex == 17) {
  }
  if (currentSceneIndex == 18) {
  }
  if (currentSceneIndex == 19) {
  }
  if (currentSceneIndex == 20) {
  }
  if (currentSceneIndex == 21) {
  }
  if (currentSceneIndex == 22) {
  }
  if (currentSceneIndex == 23) {
  }
  if (currentSceneIndex == 24) {
  }
  if (currentSceneIndex == 25) {
    if (millis() - lastScrollTime_25_text_1782801231231 > 80) {
      scrollOffset_25_text_1782801231231--;
      if (scrollOffset_25_text_1782801231231 < -200)
        scrollOffset_25_text_1782801231231 = 80;
      lastScrollTime_25_text_1782801231231 = millis();
    }
    if (millis() - lastScrollTime_25_text_1782801265079_577 > 80) {
      scrollOffset_25_text_1782801265079_577++;
      if (scrollOffset_25_text_1782801265079_577 > 80)
        scrollOffset_25_text_1782801265079_577 = -200;
      lastScrollTime_25_text_1782801265079_577 = millis();
    }
  }

  // 6. Draw current screen depending on Navigation mode
  if (!liveModeActive) {
    matrix.clear();

    if (currentState == STATE_SCENE_PLAY) {
      // Render current active scene
      switch (currentSceneIndex) {
      case 0:
        renderScene_0();
        break;
      case 1:
        renderScene_1();
        break;
      case 2:
        renderScene_2();
        break;
      case 3:
        renderScene_3();
        break;
      case 4:
        renderScene_4();
        break;
      case 5:
        renderScene_5();
        break;
      case 6:
        renderScene_6();
        break;
      case 7:
        renderScene_7();
        break;
      case 8:
        renderScene_8();
        break;
      case 9:
        renderScene_9();
        break;
      case 10:
        renderScene_10();
        break;
      case 11:
        renderScene_11();
        break;
      case 12:
        renderScene_12();
        break;
      case 13:
        renderScene_13();
        break;
      case 14:
        renderScene_14();
        break;
      case 15:
        renderScene_15();
        break;
      case 16:
        renderScene_16();
        break;
      case 17:
        renderScene_17();
        break;
      case 18:
        renderScene_18();
        break;
      case 19:
        renderScene_19();
        break;
      case 20:
        renderScene_20();
        break;
      case 21:
        renderScene_21();
        break;
      case 22:
        renderScene_22();
        break;
      case 23:
        renderScene_23();
        break;
      case 24:
        renderScene_24();
        break;
      case 25:
        renderScene_25();
        break;
      }
    } else if (currentState == STATE_TIMER_ADJUST) {
      // Render timer adjust screen (no solid background)
      drawText("ADJUST TIMER", 6, 1, 0xec4899, 1, 0, 0, 0);

      int mins = countdownSeconds / 60;
      int secs = countdownSeconds % 60;
      char timeStr[10];
      sprintf(timeStr, "%02d:%02d", mins, secs);
      drawText(timeStr, 26, 9, 0x06b6d4, 1, 0, 0, 0);
    } else if (currentState == STATE_MENU_NAV) {
      // Menu navigation screen (no solid background, displays two options)
      int menuSize = NUM_SCENES + 1;
      int firstIdx = menuCursor;
      if (firstIdx > menuSize - 2) {
        firstIdx = menuSize - 2;
      }
      if (firstIdx < 0) {
        firstIdx = 0;
      }
      int secondIdx = firstIdx + 1;

      String firstOpt = getSceneName(firstIdx);
      String secondOpt = (secondIdx < menuSize) ? getSceneName(secondIdx) : "";

      if (menuCursor == firstIdx) {
        drawText("> " + firstOpt, 4, 1, 0xffffff, 1, 0, 0, 0);
      } else {
        drawText("  " + firstOpt, 4, 1, 0x8c8c8c, 1, 0, 0, 0);
      }

      if (secondIdx < menuSize) {
        if (menuCursor == secondIdx) {
          drawText("> " + secondOpt, 4, 9, 0xffffff, 1, 0, 0, 0);
        } else {
          drawText("  " + secondOpt, 4, 9, 0x8c8c8c, 1, 0, 0, 0);
        }
      }
    } else if (currentState == STATE_BRIGHTNESS_ADJUST) {
      // Brightness page
      char brightStr[20];
      sprintf(brightStr, "BRIGHT: %d%%", currentBrightness);
      drawText(String(brightStr), 4, 1, 0x00ffcc, 1, 0, 0, 0);

      // Draw progress bar background track
      for (int px = 4; px <= 75; px++) {
        for (int py = 10; py <= 13; py++) {
          setPixel(px, py, 0x222222);
        }
      }

      // Draw filled bar
      int barWidth = (currentBrightness * 72) / 100;
      for (int px = 4; px < 4 + barWidth; px++) {
        for (int py = 10; py <= 13; py++) {
          float ratio = (float)(px - 4) / 72.0;
          uint8_t r = (uint8_t)(0x06 * (1.0 - ratio) + 0x3b * ratio);
          uint8_t g = (uint8_t)(0xb6 * (1.0 - ratio) + 0x82 * ratio);
          uint8_t b = (uint8_t)(0xd4 * (1.0 - ratio) + 0xf6 * ratio);
          setPixel(px, py, r, g, b);
        }
      }
    }

    matrix.show();
  }
  if (!liveModeActive) {
    delay(16);
  } else {
    delay(1);
  }
}

// ISR Encoder Trigger
void IRAM_ATTR handleEncoderInterrupt() {
  int aVal = digitalRead(ENCODER_A);
  int bVal = digitalRead(ENCODER_B);
  static int lastA = LOW;

  if (aVal != lastA) {
    if (digitalRead(ENCODER_B) != aVal) {
      encoderPos++;
    } else {
      encoderPos--;
    }
  }
  lastA = aVal;
}

void checkEncoder() {
  int currentSteps = encoderPos / 2;
  if (currentSteps != lastReportedPos) {
    int diff = currentSteps - lastReportedPos;
    lastReportedPos = currentSteps;

    // Register physical interaction to override streaming mode
    lastPhysicalInteractionTime = millis();
    liveModeActive = false;

    int step = (diff > 0) ? 1 : -1;

    if (currentState == STATE_SCENE_PLAY) {
      // Navigate scenes
      currentSceneIndex = (currentSceneIndex + step) % NUM_SCENES;
      if (currentSceneIndex < 0)
        currentSceneIndex += NUM_SCENES;
      Serial.print("Switched scene to: ");
      Serial.println(currentSceneIndex);
    } else if (currentState == STATE_MENU_NAV) {
      // Navigate menu options
      int menuSize = NUM_SCENES + 1;
      menuCursor = (menuCursor + step) % menuSize;
      if (menuCursor < 0)
        menuCursor += menuSize;
      Serial.print("Switched menu cursor to: ");
      Serial.println(menuCursor);
    } else if (currentState == STATE_TIMER_ADJUST) {
      // Adjust timer duration (adds/subtracts 10 seconds per click)
      countdownSeconds += step * 10;
      if (countdownSeconds < 10)
        countdownSeconds = 10;
      Serial.print("Countdown adjusted to: ");
      Serial.println(countdownSeconds);
    } else if (currentState == STATE_BRIGHTNESS_ADJUST) {
      // Adjust brightness in 1% steps
      currentBrightness += step;
      if (currentBrightness < 0)
        currentBrightness = 0;
      if (currentBrightness > 100)
        currentBrightness = 100;
      matrix.setBrightness((currentBrightness * 255) / 100);
      Serial.print("Brightness adjusted to: ");
      Serial.print(currentBrightness);
      Serial.println("%");
    }
  }
}

void checkButton() {
  static bool lastBtnState = HIGH;
  bool btnState = digitalRead(ENCODER_C);

  if (btnState != lastBtnState) {
    delay(5); // Soft debounce
    if (btnState == LOW) {
      // Button Pressed
      unsigned long pressTime = millis();

      // Register physical interaction to override streaming mode
      lastPhysicalInteractionTime = millis();
      liveModeActive = false;

      if (pressTime - buttonPressTime < doubleClickTime) {
        clickCount++;
      } else {
        clickCount = 1;
      }
      buttonPressTime = pressTime;
      buttonPressed = true;
    }
    lastBtnState = btnState;
  }

  // Check click timeout to distinguish Single click vs Double Click
  if (buttonPressed && (millis() - buttonPressTime > doubleClickTime)) {
    // Register physical interaction to override streaming mode
    lastPhysicalInteractionTime = millis();
    liveModeActive = false;

    if (clickCount >= 2) {
      // Double Push: Leads to Preset Menu
      Serial.println("Double Push: Opening Menu");
      if (currentState == STATE_MENU_NAV) {
        currentState = STATE_SCENE_PLAY;
      } else if (currentState == STATE_TIMER_ADJUST ||
                 currentState == STATE_BRIGHTNESS_ADJUST) {
        currentState = STATE_SCENE_PLAY;
      } else {
        menuCursor = currentSceneIndex;
        currentState = STATE_MENU_NAV;
      }
    } else if (clickCount == 1) {
      // Single Push: Toggle timer settings, increment counter, or save settings
      Serial.println("Single Push triggered");
      if (currentState == STATE_SCENE_PLAY) {
        bool handled = false;
        switch (currentSceneIndex) {
        case 8:
          counter_counter_1782374184679++;
          handled = true;
          break;
        }

        if (!handled && currentSceneHasTimer()) {
          currentState = STATE_TIMER_ADJUST;
        }
      } else if (currentState == STATE_TIMER_ADJUST) {
        timerRunning = !timerRunning; // Start/Stop timer
        currentState = STATE_SCENE_PLAY;
      } else if (currentState == STATE_MENU_NAV) {
        if (menuCursor == NUM_SCENES) {
          currentState = STATE_BRIGHTNESS_ADJUST;
        } else {
          currentSceneIndex = menuCursor;
          currentState = STATE_SCENE_PLAY; // Select and play
        }
      } else if (currentState == STATE_BRIGHTNESS_ADJUST) {
        currentState = STATE_MENU_NAV;
      }
    }
    buttonPressed = false;
    clickCount = 0;
  }
}

// Fetch OWM Details
void updateWeather() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  String url =
      "http://api.openweathermap.org/data/2.5/weather?q=" + String(OWM_CITY) +
      "," + String(OWM_COUNTRY) + "&appid=" + String(OWM_KEY) + "&units=metric";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      weatherTemp = doc["main"]["temp"];
      weatherHumidity = doc["main"]["humidity"];
      const char *desc = doc["weather"][0]["main"];
      weatherDesc = String(desc);
      lastWeatherUpdate = millis();
      Serial.println("Weather information updated successfully.");
    }
  }
  http.end();
}

// Fetch YouTube statistics
void updateYouTubeSubscribers() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  lastYtUpdate = millis();
  ytLastUpdateSuccess = false; // Default to false, set to true on success

  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate verification

  HTTPClient http;
  String url =
      "https://www.googleapis.com/youtube/v3/channels?part=statistics&id=" +
      String(YT_CHANNEL_ID) + "&key=" + String(YT_API_KEY);

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        if (doc["items"] && doc["items"].size() > 0) {
          const char *subCountStr =
              doc["items"][0]["statistics"]["subscriberCount"];
          if (subCountStr) {
            ytSubscriberCount = atol(subCountStr);
            ytLastUpdateSuccess = true;
            Serial.println("YouTube Subscription Count updated successfully: " +
                           String(ytSubscriberCount));
          } else {
            Serial.println(
                "YouTube Stats: 'subscriberCount' missing in statistics.");
          }
        } else {
          Serial.println("YouTube Stats: Channel items array is empty. Verify "
                         "Channel ID starts with UC.");
        }
      } else {
        Serial.print("YouTube JSON Deserialization failed: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("YouTube HTTP GET failed, error: ");
      Serial.println(httpCode);
    }
    http.end();
  }
}

void handleSerialCommands() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.startsWith("HELLO")) {
      Serial.println("ACK:PixelStudio");
    } else if (line.startsWith("SET_SCENE:")) {
      int idx = line.substring(10).toInt();
      if (idx >= 0 && idx < NUM_SCENES) {
        currentSceneIndex = idx;
        currentState = STATE_SCENE_PLAY;
        Serial.println("OK:SceneSwitched");
      }
    }
  }
}

#include <base64.h>
#include <mbedtls/sha1.h>

String getWebSocketAcceptKey(String clientKey) {
  String concat = clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  uint8_t sha1Hash[20];
  mbedtls_sha1_context ctx;
  mbedtls_sha1_init(&ctx);
  mbedtls_sha1_starts(&ctx);
  mbedtls_sha1_update(&ctx, (const unsigned char *)concat.c_str(),
                      concat.length());
  mbedtls_sha1_finish(&ctx, sha1Hash);
  mbedtls_sha1_free(&ctx);
  return base64::encode(sha1Hash, 20);
}

// Read binary WebSocket frames
bool readWebSocketFrame(WiFiClient &client, uint8_t *outBuffer,
                        int expectedLen) {
  while (client.connected()) {
    if (client.available() < 2) {
      return false; // No data available yet
    }

    uint8_t header0 = client.read();
    uint8_t header1 = client.read();
    uint8_t opcode = header0 & 0x0F;
    bool masked = (header1 & 0x80) != 0;
    uint32_t payloadLen = header1 & 0x7F;

    client.setTimeout(1000);

    if (payloadLen == 126) {
      uint8_t lenBytes[2];
      if (client.readBytes(lenBytes, 2) != 2) {
        client.stop();
        return false;
      }
      payloadLen = (lenBytes[0] << 8) | lenBytes[1];
    } else if (payloadLen == 127) {
      uint8_t lenBytes[8];
      if (client.readBytes(lenBytes, 8) != 8) {
        client.stop();
        return false;
      }
      payloadLen = ((uint32_t)lenBytes[4] << 24) |
                   ((uint32_t)lenBytes[5] << 16) |
                   ((uint32_t)lenBytes[6] << 8) | lenBytes[7];
    }

    uint8_t maskKey[4] = {0, 0, 0, 0};
    if (masked) {
      if (client.readBytes(maskKey, 4) != 4) {
        client.stop();
        return false;
      }
    }

    // Read the payload
    uint8_t tempBuf[128]; // Temporary buffer for control frame payloads
    uint8_t *targetBuf = outBuffer;

    // If it's a control frame or not our binary frame, read it into tempBuf
    if (opcode != 0x02) { // 0x02 is Binary frame
      targetBuf = tempBuf;
    }

    if (payloadLen > 0) {
      uint32_t toRead = payloadLen;
      if (opcode == 0x02) {
        if (toRead > (uint32_t)expectedLen)
          toRead = expectedLen;
      } else {
        if (toRead > sizeof(tempBuf))
          toRead = sizeof(tempBuf);
      }

      if (client.readBytes(targetBuf, toRead) != toRead) {
        client.stop();
        return false;
      }

      // Discard any overflow payload
      if (payloadLen > toRead) {
        for (uint32_t i = toRead; i < payloadLen; i++) {
          client.read();
        }
      }

      // Unmask
      if (masked) {
        for (uint32_t i = 0; i < toRead; i++) {
          targetBuf[i] ^= maskKey[i % 4];
        }
      }
    }

    if (opcode == 0x08) { // Connection Close
      Serial.println("WebSocket close frame received.");
      client.stop();
      return false;
    } else if (opcode == 0x09) { // Ping
      // Respond with Pong (opcode 0x0A)
      uint8_t pongHeader[2];
      pongHeader[0] = 0x8A;              // Fin + Pong opcode (0x0A)
      pongHeader[1] = payloadLen & 0x7F; // Unmasked payload length
      client.write(pongHeader, 2);
      if (payloadLen > 0) {
        client.write(tempBuf, payloadLen);
      }
      // Continue loop to read the actual binary frame next!
      continue;
    } else if (opcode == 0x02) { // Binary Frame
      return (payloadLen == (uint32_t)expectedLen);
    } else {
      // Discard text or other frames, check next
      continue;
    }
  }
  return false;
}

// Process live WebSocket stream frames
void handleWiFiFrames() {
  if (!wsClient || !wsClient.connected()) {
    wsClient = wsServer.available();
    if (wsClient) {
      String req = "";
      unsigned long startRead = millis();
      while (wsClient.connected() && millis() - startRead < 1000) {
        if (wsClient.available()) {
          char c = wsClient.read();
          req += c;
          if (req.endsWith("\r\n\r\n"))
            break;
        }
      }
      if (req.indexOf("Upgrade: websocket") != -1) {
        int keyIdx = req.indexOf("Sec-WebSocket-Key: ");
        if (keyIdx != -1) {
          int endIdx = req.indexOf("\r\n", keyIdx);
          String key = req.substring(keyIdx + 19, endIdx);
          key.trim();
          String acceptKey = getWebSocketAcceptKey(key);
          wsClient.print("HTTP/1.1 101 Switching Protocols\r\n");
          wsClient.print("Upgrade: websocket\r\n");
          wsClient.print("Connection: Upgrade\r\n");
          wsClient.print("Sec-WebSocket-Accept: " + acceptKey + "\r\n\r\n");
          wsClient.setNoDelay(true); // Disable Nagle's algorithm for
                                     // low-latency WiFi streaming!
          liveModeActive = true;
          lastLiveFrameTime = millis();
        }
      } else {
        wsClient.stop();
      }
    }
  }
  if (wsClient && wsClient.connected()) {
    if (wsClient.available() >= 2) {
      uint8_t frameBuf[3840];
      if (readWebSocketFrame(wsClient, frameBuf, 3840)) {
        if (millis() - lastPhysicalInteractionTime > 5000) {
          for (int y = 0; y < MATRIX_HEIGHT; y++) {
            for (int x = 0; x < MATRIX_WIDTH; x++) {
              int idx = (y * MATRIX_WIDTH + x) * 3;
              setPixel(x, y, frameBuf[idx], frameBuf[idx + 1],
                       frameBuf[idx + 2]);
            }
          }
          matrix.show();
          liveModeActive = true;
          lastLiveFrameTime = millis();
        }
      } else {
        Serial.println(
            "WebSocket frame read failed. Closing client connection.");
        wsClient.stop();
      }
    }
  }
}

// Draw Shape implementation
void drawShape(String shapeType, int x, int y, int w, int h, uint32_t color,
               bool filled, uint32_t borderColor, int borderWidth,
               String motion, int speed, int cornerRadius) {
  int startX = x;
  int startY = y;

  if (motion == "wobble") {
    startX += (int)(sin(millis() * speed * 0.001) * 2.0);
    startY += (int)(cos(millis() * speed * 0.001) * 1.0);
  } else if (motion == "wave") {
    startY += (int)(sin(millis() * speed * 0.001) * 2.5);
  } else if (motion == "bounce") {
    startY += (int)(abs(sin(millis() * speed * 0.001)) * 3.0);
  } else if (motion == "orbit") {
    startX += (int)(cos(millis() * speed * 0.001) * 2.5);
    startY += (int)(sin(millis() * speed * 0.001) * 2.5);
  } else if (motion == "blink" && ((int)(millis() * speed * 0.001) % 2) == 0) {
    return;
  } else if (motion == "glitch") {
    if (random(0, 100) > 80) {
      startX += random(-1, 2);
      startY += random(-1, 2);
    }
  } else if (motion == "scroll-left") {
    float tick = millis() / 50.0;
    int totalRange = MATRIX_WIDTH + w;
    int offset = ((int)(tick * speed * 0.4)) % totalRange;
    startX = x - offset;
    if (startX < -w) {
      startX += totalRange;
    }
  } else if (motion == "scroll-right") {
    float tick = millis() / 50.0;
    int totalRange = MATRIX_WIDTH + w;
    int offset = ((int)(tick * speed * 0.4)) % totalRange;
    startX = x + offset;
    if (startX >= MATRIX_WIDTH) {
      startX -= totalRange;
    }
  } else if (motion == "scroll-up") {
    float tick = millis() / 50.0;
    int totalRange = MATRIX_HEIGHT + h;
    int offset = ((int)(tick * speed * 0.4)) % totalRange;
    startY = y - offset;
    if (startY < -h) {
      startY += totalRange;
    }
  } else if (motion == "scroll-down") {
    float tick = millis() / 50.0;
    int totalRange = MATRIX_HEIGHT + h;
    int offset = ((int)(tick * speed * 0.4)) % totalRange;
    startY = y + offset;
    if (startY >= MATRIX_HEIGHT) {
      startY -= totalRange;
    }
  }

  float angle = millis() * speed * 0.0004;
  float cosA = cos(angle);
  float sinA = sin(angle);
  float hW = (w - 1) / 2.0;
  float hH = (h - 1) / 2.0;

  // Draw pixel helper mapping local coords to matrix screen coords
  auto drawPixel = [&](int sx, int sy, uint32_t col) {
    int finalSx = sx;
    int finalSy = sy;
    if (motion == "rotate") {
      float rx = sx - hW;
      float ry = sy - hH;
      finalSx = round(hW + rx * cosA - ry * sinA);
      finalSy = round(hH + rx * sinA + ry * cosA);
    }
    int px = startX + finalSx;
    int py = startY + finalSy;
    if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
      setPixel(px, py, col);
    }
  };

  if (shapeType == "rect") {
    for (int sy = 0; sy < h; sy++) {
      for (int sx = 0; sx < w; sx++) {
        bool isInside = true;
        bool drawBorder = false;

        bool onStraightBorder =
            (((sx >= cornerRadius && sx < w - cornerRadius) &&
              (sy < borderWidth || sy >= h - borderWidth)) ||
             ((sy >= cornerRadius && sy < h - cornerRadius) &&
              (sx < borderWidth || sx >= w - borderWidth)));

        if (onStraightBorder && borderWidth > 0) {
          drawBorder = true;
        }

        float cx = 0, cy = 0;
        bool inCorner = false;

        if (cornerRadius > 0) {
          if (sx < cornerRadius && sy < cornerRadius) {
            cx = cornerRadius - 0.5f;
            cy = cornerRadius - 0.5f;
            inCorner = true;
          } else if (sx >= w - cornerRadius && sy < cornerRadius) {
            cx = w - cornerRadius - 0.5f;
            cy = cornerRadius - 0.5f;
            inCorner = true;
          } else if (sx < cornerRadius && sy >= h - cornerRadius) {
            cx = cornerRadius - 0.5f;
            cy = h - cornerRadius - 0.5f;
            inCorner = true;
          } else if (sx >= w - cornerRadius && sy >= h - cornerRadius) {
            cx = w - cornerRadius - 0.5f;
            cy = h - cornerRadius - 0.5f;
            inCorner = true;
          }
        }

        if (inCorner) {
          float dx = sx - cx;
          float dy = sy - cy;
          float dist = sqrt(dx * dx + dy * dy);
          if (dist > cornerRadius) {
            isInside = false;
          } else if (borderWidth > 0 && dist >= cornerRadius - borderWidth) {
            drawBorder = true;
          }
        } else if (!onStraightBorder) {
          if (!filled) {
            isInside = false;
          }
        }

        if (isInside) {
          if (drawBorder && borderWidth > 0) {
            drawPixel(sx, sy, borderColor);
          } else if (filled) {
            drawPixel(sx, sy, color);
          }
        }
      }
    }
  } else if (shapeType == "circle") {
    float cx = (w - 1) / 2.0f;
    float cy = (h - 1) / 2.0f;
    float rx = w / 2.0f;
    float ry = h / 2.0f;

    for (int sy = 0; sy < h; sy++) {
      for (int sx = 0; sx < w; sx++) {
        float dx = (sx - cx) / rx;
        float dy = (sy - cy) / ry;
        float dist = dx * dx + dy * dy;

        if (dist <= 1.0f) {
          float innerRx = rx - borderWidth;
          float innerRy = ry - borderWidth;
          float innerDx = (sx - cx) / (innerRx > 0.5f ? innerRx : 0.5f);
          float innerDy = (sy - cy) / (innerRy > 0.5f ? innerRy : 0.5f);
          float innerDist = innerDx * innerDx + innerDy * innerDy;

          if (borderWidth > 0 && innerDist > 1.0f) {
            drawPixel(sx, sy, borderColor);
          } else if (filled) {
            drawPixel(sx, sy, color);
          }
        }
      }
    }
  } else if (shapeType == "triangle") {
    for (int sy = 0; sy < h; sy++) {
      float progress = (float)sy / (float)(h - 1 > 0 ? h - 1 : 1);
      float startX_val = ((w - 1) / 2.0f) * (1.0f - progress);
      float endX_val = (w - 1) - startX_val;

      for (int sx = 0; sx < w; sx++) {
        if (sx >= startX_val && sx <= endX_val) {
          bool isBorder = (sy < borderWidth || sx < startX_val + borderWidth ||
                           sx > endX_val - borderWidth);

          if (isBorder && borderWidth > 0) {
            drawPixel(sx, sy, borderColor);
          } else if (filled) {
            drawPixel(sx, sy, color);
          }
        }
      }
    }
  } else if (shapeType == "line") {
    int x1 = 0;
    int y1 = 0;
    int x2 = w - 1;
    int y2 = h - 1;
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (true) {
      drawPixel(x1, y1, color);
      if (x1 == x2 && y1 == y2)
        break;
      int e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        x1 += sx;
      }
      if (e2 < dx) {
        err += dx;
        y1 += sy;
      }
    }
  } else if (shapeType == "hline") {
    int thickness = h > 1 ? h : 1;
    for (int sy = 0; sy < thickness; sy++) {
      for (int sx = 0; sx < w; sx++) {
        drawPixel(sx, sy, color);
      }
    }
  } else if (shapeType == "vline") {
    int thickness = w > 1 ? w : 1;
    for (int sy = 0; sy < h; sy++) {
      for (int sx = 0; sx < thickness; sx++) {
        drawPixel(sx, sy, color);
      }
    }
  }
}

// Drawing Functions

// Drawing Functions
void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, int opacity) {
  if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
    return;
  int index;
  if (y % 2 == 0) {
    index = y * MATRIX_WIDTH + x;
  } else {
    index = y * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - x);
  }
  if (opacity <= 0)
    return;
  if (opacity >= 100) {
    matrix.setPixelColor(index, r, g, b);
  } else {
    uint32_t current = matrix.getPixelColor(index);
    uint8_t currR = (current >> 16) & 0xFF;
    uint8_t currG = (current >> 8) & 0xFF;
    uint8_t currB = current & 0xFF;

    uint8_t blendedR = (r * opacity + currR * (100 - opacity)) / 100;
    uint8_t blendedG = (g * opacity + currG * (100 - opacity)) / 100;
    uint8_t blendedB = (b * opacity + currB * (100 - opacity)) / 100;

    matrix.setPixelColor(index, blendedR, blendedG, blendedB);
  }
}

void setPixel(int x, int y, uint32_t color, int opacity) {
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  setPixel(x, y, r, g, b, opacity);
}

void drawSolidBackground(uint32_t color, int opacity) {
  uint8_t r = (((color >> 16) & 0xFF) * opacity) / 100;
  uint8_t g = (((color >> 8) & 0xFF) * opacity) / 100;
  uint8_t b = ((color & 0xFF) * opacity) / 100;
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, r, g, b);
  }
}

void drawGradientBackground(uint32_t color1, uint32_t color2, int opacity) {
  uint8_t r1 = (color1 >> 16) & 0xFF;
  uint8_t g1 = (color1 >> 8) & 0xFF;
  uint8_t b1 = color1 & 0xFF;

  uint8_t r2 = (color2 >> 16) & 0xFF;
  uint8_t g2 = (color2 >> 8) & 0xFF;
  uint8_t b2 = color2 & 0xFF;

  for (int x = 0; x < MATRIX_WIDTH; x++) {
    float ratio = (float)x / (float)(MATRIX_WIDTH - 1);
    uint8_t r = ((uint8_t)(r1 + ratio * (r2 - r1)) * opacity) / 100;
    uint8_t g = ((uint8_t)(g1 + ratio * (g2 - g1)) * opacity) / 100;
    uint8_t b = ((uint8_t)(b1 + ratio * (b2 - b1)) * opacity) / 100;
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      setPixel(x, y, r, g, b);
    }
  }
}

void drawText(String txt, int x, int y, int w, int h, uint32_t color, int size,
              bool shadow, uint32_t shadowColor, String effect, int speed,
              bool bold, bool rainbow) {
  int charWidth = (size == 0) ? 3 : 5 * size;
  int charHeight = (size == 0) ? 5 : 8 * size;
  int charSpacing = (size == 0) ? 1 : 1 * size;
  if (bold) {
    charSpacing += 1;
  }
  int charStep = charWidth + charSpacing;
  int textWidth = txt.length() * charStep - charSpacing;

  float tick = millis() / 50.0;
  int scrollX = 0;
  int scrollY = 0;

  if (effect == "left") {
    int totalRange = w + textWidth;
    if (totalRange > 0) {
      scrollX = w - ((int)(tick * speed * 0.4) % totalRange);
    }
  } else if (effect == "right") {
    int totalRange = w + textWidth;
    if (totalRange > 0) {
      scrollX = -textWidth + ((int)(tick * speed * 0.4) % totalRange);
    }
  } else if (effect == "bounce") {
    int range = w - textWidth;
    if (range > 0) {
      scrollX = (sin(tick * 0.05 * speed) + 1.0) * 0.5 * range;
    } else {
      scrollX = 0;
    }
  } else if (effect == "top") {
    int totalRange = h + charHeight;
    if (totalRange > 0) {
      scrollY = h - ((int)(tick * speed * 0.4) % totalRange);
    }
  } else if (effect == "bottom") {
    int totalRange = h + charHeight;
    if (totalRange > 0) {
      scrollY = -charHeight + ((int)(tick * speed * 0.4) % totalRange);
    }
  } else if (effect == "up-down") {
    int range = h - charHeight;
    if (range > 0) {
      scrollY = (sin(tick * 0.05 * speed) + 1.0) * 0.5 * range;
    } else {
      scrollY = 0;
    }
  }

  int textX = x + scrollX;

  if (size == 0) {
    // Render manual tiny font
    int curX = textX;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    uint8_t sr, sg, sb;
    if (shadowColor != 0) {
      sr = (shadowColor >> 16) & 0xFF;
      sg = (shadowColor >> 8) & 0xFF;
      sb = shadowColor & 0xFF;
    } else {
      sr = r / 4;
      sg = g / 4;
      sb = b / 4;
    }

    for (int i = 0; i < txt.length(); i++) {
      char c = txt[i];
      int ascii = (int)c;
      if (ascii < 32 || ascii > 127)
        ascii = 63; // '?'
      int fontIdx = ascii - 32;

      int charScrollY = y + scrollY;
      if (effect == "wave") {
        charScrollY =
            y + scrollY + (int)(sin(tick * 0.2 + (x + i * 6) * 0.1) * 1.5);
      }

      int finalOpacity = 100;
      if (effect == "twinkle") {
        finalOpacity =
            (int)((0.3 + 0.7 * abs(sin(tick * 0.1 + i * 0.5))) * 100);
      } else if (effect == "glow") {
        finalOpacity = (int)((0.2 + 0.8 * abs(sin(tick * 0.15))) * 100);
      }

      uint8_t finalR = r;
      uint8_t finalG = g;
      uint8_t finalB = b;
      if (effect == "shimmer") {
        float cycle = (sin(tick * 0.15 - i * 0.3) + 1.0) / 2.0;
        if (cycle > 0.7) {
          finalR = 255;
          finalG = 255;
          finalB = 255;
        }
      }

      int defaultStartX = curX;
      int startX = defaultStartX;
      float cosTheta = 1.0;
      if (effect == "rotate-3d") {
        cosTheta = cos(tick * 0.05 * speed);
        float textCenter = textX + textWidth / 2.0;
        float defaultCharCenter = defaultStartX + charWidth / 2.0;
        float charCenter =
            textCenter + (defaultCharCenter - textCenter) * cosTheta;
        startX = charCenter - (charWidth * abs(cosTheta)) / 2.0;
      }

      // Draw shadow first
      if (shadow) {
        for (int col = 0; col < 3; col++) {
          int drawCol = cosTheta < 0 ? (3 - 1 - col) : col;
          uint8_t colData = pgm_read_byte(&(tiny_font[fontIdx][drawCol]));
          for (int row = 0; row < 5; row++) {
            if ((colData >> row) & 1) {
              int px = startX + (int)(col * abs(cosTheta)) + 1;
              int py = charScrollY + row + 1;
              if (px >= x && px < x + w && py >= y && py < y + h) {
                setPixel(px, py, sr, sg, sb, finalOpacity);
                if (bold) {
                  setPixel(px + 1, py, sr, sg, sb, finalOpacity);
                }
              }
            }
          }
        }
      }

      // Draw primary character pixels
      for (int col = 0; col < 3; col++) {
        int drawCol = cosTheta < 0 ? (3 - 1 - col) : col;
        uint8_t colData = pgm_read_byte(&(tiny_font[fontIdx][drawCol]));
        for (int row = 0; row < 5; row++) {
          if ((colData >> row) & 1) {
            int px = startX + (int)(col * abs(cosTheta));
            int py = charScrollY + row;
            if (px >= x && px < x + w && py >= y && py < y + h) {
              if (rainbow) {
                uint16_t hue = ((int)(tick * 8 + px * 4) % 360) * 65535 / 360;
                uint32_t rbColor = matrix.ColorHSV(hue, 255, 255);
                setPixel(px, py, rbColor, finalOpacity);
                if (bold) {
                  setPixel(px + 1, py, rbColor, finalOpacity);
                }
              } else {
                setPixel(px, py, finalR, finalG, finalB, finalOpacity);
                if (bold) {
                  setPixel(px + 1, py, finalR, finalG, finalB, finalOpacity);
                }
              }
            }
          }
        }
      }
      curX += (bold ? 5 : 4);
    }
  } else {
    // Standard Adafruit GFX text rendering using offscreen textCanvas
    // (character by character)
    int curX = textX;
    int charWidth = 5 * size;
    int charSpacing = 1 * size + (bold ? 1 : 0);
    int charStep = charWidth + charSpacing;

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    uint8_t sr, sg, sb;
    if (shadowColor != 0) {
      sr = (shadowColor >> 16) & 0xFF;
      sg = (shadowColor >> 8) & 0xFF;
      sb = shadowColor & 0xFF;
    } else {
      sr = r / 4;
      sg = g / 4;
      sb = b / 4;
    }

    for (int i = 0; i < txt.length(); i++) {
      char c = txt[i];

      int charScrollY = y + scrollY;
      if (effect == "wave") {
        charScrollY =
            y + scrollY + (int)(sin(tick * 0.2 + (x + i * 6) * 0.1) * 1.5);
      }

      int finalOpacity = 100;
      if (effect == "twinkle") {
        finalOpacity =
            (int)((0.3 + 0.7 * abs(sin(tick * 0.1 + i * 0.5))) * 100);
      } else if (effect == "glow") {
        finalOpacity = (int)((0.2 + 0.8 * abs(sin(tick * 0.15))) * 100);
      }

      uint8_t finalR = r;
      uint8_t finalG = g;
      uint8_t finalB = b;
      if (effect == "shimmer") {
        float cycle = (sin(tick * 0.15 - i * 0.3) + 1.0) / 2.0;
        if (cycle > 0.7) {
          finalR = 255;
          finalG = 255;
          finalB = 255;
        }
      }

      // Render single character
      textCanvas.fillScreen(0);
      textCanvas.setTextSize(size);
      textCanvas.setTextWrap(false);
      textCanvas.setTextColor(1);
      textCanvas.setCursor(0, 0);
      textCanvas.print(c);

      int defaultStartX = curX;
      int startX = defaultStartX;
      float cosTheta = 1.0;
      if (effect == "rotate-3d") {
        cosTheta = cos(tick * 0.05 * speed);
        float textCenter = textX + textWidth / 2.0;
        float defaultCharCenter = defaultStartX + charWidth / 2.0;
        float charCenter =
            textCenter + (defaultCharCenter - textCenter) * cosTheta;
        startX = charCenter - (charWidth * abs(cosTheta)) / 2.0;
      }

      // Draw shadow first
      if (shadow) {
        for (int cy = 0; cy < MATRIX_HEIGHT; cy++) {
          for (int cx = 0; cx < charWidth; cx++) {
            int drawCx = cosTheta < 0 ? (charWidth - 1 - cx) : cx;
            if (textCanvas.getPixel(drawCx, cy)) {
              int px = startX + (int)(cx * abs(cosTheta)) + 1;
              int py = charScrollY + cy + 1;
              if (px >= x && px < x + w && py >= y && py < y + h) {
                setPixel(px, py, sr, sg, sb, finalOpacity);
                if (bold) {
                  setPixel(px + 1, py, sr, sg, sb, finalOpacity);
                }
              }
            }
          }
        }
      }

      // Draw main character pixels
      for (int cy = 0; cy < MATRIX_HEIGHT; cy++) {
        for (int cx = 0; cx < charWidth; cx++) {
          int drawCx = cosTheta < 0 ? (charWidth - 1 - cx) : cx;
          if (textCanvas.getPixel(drawCx, cy)) {
            int px = startX + (int)(cx * abs(cosTheta));
            int py = charScrollY + cy;
            if (px >= x && px < x + w && py >= y && py < y + h) {
              if (rainbow) {
                uint16_t hue = ((int)(tick * 8 + px * 4) % 360) * 65535 / 360;
                uint32_t rbColor = matrix.ColorHSV(hue, 255, 255);
                setPixel(px, py, rbColor, finalOpacity);
                if (bold) {
                  setPixel(px + 1, py, rbColor, finalOpacity);
                }
              } else {
                setPixel(px, py, finalR, finalG, finalB, finalOpacity);
                if (bold) {
                  setPixel(px + 1, py, finalR, finalG, finalB, finalOpacity);
                }
              }
            }
          }
        }
      }
      curX += charStep;
    }
  }
}
void drawText(String txt, int x, int y, uint32_t color, int size, bool shadow,
              uint32_t shadowColor, int scrollOffset, bool bold, bool rainbow) {
  drawText(txt, x + scrollOffset, y, MATRIX_WIDTH, MATRIX_HEIGHT, color, size,
           shadow, shadowColor, "none", 4, bold, rainbow);
}

void drawSticker(const uint32_t *sticker, int x, int y, int w, int h,
                 String motion, int speed) {
  int startX = x;
  int startY = y;

  if (motion == "wobble") {
    startX += (int)(sin(millis() * speed * 0.001) * 2.0);
    startY += (int)(cos(millis() * speed * 0.001) * 1.0);
  } else if (motion == "wave") {
    startY += (int)(sin(millis() * speed * 0.001) * 2.5);
  } else if (motion == "bounce") {
    startY += (int)(abs(sin(millis() * speed * 0.001)) * 3.0);
  } else if (motion == "orbit") {
    startX += (int)(cos(millis() * speed * 0.001) * 2.5);
    startY += (int)(sin(millis() * speed * 0.001) * 2.5);
  } else if (motion == "blink" && ((int)(millis() * speed * 0.001) % 2) == 0) {
    return;
  } else if (motion == "glitch") {
    if (random(0, 100) > 80) {
      startX += random(-1, 2);
      startY += random(-1, 2);
    }
  } else if (motion == "scroll-left") {
    float tick = millis() / 50.0;
    int totalRange = MATRIX_WIDTH + w;
    int offset = ((int)(tick * speed * 0.4)) % totalRange;
    startX = x - offset;
    if (startX < -w) {
      startX += totalRange;
    }
  } else if (motion == "scroll-right") {
    float tick = millis() / 50.0;
    int totalRange = MATRIX_WIDTH + w;
    int offset = ((int)(tick * speed * 0.4)) % totalRange;
    startX = x + offset;
    if (startX >= MATRIX_WIDTH) {
      startX -= totalRange;
    }
  } else if (motion == "scroll-up") {
    float tick = millis() / 50.0;
    int totalRange = MATRIX_HEIGHT + h;
    int offset = ((int)(tick * speed * 0.4)) % totalRange;
    startY = y - offset;
    if (startY < -h) {
      startY += totalRange;
    }
  } else if (motion == "scroll-down") {
    float tick = millis() / 50.0;
    int totalRange = MATRIX_HEIGHT + h;
    int offset = ((int)(tick * speed * 0.4)) % totalRange;
    startY = y + offset;
    if (startY >= MATRIX_HEIGHT) {
      startY -= totalRange;
    }
  }

  if (motion == "rotate") {
    float angle = millis() * speed * 0.0004;
    float cosA = cos(angle);
    float sinA = sin(angle);
    float hW = (w - 1) / 2.0;
    float hH = (h - 1) / 2.0;

    for (int r = 0; r < h; r++) {
      for (int c = 0; c < w; c++) {
        uint32_t color = pgm_read_dword(&sticker[r * w + c]);
        if ((color >> 24) & 0xFF) {
          float rx = c - hW;
          float ry = r - hH;
          int nx = round(hW + rx * cosA - ry * sinA);
          int ny = round(hH + rx * sinA + ry * cosA);
          int px = startX + nx;
          int py = startY + ny;
          if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            uint8_t pr = (color >> 16) & 0xFF;
            uint8_t pg = (color >> 8) & 0xFF;
            uint8_t pb = color & 0xFF;
            setPixel(px, py, pr, pg, pb);
          }
        }
      }
    }
  } else {
    for (int sy = 0; sy < h; sy++) {
      for (int sx = 0; sx < w; sx++) {
        int px = startX + sx;
        int py = startY + sy;
        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
          uint32_t color = pgm_read_dword(&sticker[sy * w + sx]);
          if ((color >> 24) & 0xFF) {
            uint8_t pr = (color >> 16) & 0xFF;
            uint8_t pg = (color >> 8) & 0xFF;
            uint8_t pb = color & 0xFF;
            setPixel(px, py, pr, pg, pb);
          }
        }
      }
    }
  }
}

void drawBackgroundPixels(const uint32_t *bg, int x, int y, int w, int h,
                          int opacity) {
  for (int sy = 0; sy < h; sy++) {
    for (int sx = 0; sx < w; sx++) {
      int px = x + sx;
      int py = y + sy;
      if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
        uint32_t color = pgm_read_dword(&bg[sy * w + sx]);
        if ((color >> 24) & 0xFF) {
          uint8_t pr = (((color >> 16) & 0xFF) * opacity) / 100;
          uint8_t pg = (((color >> 8) & 0xFF) * opacity) / 100;
          uint8_t pb = ((color & 0xFF) * opacity) / 100;
          setPixel(px, py, pr, pg, pb);
        }
      }
    }
  }
}

void drawCustomAnimation(const uint32_t *const *frames, int framesCount, int x,
                         int y, int w, int h, int frameRate, int opacity) {
  int frameIdx = (millis() * frameRate / 1000) % framesCount;
  const uint32_t *frame = (const uint32_t *)pgm_read_ptr(&frames[frameIdx]);
  drawBackgroundPixels(frame, x, y, w, h, opacity);
}

void drawLocalTime(int x, int y, int w, int h, uint32_t color, int size,
                   String format, bool shadow, uint32_t shadowColor,
                   String effect, int speed, bool bold, bool rainbow) {
  String text = timeClient.getFormattedTime();
  if (format == "HH:MM") {
    text = text.substring(0, 5);
  }
  drawText(text, x, y, w, h, color, size, shadow, shadowColor, effect, speed,
           bold, rainbow);
}

void drawLocalDate(int x, int y, int w, int h, uint32_t color, int size,
                   String format, bool shadow, uint32_t shadowColor,
                   String effect, int speed, bool bold, bool rainbow) {
  time_t rawtime = timeClient.getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);
  char dateStr[20];
  if (format == "YYYY-MM-DD") {
    sprintf(dateStr, "%04d-%02d-%02d", ti->tm_year + 1900, ti->tm_mon + 1,
            ti->tm_mday);
  } else if (format == "MM/DD/YYYY") {
    sprintf(dateStr, "%02d/%02d/%04d", ti->tm_mon + 1, ti->tm_mday,
            ti->tm_year + 1900);
  } else if (format == "DD MMM") {
    const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    int mIdx = ti->tm_mon;
    if (mIdx < 0 || mIdx > 11)
      mIdx = 0;
    sprintf(dateStr, "%02d %s", ti->tm_mday, months[mIdx]);
  } else {
    sprintf(dateStr, "%02d/%02d/%04d", ti->tm_mday, ti->tm_mon + 1,
            ti->tm_year + 1900);
  }
  drawText(dateStr, x, y, w, h, color, size, shadow, shadowColor, effect, speed,
           bold, rainbow);
}

void drawTimer(int x, int y, int w, int h, uint32_t color, int size,
               bool shadow, uint32_t shadowColor, String effect, int speed,
               bool bold, bool rainbow) {
  int mins = countdownSeconds / 60;
  int secs = countdownSeconds % 60;
  char timeStr[10];
  sprintf(timeStr, "%02d:%02d", mins, secs);
  drawText(timeStr, x, y, w, h, color, size, shadow, shadowColor, effect, speed,
           bold, rainbow);
}

void drawCounter(long count, int x, int y, int w, int h, uint32_t color,
                 int size, bool shadow, uint32_t shadowColor, String effect,
                 int speed, bool bold, bool rainbow) {
  char countStr[32];
  sprintf(countStr, "%ld", count);
  drawText(countStr, x, y, w, h, color, size, shadow, shadowColor, effect,
           speed, bold, rainbow);
}

void drawWeatherSticker(int sx, int sy, String condition) {
  condition.toLowerCase();

  if (condition == "clear" || condition == "sunny") {
    float tick = millis() / 50.0;
    float cx = 8.0;
    float cy = 8.0;
    float pulse = sin(tick * 0.1) * 0.5;
    float r = 5.0 + pulse;
    for (int py = 0; py < 16; py++) {
      for (int px = 0; px < 16; px++) {
        float dx = px - cx;
        float dy = py - cy;
        float dist = sqrt(dx * dx + dy * dy);
        if (dist < r) {
          setPixel(sx + px, sy + py, 250, 204, 21); // yellow
        } else if (dist < r + 6.0) {
          float intensity = 1.0 - (dist - r) / 6.0;
          float finalOpacity = intensity * 0.8;
          uint8_t finalR = (uint8_t)(255 * finalOpacity);
          uint8_t finalG = (uint8_t)((100 + intensity * 100) * finalOpacity);
          setPixel(sx + px, sy + py, finalR, finalG, 0); // glow orange-yellow
        }
      }
    }
  } else if (condition == "clouds" || condition == "cloudy") {
    float tick = millis() / 50.0;
    int wave = (int)(floor(sin(tick * 0.14) * 2.0));
    uint32_t W1 = 0xffe2e8f0;
    uint32_t W2 = 0xff94a3b8;
    uint32_t W3 = 0xff64748b;

    struct CloudRow {
      int x1, x2;
      uint32_t col;
    };
    CloudRow rows[8] = {{5, 9, W1},  {3, 11, W1}, {1, 13, W1}, {0, 14, W1},
                        {0, 15, W2}, {0, 15, W2}, {1, 14, W3}, {3, 12, W3}};
    for (int r = 0; r < 8; r++) {
      for (int c = rows[r].x1; c <= rows[r].x2; c++) {
        int px = c;
        int py = 4 + r + wave;
        if (px >= 0 && px < 16 && py >= 0 && py < 16) {
          setPixel(sx + px, sy + py, rows[r].col);
        }
      }
    }
  } else if (condition == "rain" || condition == "rainy" ||
             condition == "drizzle") {
    float tick = millis() / 50.0;
    uint32_t W1 = 0xffcbd5e1;
    uint32_t W2 = 0xff94a3b8;
    uint32_t W3 = 0xff64748b;
    struct CloudRow {
      int x1, x2;
      uint32_t col;
    };
    CloudRow rows[7] = {{4, 8, W1},  {2, 10, W1}, {0, 13, W1}, {0, 14, W2},
                        {0, 14, W2}, {1, 13, W3}, {3, 11, W3}};
    for (int r = 0; r < 7; r++) {
      for (int c = rows[r].x1; c <= rows[r].x2; c++) {
        if (c >= 0 && c < 16 && r >= 0 && r < 16) {
          setPixel(sx + c, sy + r, rows[r].col);
        }
      }
    }
    // 5 animated rain streams
    int rainDrops[5][2] = {{1, 0}, {3, 3}, {5, 1}, {7, 4}, {9, 2}};
    for (int i = 0; i < 5; i++) {
      int dx = rainDrops[i][0];
      int ph = rainDrops[i][1];
      int dropY = 8 + (((int)(floor(tick * 0.6)) + ph * 2) % 8);
      if (dx >= 0 && dx < 16 && dropY >= 0 && dropY < 16) {
        setPixel(sx + dx, sy + dropY, 0xff38bdf8);
      }
      if (dx >= 0 && dx < 16 && (dropY + 1) >= 0 && (dropY + 1) < 16) {
        setPixel(sx + dx, sy + dropY + 1, 0xff7dd3fc);
      }
    }
  } else if (condition == "thunder" || condition == "thunderstorm" ||
             condition == "storm") {
    float tick = millis() / 50.0;
    uint32_t D1 = 0xff334155;
    uint32_t D2 = 0xff475569;
    uint32_t D3 = 0xff64748b;
    struct CloudRow {
      int x1, x2;
      uint32_t col;
    };
    CloudRow rows[7] = {{4, 8, D1},  {2, 10, D1}, {0, 13, D1}, {0, 14, D2},
                        {0, 14, D2}, {1, 13, D3}, {3, 11, D3}};
    for (int r = 0; r < 7; r++) {
      for (int c = rows[r].x1; c <= rows[r].x2; c++) {
        if (c >= 0 && c < 16 && r >= 0 && r < 16) {
          setPixel(sx + c, sy + r, rows[r].col);
        }
      }
    }
    // Large flashing lightning bolt
    int boltPhase = ((int)(floor(tick / 4.0))) % 5;
    uint32_t boltColor = 0xfffacc15;
    if (boltPhase == 3)
      boltColor = 0xfffde047;
    else if (boltPhase == 4)
      boltColor = 0xfffef08a;

    // Bolt shape: zig-zag down
    int boltCols[7][3] = {{5, 6, -1}, {4, 5, -1}, {5, 6, 7}, {6, 7, -1},
                          {5, 6, -1}, {6, 7, -1}, {7, 8, -1}};
    for (int r = 0; r < 7; r++) {
      for (int i = 0; i < 3; i++) {
        int c = boltCols[r][i];
        if (c != -1 && c >= 0 && c < 16 && (8 + r) >= 0 && (8 + r) < 16) {
          setPixel(sx + c, sy + 8 + r, boltColor);
        }
      }
    }
  } else if (condition == "snow" || condition == "snowy") {
    float tick = millis() / 50.0;
    uint32_t W1 = 0xffe2e8f0;
    uint32_t W2 = 0xff94a3b8;
    uint32_t W3 = 0xff64748b;
    struct CloudRow {
      int x1, x2;
      uint32_t col;
    };
    CloudRow rows[7] = {{4, 8, W1},  {2, 10, W1}, {0, 13, W1}, {0, 14, W2},
                        {0, 14, W2}, {1, 13, W3}, {3, 11, W3}};
    for (int r = 0; r < 7; r++) {
      for (int c = rows[r].x1; c <= rows[r].x2; c++) {
        if (c >= 0 && c < 16 && r >= 0 && r < 16) {
          setPixel(sx + c, sy + r, rows[r].col);
        }
      }
    }
    // 6 drifting snowflakes
    int flakes[6][2] = {{0, 0}, {2, 3}, {4, 1}, {6, 4}, {8, 2}, {10, 5}};
    for (int i = 0; i < 6; i++) {
      int dx = flakes[i][0];
      int ph = flakes[i][1];
      int fy = 8 + (((int)(floor(tick * 0.22)) + ph * 2) % 8);
      int fx = dx + (int)(floor(sin(tick * 0.06 + ph) * 1.5));
      if (fy >= 0 && fy < 16 && fx >= 0 && fx < 16) {
        setPixel(sx + fx, sy + fy, 0xffe0f2fe);
        if (ph % 2 == 0) {
          if (fx + 1 < 16)
            setPixel(sx + fx + 1, sy + fy, 0xffbae6fd);
          if (fx - 1 >= 0)
            setPixel(sx + fx - 1, sy + fy, 0xffbae6fd);
        }
      }
    }
  } else {
    float tick = millis() / 50.0;
    struct Streak {
      int yOff;
      uint32_t col;
    };
    Streak streaks[5] = {{2, 0xff94a3b8},
                         {5, 0xffcbd5e1},
                         {8, 0xff94a3b8},
                         {11, 0xffe2e8f0},
                         {14, 0xff94a3b8}};
    for (int i = 0; i < 5; i++) {
      int shift = ((int)(floor(tick * 0.35 + i * 3.3))) % 16;
      for (int c = 0; c < 12; c++) {
        int px = (c + shift) % 15;
        if (px < 16 && streaks[i].yOff >= 0 && streaks[i].yOff < 16) {
          setPixel(sx + px, sy + streaks[i].yOff, streaks[i].col);
        }
      }
    }
  }
}

void drawWeather(int x, int y, int w, int h, String param, int iconX, int iconY,
                 int tempX, int tempY, int humiX, int humiY, int briefX,
                 int briefY, String tempColorMode, uint32_t tempColor,
                 bool tempBold, int tempSize, bool tempShadow,
                 uint32_t tempShadowColor, String humiColorMode,
                 uint32_t humiColor, bool humiBold, int humiSize,
                 bool humiShadow, uint32_t humiShadowColor,
                 String briefColorMode, uint32_t briefColor, bool briefBold,
                 int briefSize, bool briefShadow, uint32_t briefShadowColor) {
  String currentCond = weatherDesc;
  currentCond.toLowerCase();

  uint32_t followColor = 0xff00ffff; // mist/wind: vibrant cyan
  if (currentCond == "clear" || currentCond == "sunny") {
    followColor = 0xffffd700; // vibrant golden yellow
  } else if (currentCond == "clouds" || currentCond == "cloudy") {
    followColor = 0xffa8c0d8; // vibrant light blue-grey
  } else if (currentCond == "rain" || currentCond == "rainy" ||
             currentCond == "drizzle") {
    followColor = 0xff00a2ff; // vibrant electric blue
  } else if (currentCond == "thunderstorm" || currentCond == "thunder" ||
             currentCond == "storm") {
    followColor = 0xffb833ff; // vibrant electric purple
  } else if (currentCond == "snow" || currentCond == "snowy") {
    followColor = 0xffffffff; // snow: pure bright white
  }

  uint32_t finalTempColor =
      (tempColorMode == "followBrief") ? followColor : tempColor;
  uint32_t finalHumiColor =
      (humiColorMode == "followBrief") ? followColor : humiColor;
  uint32_t finalBriefColor =
      (briefColorMode == "followBrief") ? followColor : briefColor;

  for (int py = 0; py < h; py++) {
    for (int px = 0; px < w; px++) {
      setPixel(x + px, y + py, 0, 0, 0);
    }
  }

  String iconCond = weatherDesc;
  if (iconCond == "" || iconCond == "Clear" || iconCond == "sunny") {
    iconCond = param;
  }
  drawWeatherSticker(x + iconX, y + iconY, iconCond);

  String tempStr = String(weatherTemp, 1) + "C";
  String humiStr = String(weatherHumidity) + "%";
  String briefStr = weatherDesc;
  if (briefStr == "") {
    briefStr = param;
  }

  drawText(tempStr, x + tempX, y + tempY, w - tempX, 8, finalTempColor,
           tempSize, tempShadow, tempShadowColor, "none", 4, tempBold);
  drawText(humiStr, x + humiX, y + humiY, w - humiX, 8, finalHumiColor,
           humiSize, humiShadow, humiShadowColor, "none", 4, humiBold);
  drawText(briefStr, x + briefX, y + briefY, w - briefX, 8, finalBriefColor,
           briefSize, briefShadow, briefShadowColor, "none", 4, briefBold);
}

String formatYouTubeCount(long count, String format) {
  if (format == "full") {
    return String(count);
  }
  if (count >= 1000000) {
    return String((float)count / 1000000.0, 2) + "M";
  } else if (count >= 1000) {
    return String((float)count / 1000.0, 2) + "K";
  }
  return String(count);
}

void drawYouTubeSub(int x, int y, int w, int h, int textX, int textY,
                    uint32_t color, int size, bool shadow, uint32_t shadowColor,
                    String effect, int speed, bool bold, String format,
                    bool rainbow) {
  // Clear bounding box
  for (int py = 0; py < h; py++) {
    for (int px = 0; px < w; px++) {
      setPixel(x + px, y + py, 0, 0, 0);
    }
  }

  // Format and draw count text
  String countText = formatYouTubeCount(ytSubscriberCount, format);
  drawText(countText, x + textX, y + textY, w - textX, h, color, size, shadow,
           shadowColor, effect, speed, bold, rainbow);
}

void drawClock(int x, int y, int w, int h, uint32_t color, int size,
               String timeOfDayOverride, bool shadow, uint32_t shadowColor,
               bool bold, int bgX, int bgY, int dateX, int dateY, int timeX,
               int timeY, String dateFormat, String timeFormat,
               uint32_t dateColor, uint32_t timeColor, bool dateBold,
               bool timeBold, int dateSize, int timeSize, bool dateShadow,
               bool timeShadow, uint32_t dateShadowColor,
               uint32_t timeShadowColor, String dateColorMode,
               String timeColorMode) {
  String phase = "morning";
  time_t rawtime = timeClient.getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);
  int hour = ti->tm_hour;

  if (timeOfDayOverride == "auto") {
    if (hour >= 6 && hour < 12)
      phase = "morning";
    else if (hour >= 12 && hour < 17)
      phase = "afternoon";
    else if (hour >= 17 && hour < 20)
      phase = "evening";
    else
      phase = "night";
  } else {
    phase = timeOfDayOverride;
  }

  String resolvedAnim = "sunrise";
  if (phase == "morning")
    resolvedAnim = "sunrise";
  else if (phase == "sunset" || phase == "evening")
    resolvedAnim = "sunset";
  else if (phase == "afternoon")
    resolvedAnim = "afternoon";
  else
    resolvedAnim = "night";

  uint32_t resolvedDateColor = dateColor;
  uint32_t resolvedTimeColor = timeColor;

  if (dateColorMode == "followTime") {
    if (phase == "morning")
      resolvedDateColor = 0xffffaa00; // warm golden yellow
    else if (phase == "afternoon")
      resolvedDateColor = 0xffffffff; // bright afternoon white
    else if (phase == "evening")
      resolvedDateColor = 0xffff4400; // vibrant sunset red-orange
    else
      resolvedDateColor = 0xff0066ff; // vibrant electric night blue
  }

  if (timeColorMode == "followTime") {
    if (phase == "morning")
      resolvedTimeColor = 0xffffaa00; // warm golden yellow
    else if (phase == "afternoon")
      resolvedTimeColor = 0xffffffff; // bright afternoon white
    else if (phase == "evening")
      resolvedTimeColor = 0xffff4400; // vibrant sunset red-orange
    else
      resolvedTimeColor = 0xff0066ff; // vibrant electric night blue
  }

  // Clear background to black first (optional, but keeps it clean when bg is
  // offset)
  for (int py = 0; py < h; py++) {
    for (int px = 0; px < w; px++) {
      setPixel(x + px, y + py, 0, 0, 0);
    }
  }

  drawPrebuiltAnimation(resolvedAnim, x, y, 100, bgX, bgY);

  // Format Date
  char dateStr[24];
  if (dateFormat == "YYYY-MM-DD") {
    sprintf(dateStr, "%04d-%02d-%02d", ti->tm_year + 1900, ti->tm_mon + 1,
            ti->tm_mday);
  } else if (dateFormat == "MM/DD/YYYY") {
    sprintf(dateStr, "%02d/%02d/%04d", ti->tm_mon + 1, ti->tm_mday,
            ti->tm_year + 1900);
  } else if (dateFormat == "DD/MM/YYYY") {
    sprintf(dateStr, "%02d/%02d/%04d", ti->tm_mday, ti->tm_mon + 1,
            ti->tm_year + 1900);
  } else {
    // Default to "DD MMM"
    const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    int mIdx = ti->tm_mon;
    if (mIdx < 0 || mIdx > 11)
      mIdx = 0;
    sprintf(dateStr, "%02d %s", ti->tm_mday, months[mIdx]);
  }

  // Format Time
  char timeStr[24];
  if (timeFormat == "HH:MM:SS") {
    sprintf(timeStr, "%02d:%02d:%02d", ti->tm_hour, ti->tm_min, ti->tm_sec);
  } else if (timeFormat == "HH:MM AM/PM") {
    int hour12 = ti->tm_hour % 12;
    if (hour12 == 0)
      hour12 = 12;
    const char *ampm = ti->tm_hour >= 12 ? "PM" : "AM";
    sprintf(timeStr, "%02d:%02d %s", hour12, ti->tm_min, ampm);
  } else {
    // Default to "HH:MM"
    sprintf(timeStr, "%02d:%02d", ti->tm_hour, ti->tm_min);
  }

  drawText(dateStr, x + dateX, y + dateY, w - dateX, 8, resolvedDateColor,
           dateSize, dateShadow, dateShadowColor, "none", 4, dateBold);
  drawText(timeStr, x + timeX, y + timeY, w - timeX, 8, resolvedTimeColor,
           timeSize, timeShadow, timeShadowColor, "none", 4, timeBold);
}

void drawPrebuiltAnimation(String animId, int x, int y, int opacity, int bgX,
                           int bgY) {
  // ==================== 1. Cosmic & Space ====================
  if (animId == "supernova") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;
      int cycle = ((int)tick) % 120;

      if (cycle < 40) {
        float starSize = 1.5 + sin(cycle * 0.2) * 1.0;
        for (int px = 0; px < MATRIX_WIDTH; px++) {
          for (int py = 0; py < MATRIX_HEIGHT; py++) {
            float dx = px - cx;
            float dy = py - cy;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < starSize) {
              setPixel(x + px, y + py, 255 * opacity / 100, 255 * opacity / 100,
                       255 * opacity / 100);
            } else if (dist < starSize * 2.0) {
              float distOpacity = 1.0 - (dist - starSize) / starSize;
              float finalOpacity = distOpacity * 0.6 * opacity / 100.0;
              uint8_t r = (uint8_t)(249 * finalOpacity);
              uint8_t g = (uint8_t)(115 * finalOpacity);
              uint8_t b = (uint8_t)(22 * finalOpacity);
              setPixel(x + px, y + py, r, g, b);
            }
          }
        }
      } else if (cycle < 90) {
        int expTime = cycle - 40;
        float radius = expTime * 0.8;
        float fade = 1.0 - expTime / 50.0;
        if (fade < 0)
          fade = 0;
        float finalOpacity = fade * opacity / 100.0;

        for (float theta = 0; theta < 6.28318; theta += 0.1) {
          int px = round(cx + cos(theta) * radius);
          int py = round(cy + sin(theta) * radius);
          if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            int hue =
                ((int)(expTime * 4.0 + theta * 10.0 * 180.0 / 3.14159)) % 360;
            if (hue < 0)
              hue += 360;
            uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 255);
            uint8_t r = (((color >> 16) & 0xFF) * finalOpacity);
            uint8_t g = (((color >> 8) & 0xFF) * finalOpacity);
            uint8_t b = ((color & 0xFF) * finalOpacity);
            setPixel(x + px, y + py, r, g, b);
          }
        }

        if (radius > 5.0) {
          float rad2 = radius - 5.0;
          for (float theta = 0; theta < 6.28318; theta += 0.2) {
            int px = round(cx + cos(theta) * rad2);
            int py = round(cy + sin(theta) * rad2);
            if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
              uint8_t r = (uint8_t)(239 * finalOpacity);
              uint8_t g = (uint8_t)(68 * finalOpacity);
              uint8_t b = (uint8_t)(68 * finalOpacity);
              setPixel(x + px, y + py, r, g, b);
            }
          }
        }
      } else {
        int impTime = cycle - 90;
        float strength = (30.0 - impTime) / 30.0;
        if (strength < 0)
          strength = 0;
        float finalOpacity = strength * opacity / 100.0;

        for (int px = 0; px < MATRIX_WIDTH; px++) {
          for (int py = 0; py < MATRIX_HEIGHT; py++) {
            float dx = px - cx;
            float dy = py - cy;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist > 1.0) {
              float ringVal = sin(dist * 0.8 - tick * 0.2);
              if (abs(ringVal) > 0.85) {
                float alpha = strength * 0.5 * opacity / 100.0;
                uint8_t r = (uint8_t)(99 * alpha);
                uint8_t g = (uint8_t)(102 * alpha);
                uint8_t b = (uint8_t)(241 * alpha);
                setPixel(x + px, y + py, r, g, b);
              }
            }
          }
        }
      }
    }
  }

  else if (animId == "pulsar") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;
      float angle = tick * 0.05;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = px - cx;
          float dy = py - cy;
          float dist = sqrt(dx * dx + dy * dy);
          float pAngle = atan2(dy, dx);

          float jetIntensity = 0.0;
          float diff1 = abs(fmod(pAngle - angle, 3.14159265f));
          float diff2 = abs(fmod(pAngle - angle - 3.14159265f, 3.14159265f));
          float minDiff = diff1 < diff2 ? diff1 : diff2;

          if (minDiff < 0.12) {
            jetIntensity = (1.0 - minDiff / 0.12) * (1.0 - dist / 50.0);
          }

          float loopRad = sin(dist * 0.6 - tick * 0.15);
          float loopGlow = 0.0;
          if (abs(loopRad) > 0.9) {
            loopGlow = (1.0 - dist / 30.0) * 0.4;
          }

          if (jetIntensity > 0.05) {
            int jetVal = (int)(jetIntensity * 255.0);
            if (jetVal > 255)
              jetVal = 255;
            uint8_t r = (uint8_t)(jetVal * opacity / 100);
            uint8_t g = (uint8_t)((int)(jetVal * 0.9) * opacity / 100);
            uint8_t b = (uint8_t)(255 * opacity / 100);
            setPixel(x + px, y + py, r, g, b);
          } else if (loopGlow > 0.05) {
            uint8_t r = (uint8_t)(59 * loopGlow * opacity / 100);
            uint8_t g = (uint8_t)(130 * loopGlow * opacity / 100);
            uint8_t b = (uint8_t)(246 * loopGlow * opacity / 100);
            setPixel(x + px, y + py, r, g, b);
          }

          if (dist < 2.0) {
            setPixel(x + px, y + py, 255 * opacity / 100, 255 * opacity / 100,
                     255 * opacity / 100);
          }
        }
      }
    }
  }

  // ==================== 2. Nature, Weather & Landscapes ====================
  else if (animId == "sunrise") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cx = bgX;
      float cy = (MATRIX_HEIGHT + 5) - fmod(tick * 0.15, MATRIX_HEIGHT + 10) +
                 (bgY - 8);
      float r = 5.0;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = px - cx;
          float dy = py - cy;
          float dist = sqrt(dx * dx + dy * dy);

          if (dist < r) {
            uint8_t finalR = (250 * opacity) / 100;
            uint8_t finalG = (204 * opacity) / 100;
            uint8_t finalB = (21 * opacity) / 100;
            setPixel(x + px, y + py, finalR, finalG, finalB);
          } else if (dist < r + 6.0) {
            float intensity = 1.0 - (dist - r) / 6.0;
            float finalOpacity = intensity * 0.8 * opacity / 100.0;
            uint8_t finalR = (uint8_t)(255 * finalOpacity);
            uint8_t finalG = (uint8_t)((100 + intensity * 100) * finalOpacity);
            setPixel(x + px, y + py, finalR, finalG, 0);
          }
        }
      }
    }
  }

  else if (animId == "sunset") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cx = bgX;
      float cy = fmod(tick * 0.15, MATRIX_HEIGHT + 10) - 5.0 + (bgY - 8);
      float r = 5.0;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = px - cx;
          float dy = py - cy;
          float dist = sqrt(dx * dx + dy * dy);

          if (dist < r) {
            uint8_t finalR = (234 * opacity) / 100;
            uint8_t finalG = (88 * opacity) / 100;
            uint8_t finalB = (12 * opacity) / 100;
            setPixel(x + px, y + py, finalR, finalG, finalB);
          } else if (dist < r + 7.0) {
            float intensity = 1.0 - (dist - r) / 7.0;
            float finalOpacity = intensity * 0.75 * opacity / 100.0;
            uint8_t finalR = (uint8_t)((180 + intensity * 75) * finalOpacity);
            uint8_t finalB = (uint8_t)((50 + intensity * 100) * finalOpacity);
            setPixel(x + px, y + py, finalR, 0, finalB);
          }
        }
      }
    }
  }

  else if (animId == "afternoon") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cx = bgX;
      float cy = bgY;
      float pulse = sin(tick * 0.1) * 0.5;
      float r = 5.0 + pulse;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = px - cx;
          float dy = py - cy;
          float dist = sqrt(dx * dx + dy * dy);

          if (dist < r) {
            uint8_t finalR = (250 * opacity) / 100;
            uint8_t finalG = (204 * opacity) / 100;
            uint8_t finalB = (21 * opacity) / 100;
            setPixel(x + px, y + py, finalR, finalG, finalB);
          } else if (dist < r + 6.0) {
            float intensity = 1.0 - (dist - r) / 6.0;
            float finalOpacity = intensity * 0.8 * opacity / 100.0;
            uint8_t finalR = (uint8_t)(255 * finalOpacity);
            uint8_t finalG = (uint8_t)((100 + intensity * 100) * finalOpacity);
            setPixel(x + px, y + py, finalR, finalG, 0);
          }
        }
      }
    }
  }

  else if (animId == "night") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cx = bgX;
      float cy = (MATRIX_HEIGHT + 5) - fmod(tick * 0.15, MATRIX_HEIGHT + 10) +
                 (bgY - 8);
      float r = 5.0;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = px - cx;
          float dy = py - cy;
          float dist = sqrt(dx * dx + dy * dy);

          if (dist < r) {
            uint8_t finalVal = (255 * opacity) / 100;
            setPixel(x + px, y + py, finalVal, finalVal, finalVal);
          } else if (dist < r + 6.0) {
            float intensity = 1.0 - (dist - r) / 6.0;
            float finalOpacity = intensity * 0.8 * opacity / 100.0;
            uint8_t finalR = (uint8_t)((50 + intensity * 100) * finalOpacity);
            uint8_t finalG = (uint8_t)((100 + intensity * 100) * finalOpacity);
            uint8_t finalB = (uint8_t)(255 * finalOpacity);
            setPixel(x + px, y + py, finalR, finalG, finalB);
          }
        }
      }
    }
  }

  else if (animId == "beach") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      int sandY = MATRIX_HEIGHT - 2;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        float waveY = (MATRIX_HEIGHT - 3) + sin(px * 0.12 + tick * 0.08) * 2.0 +
                      cos(tick * 0.04) * 1.5;

        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          if (py >= waveY) {
            if (abs(py - waveY) < 1.0) {
              setPixel(x + px, y + py, 255 * opacity / 100, 255 * opacity / 100,
                       255 * opacity / 100);
            } else {
              setPixel(x + px, y + py, 2 * opacity / 100, 132 * opacity / 100,
                       199 * opacity / 100);
            }
          } else if (py >= sandY) {
            setPixel(x + px, y + py, 217 * opacity / 100, 119 * opacity / 100,
                     6 * opacity / 100);
          }
        }
      }
    }
  }

  else if (animId == "tsunami") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float waveX = fmod(tick * 0.6, MATRIX_WIDTH + 30) - 15;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        float dx = px - waveX;
        float crest = 0;
        float exponent = -(dx * dx) / 64.0;
        if (exponent > -20.0) {
          crest = 13.0 * exp(exponent);
        }
        float waveY = MATRIX_HEIGHT - crest;

        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          if (py >= waveY) {
            if (py < waveY + 1.2) {
              setPixel(x + px, y + py, 255 * opacity / 100, 255 * opacity / 100,
                       255 * opacity / 100);
            } else {
              float depth = py - waveY;
              int greenVal = 50 + (int)(depth * 15);
              if (greenVal > 150)
                greenVal = 150;
              setPixel(x + px, y + py, 15 * opacity / 100,
                       greenVal * opacity / 100, 140 * opacity / 100);
            }
          }
        }
      }
    }
  }

  else if (animId == "drippingrain") {
    {
      struct RainDrop {
        float x;
        float y;
        float speed;
      };
      static RainDrop drops[20];
      static bool rainInit = false;
      if (!rainInit) {
        for (int i = 0; i < 20; i++) {
          drops[i].x = random(0, MATRIX_WIDTH);
          drops[i].y = -1.0 * random(0, MATRIX_HEIGHT);
          drops[i].speed = 0.7 + (random(0, 700) / 1000.0);
        }
        rainInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int i = 0; i < 20; i++) {
        drops[i].y += drops[i].speed;
        if (drops[i].y >= MATRIX_HEIGHT) {
          drops[i].y = -1.0 * random(0, 6);
          drops[i].x = random(0, MATRIX_WIDTH);
          drops[i].speed = 0.7 + (random(0, 700) / 1000.0);
        }

        int dx = (int)drops[i].x;
        int dy = (int)drops[i].y;

        if (dy >= 0 && dy < MATRIX_HEIGHT) {
          setPixel(x + dx, y + dy, 56 * opacity / 100, 189 * opacity / 100,
                   248 * opacity / 100);
          if (dy > 0) {
            uint8_t trailR = (uint8_t)(56 * 0.4 * opacity / 100);
            uint8_t trailG = (uint8_t)(189 * 0.4 * opacity / 100);
            uint8_t trailB = (uint8_t)(248 * 0.4 * opacity / 100);
            setPixel(x + dx, y + dy - 1, trailR, trailG, trailB);
          }

          if (dy >= MATRIX_HEIGHT - 1) {
            uint8_t splashR = (uint8_t)(125 * 0.6 * opacity / 100);
            uint8_t splashG = (uint8_t)(211 * 0.6 * opacity / 100);
            uint8_t splashB = (uint8_t)(252 * 0.6 * opacity / 100);
            if (dx > 0) {
              setPixel(x + dx - 1, y + MATRIX_HEIGHT - 1, splashR, splashG,
                       splashB);
            }
            if (dx < MATRIX_WIDTH - 1) {
              setPixel(x + dx + 1, y + MATRIX_HEIGHT - 1, splashR, splashG,
                       splashB);
            }
          }
        }
      }
    }
  }

  else if (animId == "tornado") {
    {
      struct TornadoDebris {
        float angle;
        float r;
      };
      static TornadoDebris debris[10];
      static bool debrisInit = false;
      if (!debrisInit) {
        for (int i = 0; i < 10; i++) {
          debris[i].angle = (random(0, 6283) / 1000.0);
          debris[i].r = 2.0 + (random(0, 700) / 100.0);
        }
        debrisInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float tick = millis() / 50.0;
      float centerX = MATRIX_WIDTH / 2.0 + sin(tick * 0.04) * 15.0;

      for (int py = 0; py < MATRIX_HEIGHT; py++) {
        float funnelX = centerX + sin(tick * 0.1 + py * 0.2) * 2.0;
        float radius = 1.0 + (1.0 - (float)py / MATRIX_HEIGHT) * 4.5;

        for (int a = 0; a < 3; a++) {
          float angle = tick * 0.2 + py * 0.35 + (a * 6.28318) / 3.0;
          int px = round(funnelX + cos(angle) * radius);
          float depth = sin(angle);

          if (depth > -0.3) {
            uint8_t finalR = 107;
            uint8_t finalG = 114;
            uint8_t finalB = 128;
            if (depth > 0.4) {
              finalR = 255;
              finalG = 255;
              finalB = 255;
            }
            setPixel(x + px, y + py, finalR * opacity / 100,
                     finalG * opacity / 100, finalB * opacity / 100);
          }
        }
      }

      for (int i = 0; i < 10; i++) {
        debris[i].angle += 0.18;
        debris[i].r += 0.08;
        if (debris[i].r > 10.0) {
          debris[i].r = 2.0;
          debris[i].angle = (random(0, 6283) / 1000.0);
        }
        int px = round(centerX + cos(debris[i].angle) * debris[i].r);
        int py = round(MATRIX_HEIGHT - 1 - (random(0, 200) / 100.0));
        setPixel(x + px, y + py, 217 * opacity / 100, 119 * opacity / 100,
                 6 * opacity / 100);
      }
    }
  }

  // ==================== 3. Fluid & Plasma Waves ====================
  else if (animId == "plasma") {
    {
      float tick = millis() / 50.0;
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float v1 = sin(px * 0.12 + tick * 0.08);
          float v2 = sin(py * 0.18 + tick * 0.06);
          float v3 = sin((px + py) * 0.1 + tick * 0.05);
          float r =
              sqrt((px - MATRIX_WIDTH / 2.0) * (px - MATRIX_WIDTH / 2.0) +
                   (py - MATRIX_HEIGHT / 2.0) * (py - MATRIX_HEIGHT / 2.0));
          float v4 = sin(r * 0.12 - tick * 0.04);

          float waveSum = (v1 + v2 + v3 + v4) / 4.0;
          int hue =
              (int)((waveSum + 1.0) * 180.0 * 65535.0 / 360.0 + tick * 300.0) &
              65535;
          uint32_t color = matrix.ColorHSV(hue, 255, 115);

          uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
          uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
          uint8_t blu = ((color & 0xFF) * opacity) / 100;
          setPixel(x + px, y + py, red, grn, blu);
        }
      }
    }
  }

  else if (animId == "aurora") {
    {
      struct AStar {
        float x, y;
        float brightness;
      };
      static AStar stars[12];
      static bool auroraInit = false;
      if (!auroraInit) {
        for (int i = 0; i < 12; i++) {
          stars[i].x = random(0, MATRIX_WIDTH);
          stars[i].y = random(0, MATRIX_HEIGHT);
          stars[i].brightness = random(0, 100) / 100.0;
        }
        auroraInit = true;
      }

      float tick = millis() / 50.0;
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int i = 0; i < 12; i++) {
        float alpha =
            0.2 + 0.8 * abs(sin(tick * 0.03 + stars[i].brightness * 10));
        uint8_t c = (uint8_t)(255 * alpha * opacity / 100);
        setPixel(x + (int)stars[i].x, y + (int)stars[i].y, c, c, c);
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        float waveVal1 = sin(px * 0.08 + tick * 0.035) * 3.5;
        float waveVal2 = cos(px * 0.14 - tick * 0.015) * 1.5;
        float waveY = (MATRIX_HEIGHT / 2.0) + waveVal1 + waveVal2;

        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dist = abs(py - waveY);
          if (dist < 6.0) {
            float intensity = 1.0 - (dist / 6.0);
            int hueBase = (py < waveY) ? (int)(120.0 + dist * 15.0)
                                       : (int)(270.0 - dist * 10.0);
            int hue = (int)(hueBase * 65535.0 / 360.0) & 65535;
            uint32_t color =
                matrix.ColorHSV(hue, 255, (uint8_t)(255 * intensity * 0.8));

            uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
            uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
            uint8_t blu = ((color & 0xFF) * opacity) / 100;

            uint32_t current = matrix.getPixelColor(py * MATRIX_WIDTH + px);
            uint8_t bgR = (current >> 16) & 0xFF;
            uint8_t bgG = (current >> 8) & 0xFF;
            uint8_t bgB = current & 0xFF;

            uint16_t finalR = bgR + red;
            if (finalR > 255)
              finalR = 255;
            uint16_t finalG = bgG + grn;
            if (finalG > 255)
              finalG = 255;
            uint16_t finalB = bgB + blu;
            if (finalB > 255)
              finalB = 255;

            setPixel(x + px, y + py, (uint8_t)finalR, (uint8_t)finalG,
                     (uint8_t)finalB);
          }
        }
      }
    }
  }

  else if (animId == "rainbowwaves") {
    {
      float tick = millis() / 50.0;
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float wave =
              sin(px * 0.15 + tick * 0.08) * cos(py * 0.25 - tick * 0.05);
          int hue = ((int)(wave * 180.0 + tick * 2.0)) % 360;
          if (hue < 0)
            hue += 360;
          uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 255);
          uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
          uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
          uint8_t blu = ((color & 0xFF) * opacity) / 100;
          setPixel(x + px, y + py, red, grn, blu);
        }
      }
    }
  }

  else if (animId == "wavefront") {
    {
      float tick = millis() / 50.0;
      float timeVal = tick * 0.04;
      float x1 = MATRIX_WIDTH / 2.0 + sin(timeVal) * 16.0;
      float y1 = MATRIX_HEIGHT / 2.0 + cos(timeVal * 0.8) * 5.0;
      float x2 = MATRIX_WIDTH / 2.0 + cos(timeVal * 1.2) * 20.0;
      float y2 = MATRIX_HEIGHT / 2.0 + sin(timeVal * 0.5) * 4.0;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float d1 = sqrt((px - x1) * (px - x1) + (py - y1) * (py - y1));
          float d2 = sqrt((px - x2) * (px - x2) + (py - y2) * (py - y2));
          float val1 = sin(d1 * 0.8 - tick * 0.12);
          float val2 = sin(d2 * 0.6 - tick * 0.08);
          float intensity = (val1 + val2) / 2.0;

          if (intensity > 0.4) {
            int hue = ((int)((px + py) * 2.0 + tick)) % 360;
            if (hue < 0)
              hue += 360;
            uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255,
                                             (uint8_t)(intensity * 255));
            uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
            uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
            uint8_t blu = ((color & 0xFF) * opacity) / 100;
            setPixel(x + px, y + py, red, grn, blu);
          } else {
            setPixel(x + px, y + py, 0, 0, 0);
          }
        }
      }
    }
  }

  else if (animId == "watercells") {
    {
      struct WCell {
        float x, y, vx, vy;
      };
      static WCell cells[3] = {{15.0, 5.0, 0.15, 0.1},
                               {45.0, 10.0, -0.1, 0.12},
                               {65.0, 6.0, 0.12, -0.08}};

      float tick = millis() / 50.0;

      for (int i = 0; i < 3; i++) {
        cells[i].x += cells[i].vx;
        cells[i].y += cells[i].vy;
        if (cells[i].x < 5.0 || cells[i].x >= MATRIX_WIDTH - 5.0)
          cells[i].vx *= -1;
        if (cells[i].y < 2.0 || cells[i].y >= MATRIX_HEIGHT - 2.0)
          cells[i].vy *= -1;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float minDist = 9999.0;
          for (int i = 0; i < 3; i++) {
            float dist = (px - cells[i].x) * (px - cells[i].x) +
                         (py - cells[i].y) * (py - cells[i].y);
            if (dist < minDist) {
              minDist = dist;
            }
          }

          float cellVal = sin(sqrt(minDist) * 1.5 - tick * 0.15);
          if (cellVal > 0.6) {
            uint8_t r = (6 * opacity) / 100;
            uint8_t g = (182 * opacity) / 100;
            uint8_t b = (212 * opacity) / 100;
            setPixel(x + px, y + py, r, g, b);
          } else {
            setPixel(x + px, y + py, 0, 0, 0);
          }
        }
      }
    }
  }

  else if (animId == "waterfall") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float flow =
              sin(py * 0.35 - tick * 0.18) * cos(px * 0.12 + tick * 0.04);
          if (flow > 0.65) {
            float flowOpacity = (flow - 0.65) / 0.35;
            float finalOpacity = flowOpacity * 0.5 * opacity / 100.0;
            uint8_t r = (uint8_t)(2 * finalOpacity);
            uint8_t g = (uint8_t)(132 * finalOpacity);
            uint8_t b = (uint8_t)(199 * finalOpacity);
            setPixel(x + px, y + py, r, g, b);
          }
        }
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        float splashHeight = abs(sin(px * 0.3 + tick * 0.25)) * 3.5;
        for (int sy = 0; sy < (int)splashHeight; sy++) {
          int py = MATRIX_HEIGHT - 1 - sy;
          float splashOpacity = (1.0 - sy / 4.0) * opacity / 100.0;
          if (splashOpacity < 0)
            splashOpacity = 0;
          uint8_t r = (uint8_t)(224 * splashOpacity);
          uint8_t g = (uint8_t)(242 * splashOpacity);
          uint8_t b = (uint8_t)(254 * splashOpacity);
          setPixel(x + px, y + py, r, g, b);
        }
      }
    }
  }

  // ==================== 4. Fractals & Math Curves ====================
  else if (animId == "attractor3d") {
    {
      struct AttractorPt {
        float x, y, z;
      };
      struct AttractorHistory {
        int x, y;
      };
      static AttractorPt apt = {0.1, 0.1, 0.1};
      static AttractorHistory ahistory[30];
      static int ahistoryLen = 0;
      static bool ainitialized = false;
      if (!ainitialized) {
        for (int i = 0; i < 30; i++)
          ahistory[i] = {0, 0};
        ainitialized = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float dt = 0.06;
      float a = 0.2;
      float b = 0.2;
      float c = 5.7;

      float dx = (-apt.y - apt.z) * dt;
      float dy = (apt.x + a * apt.y) * dt;
      float dz = (b + apt.z * (apt.x - c)) * dt;

      apt.x += dx;
      apt.y += dy;
      apt.z += dz;

      int projX = round(MATRIX_WIDTH / 2.0 + apt.x * 2.2);
      int projY = round(MATRIX_HEIGHT / 2.0 + apt.y * 0.4);

      for (int i = 29; i > 0; i--) {
        ahistory[i] = ahistory[i - 1];
      }
      ahistory[0] = {projX, projY};
      if (ahistoryLen < 30)
        ahistoryLen++;

      if (abs(apt.x) > 40.0 || abs(apt.y) > 40.0) {
        apt = {0.1, 0.1, 0.1};
        ahistoryLen = 0;
      }

      float tick = millis() / 50.0;
      for (int i = 0; i < ahistoryLen; i++) {
        float hOpacity = (1.0 - i / 30.0) * opacity / 100.0;
        if (hOpacity < 0)
          hOpacity = 0;
        int hue = ((int)(tick + i * 5.0)) % 360;
        if (hue < 0)
          hue += 360;
        uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 140);
        uint8_t red = (((color >> 16) & 0xFF) * hOpacity);
        uint8_t grn = (((color >> 8) & 0xFF) * hOpacity);
        uint8_t blu = ((color & 0xFF) * hOpacity);
        setPixel(x + ahistory[i].x, y + ahistory[i].y, red, grn, blu);
      }
    }
  }

  else if (animId == "lissajous3d") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;
      float thetaY = tick * 0.02;
      float thetaZ = tick * 0.03;

      float step = 0.08;
      for (int t = 0; t < 80; t++) {
        float x3d = sin(2.0 * t * step) * 26.0;
        float y3d = cos(3.0 * t * step) * 6.0;
        float z3d = sin(5.0 * t * step) * 10.0;

        float cosY = cos(thetaY);
        float sinY = sin(thetaY);
        float rx = x3d * cosY + z3d * sinY;
        float rz = -x3d * sinY + z3d * cosY;

        float cosZ = cos(thetaZ);
        float sinZ = sin(thetaZ);
        float ry = y3d * cosZ - rx * sinZ;
        rx = y3d * sinZ + rx * cosZ;

        int px = round(cx + rx);
        int py = round(cy + ry);

        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
          float intensity = 0.4 + ((rz + 10.0) / 20.0) * 0.6;
          float finalOpacity = intensity * opacity / 100.0;
          int hue = ((int)(t * 4.0 + tick)) % 360;
          if (hue < 0)
            hue += 360;
          uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 140);
          uint8_t red = (((color >> 16) & 0xFF) * finalOpacity);
          uint8_t grn = (((color >> 8) & 0xFF) * finalOpacity);
          uint8_t blu = ((color & 0xFF) * finalOpacity);
          setPixel(x + px, y + py, red, grn, blu);
        }
      }
    }
  }

  else if (animId == "kaleidoscope") {
    {
      float tick = millis() / 50.0;
      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;
      float timeVal = tick * 0.05;
      float targetX = cx + sin(timeVal) * 12.0;
      float targetY = cy + cos(timeVal * 0.7) * 4.0;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = abs(px - cx);
          float dy = abs(py - cy);
          if (dx < dy) {
            float tmp = dx;
            dx = dy;
            dy = tmp;
          }
          float tx = abs(targetX - cx);
          float ty = abs(targetY - cy);
          float dist = sqrt((dx - tx) * (dx - tx) + (dy - ty) * (dy - ty));
          float val = sin(dist * 0.6 - tick * 0.1);
          if (val > 0.5) {
            int hue = ((int)(dist * 10.0 + tick * 2.0)) % 360;
            if (hue < 0)
              hue += 360;
            uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 255);
            uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
            uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
            uint8_t blu = ((color & 0xFF) * opacity) / 100;
            setPixel(x + px, y + py, red, grn, blu);
          } else {
            setPixel(x + px, y + py, 0, 0, 0);
          }
        }
      }
    }
  }

  else if (animId == "vortex") {
    {
      float tick = millis() / 50.0;
      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = px - cx;
          float dy = py - cy;
          float r = sqrt(dx * dx + dy * dy);
          float angle = atan2(dy, dx);
          float vortexVal = sin(angle * 4.0 - r * 0.6 + tick * 0.15);
          if (vortexVal > 0.25) {
            float intensity = 1.0 - (r / 30.0);
            if (intensity < 0)
              intensity = 0;
            int hue = ((int)(r * 12.0 - tick * 1.5)) % 360;
            if (hue < 0)
              hue += 360;
            uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 255);
            uint8_t red = (((color >> 16) & 0xFF) * opacity * intensity) / 100;
            uint8_t grn = (((color >> 8) & 0xFF) * opacity * intensity) / 100;
            uint8_t blu = ((color & 0xFF) * opacity * intensity) / 100;
            setPixel(x + px, y + py, red, grn, blu);
          } else {
            setPixel(x + px, y + py, 0, 0, 0);
          }
        }
      }
    }
  }

  else if (animId == "spiral") {
    {
      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;
      float numArms = 3.0;
      float tick = millis() / 50.0;

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = px - cx;
          float dy = py - cy;
          float r = sqrt(dx * dx + dy * dy);
          float angle = atan2(dy, dx);

          float spiralVal = sin(numArms * angle - r * 0.35 + tick * 0.12);

          if (spiralVal > 0) {
            float angleDeg = (angle + 3.14159) * (180.0 / 3.14159);
            int hue = (int)(angleDeg * 65535.0 / 360.0 + tick * 600.0) & 65535;
            uint32_t color =
                matrix.ColorHSV(hue, 255, (uint8_t)(50 + spiralVal * 205));
            uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
            uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
            uint8_t blu = ((color & 0xFF) * opacity) / 100;
            setPixel(x + px, y + py, red, grn, blu);
          } else {
            setPixel(x + px, y + py, 0, 0, 0);
          }
        }
      }
    }
  }

  // ==================== 5. Particles & Physics ====================
  else if (animId == "particles") {
    {
      struct GParticle {
        float x, y, vx, vy;
        uint16_t hue;
      };
      static GParticle gparticles[30];
      static bool gparticlesInit = false;
      if (!gparticlesInit) {
        for (int i = 0; i < 30; i++) {
          gparticles[i].x = random(0, MATRIX_WIDTH);
          gparticles[i].y = random(0, MATRIX_HEIGHT);
          gparticles[i].vx = (random(0, 1000) / 1000.0 - 0.5) * 0.8;
          gparticles[i].vy = (random(0, 1000) / 1000.0 - 0.5) * 0.8;
          gparticles[i].hue = (random(0, 360) * 65535 / 360) & 65535;
        }
        gparticlesInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float tick = millis() / 50.0;
      float timeVal = tick * 0.04;
      float gx = MATRIX_WIDTH / 2.0 + sin(timeVal) * 20.0;
      float gy = MATRIX_HEIGHT / 2.0 + cos(timeVal * 1.5) * 5.0;

      setPixel(x + (int)round(gx), y + (int)round(gy), 255 * opacity / 100,
               255 * opacity / 100, 255 * opacity / 100);

      for (int i = 0; i < 30; i++) {
        float dx = gx - gparticles[i].x;
        float dy = gy - gparticles[i].y;
        float distSq = dx * dx + dy * dy;
        float dist = sqrt(distSq);

        if (dist > 1.0) {
          gparticles[i].vx += (dx / dist) * (0.85 / distSq);
          gparticles[i].vy += (dy / dist) * (0.85 / distSq);
        }

        gparticles[i].x += gparticles[i].vx;
        gparticles[i].y += gparticles[i].vy;

        gparticles[i].vx *= 0.98;
        gparticles[i].vy *= 0.98;

        if (gparticles[i].x < 0) {
          gparticles[i].x = 0;
          gparticles[i].vx *= -1;
        }
        if (gparticles[i].x >= MATRIX_WIDTH) {
          gparticles[i].x = MATRIX_WIDTH - 1;
          gparticles[i].vx *= -1;
        }
        if (gparticles[i].y < 0) {
          gparticles[i].y = 0;
          gparticles[i].vy *= -1;
        }
        if (gparticles[i].y >= MATRIX_HEIGHT) {
          gparticles[i].y = MATRIX_HEIGHT - 1;
          gparticles[i].vy *= -1;
        }

        uint32_t color = matrix.ColorHSV(gparticles[i].hue, 255, 255);
        uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
        uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
        uint8_t blu = ((color & 0xFF) * opacity) / 100;
        setPixel(x + (int)gparticles[i].x, y + (int)gparticles[i].y, red, grn,
                 blu);
      }
    }
  }

  else if (animId == "noiseflow") {
    {
      struct NParticle {
        float x, y, speed;
        uint16_t hue;
      };
      static NParticle nparticles[45];
      static bool noiseInit = false;
      if (!noiseInit) {
        for (int i = 0; i < 45; i++) {
          nparticles[i].x = random(0, MATRIX_WIDTH);
          nparticles[i].y = random(0, MATRIX_HEIGHT);
          nparticles[i].speed = 0.4 + (random(0, 400) / 1000.0);
          nparticles[i].hue = (random(0, 360) * 65535 / 360) & 65535;
        }
        noiseInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          uint32_t current = matrix.getPixelColor(py * MATRIX_WIDTH + px);
          uint8_t r = (uint8_t)(((current >> 16) & 0xFF) * 0.75);
          uint8_t g = (uint8_t)(((current >> 8) & 0xFF) * 0.75);
          uint8_t b = (uint8_t)((current & 0xFF) * 0.75);
          setPixel(x + px, y + py, r, g, b);
        }
      }

      float tick = millis() / 50.0;
      for (int i = 0; i < 45; i++) {
        float angle = sin(nparticles[i].x * 0.08) *
                          cos(nparticles[i].y * 0.15) * 6.28318 +
                      tick * 0.03;
        nparticles[i].x += cos(angle) * nparticles[i].speed;
        nparticles[i].y += sin(angle) * nparticles[i].speed;

        if (nparticles[i].x < 0 || nparticles[i].x >= MATRIX_WIDTH ||
            nparticles[i].y < 0 || nparticles[i].y >= MATRIX_HEIGHT) {
          nparticles[i].x = random(0, MATRIX_WIDTH);
          nparticles[i].y = random(0, MATRIX_HEIGHT);
        }

        uint32_t color = matrix.ColorHSV(nparticles[i].hue, 255, 255);
        uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
        uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
        uint8_t blu = ((color & 0xFF) * opacity) / 100;
        setPixel(x + (int)nparticles[i].x, y + (int)nparticles[i].y, red, grn,
                 blu);
      }
    }
  }

  else if (animId == "stars") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      for (int i = 0; i < 15; i++) {
        int seed = i * 73 + 19;
        int px = seed % MATRIX_WIDTH;
        int py = (seed / MATRIX_WIDTH) % MATRIX_HEIGHT;
        float twinkle = abs(sin(tick * 0.1 + i));
        if (twinkle > 0.4) {
          uint8_t r = (uint8_t)(253 * twinkle);
          uint8_t g = (uint8_t)(224 * twinkle);
          uint8_t b = (uint8_t)(71 * twinkle);
          setPixel(x + px, y + py, (r * opacity) / 100, (g * opacity) / 100,
                   (b * opacity) / 100);
        }
      }
    }
  }

  else if (animId == "hyperspace") {
    {
      struct HStar {
        float x, y, z;
      };
      static HStar hstars[32];
      static bool hstarsInit = false;
      if (!hstarsInit) {
        for (int i = 0; i < 32; i++) {
          hstars[i].x = (random(0, 1000) / 1000.0 - 0.5) * 40.0;
          hstars[i].y = (random(0, 1000) / 1000.0 - 0.5) * 16.0;
          hstars[i].z = 1.0 + (random(0, 1500) / 100.0);
        }
        hstarsInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;

      for (int i = 0; i < 32; i++) {
        hstars[i].z -= 0.18;
        if (hstars[i].z <= 0.1) {
          hstars[i].x = (random(0, 1000) / 1000.0 - 0.5) * 40.0;
          hstars[i].y = (random(0, 1000) / 1000.0 - 0.5) * 16.0;
          hstars[i].z = 15.0;
        }

        float k = 15.0 / hstars[i].z;
        int px = round(cx + hstars[i].x * k);
        int py = round(cy + hstars[i].y * k);

        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
          int brightness = (int)((1.0 - hstars[i].z / 15.0) * 255.0);
          if (brightness < 0)
            brightness = 0;
          if (brightness > 255)
            brightness = 255;
          uint8_t r = (brightness * opacity) / 100;
          uint8_t g = (brightness * opacity) / 100;
          uint8_t b = (255 * opacity) / 100;
          setPixel(x + px, y + py, r, g, b);
        }
      }
    }
  }

  else if (animId == "fireflies") {
    {
      struct FFly {
        float x, y, angle, speed, phase;
      };
      static FFly flies[12];
      static bool fliesInit = false;
      if (!fliesInit) {
        for (int i = 0; i < 12; i++) {
          flies[i].x = random(0, MATRIX_WIDTH);
          flies[i].y = random(0, MATRIX_HEIGHT);
          flies[i].angle = (random(0, 628) / 100.0);
          flies[i].speed = 0.15 + (random(0, 150) / 1000.0);
          flies[i].phase = (random(0, 628) / 100.0);
        }
        fliesInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float tick = millis() / 50.0;
      for (int i = 0; i < 12; i++) {
        flies[i].x += cos(flies[i].angle) * flies[i].speed;
        flies[i].y += sin(flies[i].angle) * flies[i].speed;
        flies[i].angle += (random(0, 100) / 100.0 - 0.5) * 0.35;

        if (flies[i].x < 0) {
          flies[i].x = 0;
          flies[i].angle = 3.14159 - flies[i].angle;
        }
        if (flies[i].x >= MATRIX_WIDTH) {
          flies[i].x = MATRIX_WIDTH - 1;
          flies[i].angle = 3.14159 - flies[i].angle;
        }
        if (flies[i].y < 0) {
          flies[i].y = 0;
          flies[i].angle = -flies[i].angle;
        }
        if (flies[i].y >= MATRIX_HEIGHT) {
          flies[i].y = MATRIX_HEIGHT - 1;
          flies[i].angle = -flies[i].angle;
        }

        float glow = abs(sin(tick * 0.05 + flies[i].phase));
        int flyX = (int)flies[i].x;
        int flyY = (int)flies[i].y;

        float sideOpacity = glow * 0.4 * opacity / 100.0;
        uint8_t sideR = (uint8_t)(163 * sideOpacity);
        uint8_t sideG = (uint8_t)(230 * sideOpacity);
        uint8_t sideB = (uint8_t)(53 * sideOpacity);

        if (flyX > 0)
          setPixel(x + flyX - 1, y + flyY, sideR, sideG, sideB);
        if (flyX < MATRIX_WIDTH - 1)
          setPixel(x + flyX + 1, y + flyY, sideR, sideG, sideB);
        if (flyY > 0)
          setPixel(x + flyX, y + flyY - 1, sideR, sideG, sideB);
        if (flyY < MATRIX_HEIGHT - 1)
          setPixel(x + flyX, y + flyY + 1, sideR, sideG, sideB);

        setPixel(x + flyX, y + flyY, 255 * opacity / 100, 255 * opacity / 100,
                 255 * opacity / 100);
      }
    }
  }

  else if (animId == "bounceballs") {
    {
      struct BBall {
        float x, y, vx, vy;
        uint32_t color;
        float radius;
        float trailX[6];
        float trailY[6];
      };
      static BBall balls[3];
      static bool ballsInit = false;
      if (!ballsInit) {
        balls[0] = {10, 4, 0.45, 0.35, 0xf43f5e, 1.2, {0}, {0}};
        balls[1] = {30, 8, -0.35, 0.55, 0x06b6d4, 1.5, {0}, {0}};
        balls[2] = {60, 5, 0.55, -0.45, 0x10b981, 1.0, {0}, {0}};
        for (int b = 0; b < 3; b++) {
          for (int t = 0; t < 6; t++) {
            balls[b].trailX[t] = balls[b].x;
            balls[b].trailY[t] = balls[b].y;
          }
        }
        ballsInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int b = 0; b < 3; b++) {
        for (int t = 5; t > 0; t--) {
          balls[b].trailX[t] = balls[b].trailX[t - 1];
          balls[b].trailY[t] = balls[b].trailY[t - 1];
        }
        balls[b].trailX[0] = balls[b].x;
        balls[b].trailY[0] = balls[b].y;

        balls[b].x += balls[b].vx;
        balls[b].y += balls[b].vy;

        if (balls[b].x - balls[b].radius < 0) {
          balls[b].x = balls[b].radius;
          balls[b].vx *= -1;
        }
        if (balls[b].x + balls[b].radius >= MATRIX_WIDTH) {
          balls[b].x = MATRIX_WIDTH - balls[b].radius;
          balls[b].vx *= -1;
        }
        if (balls[b].y - balls[b].radius < 0) {
          balls[b].y = balls[b].radius;
          balls[b].vy *= -1;
        }
        if (balls[b].y + balls[b].radius >= MATRIX_HEIGHT) {
          balls[b].y = MATRIX_HEIGHT - balls[b].radius;
          balls[b].vy *= -1;
        }

        uint8_t ballR = (balls[b].color >> 16) & 0xFF;
        uint8_t ballG = (balls[b].color >> 8) & 0xFF;
        uint8_t ballB = balls[b].color & 0xFF;

        for (int t = 0; t < 6; t++) {
          float tOpacity = (1.0 - (t / 6.0)) * 0.45 * opacity / 100.0;
          float trailR = balls[b].radius * (1.0 - (t / 6.0) * 0.5);
          int tx = (int)balls[b].trailX[t];
          int ty = (int)balls[b].trailY[t];

          for (int sx = -2; sx <= 2; sx++) {
            for (int sy = -2; sy <= 2; sy++) {
              if (sx * sx + sy * sy <= trailR * trailR) {
                int px = tx + sx;
                int py = ty + sy;
                if (px >= 0 && px < MATRIX_WIDTH && py >= 0 &&
                    py < MATRIX_HEIGHT) {
                  setPixel(x + px, y + py, ballR * tOpacity, ballG * tOpacity,
                           ballB * tOpacity);
                }
              }
            }
          }
        }

        int bx = (int)balls[b].x;
        int by = (int)balls[b].y;

        for (int sx = -2; sx <= 2; sx++) {
          for (int sy = -2; sy <= 2; sy++) {
            if (sx * sx + sy * sy <= balls[b].radius * balls[b].radius) {
              int px = bx + sx;
              int py = by + sy;
              if (px >= 0 && px < MATRIX_WIDTH && py >= 0 &&
                  py < MATRIX_HEIGHT) {
                setPixel(x + px, y + py, 255 * opacity / 100,
                         255 * opacity / 100, 255 * opacity / 100);
              }
            }
          }
        }

        for (int sx = -2; sx <= 2; sx++) {
          for (int sy = -2; sy <= 2; sy++) {
            float distSq = sx * sx + sy * sy;
            if (distSq <= balls[b].radius * balls[b].radius &&
                distSq > (balls[b].radius - 0.7) * (balls[b].radius - 0.7)) {
              int px = bx + sx;
              int py = by + sy;
              if (px >= 0 && px < MATRIX_WIDTH && py >= 0 &&
                  py < MATRIX_HEIGHT) {
                setPixel(x + px, y + py, ballR * opacity / 100,
                         ballG * opacity / 100, ballB * opacity / 100);
              }
            }
          }
        }
      }
    }
  }

  else if (animId == "sparks") {
    {
      struct PSpark {
        float x, y, vx, vy;
        int life, maxLife;
        uint16_t hue;
      };
      static PSpark sparks[25];
      static bool sparksInit = false;
      if (!sparksInit) {
        for (int i = 0; i < 25; i++) {
          sparks[i].x = MATRIX_WIDTH / 2.0;
          sparks[i].y = MATRIX_HEIGHT / 2.0;
          sparks[i].vx = (random(0, 1000) / 1000.0 - 0.5) * 1.5;
          sparks[i].vy = (random(0, 1000) / 1000.0 - 0.5) * 1.5 - 0.4;
          sparks[i].life = random(5, 25);
          sparks[i].maxLife = 25;
          sparks[i].hue = (random(0, 360) * 65535 / 360) & 65535;
        }
        sparksInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;

      for (int i = 0; i < 25; i++) {
        sparks[i].x += sparks[i].vx;
        sparks[i].y += sparks[i].vy;
        sparks[i].vy += 0.04;
        sparks[i].life--;

        if (sparks[i].life <= 0 || sparks[i].x < 0 ||
            sparks[i].x >= MATRIX_WIDTH || sparks[i].y < 0 ||
            sparks[i].y >= MATRIX_HEIGHT) {
          sparks[i].x = cx;
          sparks[i].y = cy;
          sparks[i].vx = (random(0, 1000) / 1000.0 - 0.5) * 1.5;
          sparks[i].vy = (random(0, 1000) / 1000.0 - 0.5) * 1.5 - 0.4;
          sparks[i].life = random(5, 25);
          sparks[i].maxLife = 25;
          sparks[i].hue = (random(0, 360) * 65535 / 360) & 65535;
        }

        float sOpacity =
            ((float)sparks[i].life / sparks[i].maxLife) * opacity / 100.0;
        uint32_t color = matrix.ColorHSV(sparks[i].hue, 255, 255);
        uint8_t red = (((color >> 16) & 0xFF) * sOpacity);
        uint8_t grn = (((color >> 8) & 0xFF) * sOpacity);
        uint8_t blu = ((color & 0xFF) * sOpacity);
        setPixel(x + (int)sparks[i].x, y + (int)sparks[i].y, red, grn, blu);
      }
    }
  }

  // ==================== 6. Helixes & 3D Structures ====================
  else if (animId == "dnahelix") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      for (int px = 2; px < MATRIX_WIDTH - 2; px += 3) {
        float offset = px * 0.16 + tick * 0.08;
        float waveA = sin(offset) * 5.0;
        float waveB = -sin(offset) * 5.0;

        int yA = round(MATRIX_HEIGHT / 2.0 + waveA);
        int yB = round(MATRIX_HEIGHT / 2.0 + waveB);

        float cosVal = cos(offset);
        if (cosVal > -0.2) {
          int startY = yA < yB ? yA : yB;
          int endY = yA > yB ? yA : yB;
          for (int py = startY; py <= endY; py++) {
            setPixel(x + px, y + py, 75 * opacity / 100, 85 * opacity / 100,
                     99 * opacity / 100);
          }
        }

        if (cosVal >= 0) {
          setPixel(x + px, y + yA, 6 * opacity / 100, 182 * opacity / 100,
                   212 * opacity / 100);
          setPixel(x + px, y + yB, 185 * opacity / 100, 28 * opacity / 100,
                   28 * opacity / 100);
        } else {
          setPixel(x + px, y + yA, 8 * opacity / 100, 145 * opacity / 100,
                   178 * opacity / 100);
          setPixel(x + px, y + yB, 244 * opacity / 100, 63 * opacity / 100,
                   94 * opacity / 100);
        }
      }
    }
  }

  else if (animId == "dna3d") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cy = MATRIX_HEIGHT / 2.0;

      for (int px = 4; px < MATRIX_WIDTH - 4; px += 2) {
        float angle = px * 0.18 + tick * 0.1;
        float cosVal = cos(angle);
        float sinVal = sin(angle);

        int yA = round(cy + sinVal * 6.0);
        int yB = round(cy - sinVal * 6.0);

        if (cosVal > -0.3) {
          int startY = yA < yB ? yA : yB;
          int endY = yA > yB ? yA : yB;
          for (int py = startY; py <= endY; py++) {
            setPixel(x + px, y + py, 55 * opacity / 100, 65 * opacity / 100,
                     81 * opacity / 100);
          }
        }

        if (cosVal >= 0) {
          setPixel(x + px, y + yA, 6 * opacity / 100, 182 * opacity / 100,
                   212 * opacity / 100);
          setPixel(x + px, y + yB, 153 * opacity / 100, 27 * opacity / 100,
                   27 * opacity / 100);
        } else {
          setPixel(x + px, y + yA, 8 * opacity / 100, 145 * opacity / 100,
                   178 * opacity / 100);
          setPixel(x + px, y + yB, 239 * opacity / 100, 68 * opacity / 100,
                   68 * opacity / 100);
        }
      }
    }
  }

  else if (animId == "tunnel") {
    {
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }
      float tick = millis() / 50.0;
      float cx = MATRIX_WIDTH / 2.0;
      float cy = MATRIX_HEIGHT / 2.0;
      float maxDist = sqrt(cx * cx + cy * cy);

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float dx = px - cx;
          float dy = py - cy;
          float dist = sqrt(dx * dx + dy * dy);
          float tunnelVal = sin(dist * 0.4 - tick * 0.15);
          if (tunnelVal > 0.6) {
            float tOpacity = (dist / maxDist) * opacity / 100.0;
            int hue = ((int)(dist * 8.0 + tick)) % 360;
            if (hue < 0)
              hue += 360;
            uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 255);
            uint8_t red = (((color >> 16) & 0xFF) * tOpacity);
            uint8_t grn = (((color >> 8) & 0xFF) * tOpacity);
            uint8_t blu = ((color & 0xFF) * tOpacity);
            setPixel(x + px, y + py, red, grn, blu);
          }
        }
      }
    }
  }

  // ==================== 7. Fire, Heat & Explosions ====================
  else if (animId == "firefastled") {
    {
      static uint8_t heat[80 * 16];
      static bool heatInit = false;
      if (!heatInit) {
        memset(heat, 0, sizeof(heat));
        heatInit = true;
      }

      int COOLING = 55;
      int SPARKING = 120;

      for (int i = 0; i < MATRIX_WIDTH; i++) {
        for (int j = 0; j < MATRIX_HEIGHT; j++) {
          int cooldown = random(0, ((COOLING * 10) / MATRIX_HEIGHT) + 2);
          int idx = i * MATRIX_HEIGHT + j;
          if (heat[idx] < cooldown) {
            heat[idx] = 0;
          } else {
            heat[idx] -= cooldown;
          }
        }
      }

      for (int i = 0; i < MATRIX_WIDTH; i++) {
        for (int j = MATRIX_HEIGHT - 1; j >= 2; j--) {
          int idx = i * MATRIX_HEIGHT + j;
          heat[idx] = (heat[idx - 1] + heat[idx - 2] + heat[idx - 2]) / 3;
        }
      }

      for (int i = 0; i < MATRIX_WIDTH; i++) {
        if (random(0, 255) < SPARKING) {
          int sy = random(0, 3);
          int idx = i * MATRIX_HEIGHT + sy;
          int val = heat[idx] + 160 + random(0, 95);
          if (val > 255)
            val = 255;
          heat[idx] = val;
        }
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          int hVal = heat[px * MATRIX_HEIGHT + py];
          int targetY = MATRIX_HEIGHT - 1 - py;
          if (hVal > 15) {
            uint8_t red = 0, grn = 0, blu = 0;
            if (hVal < 85) {
              red = hVal * 3;
            } else if (hVal < 170) {
              red = 255;
              grn = (hVal - 85) * 3;
            } else {
              red = 255;
              grn = 255;
              blu = (hVal - 170) * 3;
            }
            setPixel(x + px, y + targetY, red * opacity / 100,
                     grn * opacity / 100, blu * opacity / 100);
          } else {
            setPixel(x + px, y + targetY, 0, 0, 0);
          }
        }
      }
    }
  }

  else if (animId == "combustion") {
    {
      float tick = millis() / 50.0;
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        float wave = sin(px * 0.25 + tick * 0.15) * 3.5 +
                     cos(px * 0.45 - tick * 0.25) * 1.5;
        int flameHeight = 11 + (int)round(wave);

        for (int py = MATRIX_HEIGHT - 1; py >= MATRIX_HEIGHT - flameHeight;
             py--) {
          float depthRatio = (float)(MATRIX_HEIGHT - 1 - py) / flameHeight;
          int heat = 255 - (int)(depthRatio * 255.0);

          uint8_t red = 0, grn = 0, blu = 0;
          if (heat > 160) {
            red = 255;
            grn = 230;
            blu = (uint8_t)((heat - 160) * 2.5);
          } else if (heat > 80) {
            red = 255;
            grn = (uint8_t)((heat - 80) * 3);
          } else {
            red = heat * 3;
          }
          setPixel(x + px, y + py, red * opacity / 100, grn * opacity / 100,
                   blu * opacity / 100);
        }
      }
    }
  }

  else if (animId == "firecracker") {
    {
      struct Rocket {
        float x, y, vy;
        uint16_t hue;
      };
      struct FSpark {
        float x, y, vx, vy;
        uint16_t hue;
        int life, maxLife;
      };
      static Rocket rockets[2];
      static int numRockets = 0;
      static FSpark fsparks[60];
      static int numFSparks = 0;
      static bool fcInit = false;
      if (!fcInit) {
        numRockets = 0;
        numFSparks = 0;
        fcInit = true;
      }

      if (numRockets < 2 && random(0, 100) < 8) {
        rockets[numRockets] = {10.0f + random(0, MATRIX_WIDTH - 20),
                               (float)(MATRIX_HEIGHT - 1),
                               -0.8f - (random(0, 600) / 1000.0f),
                               (uint16_t)(random(0, 360) * 65535 / 360)};
        numRockets++;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int i = numRockets - 1; i >= 0; i--) {
        rockets[i].y += rockets[i].vy;

        setPixel(x + (int)rockets[i].x, y + (int)rockets[i].y,
                 255 * opacity / 100, 255 * opacity / 100, 255 * opacity / 100);
        if (rockets[i].y < MATRIX_HEIGHT - 1) {
          setPixel(x + (int)rockets[i].x, y + (int)rockets[i].y + 1,
                   255 * opacity / 100, 136 * opacity / 100, 0);
        }

        if (rockets[i].y < 3.0 + (random(0, 400) / 100.0) ||
            random(0, 100) < 5) {
          int count = 12 + random(0, 8);
          for (int p = 0; p < count; p++) {
            if (numFSparks < 60) {
              float angle = (p / (float)count) * 6.28318f +
                            (random(0, 100) / 100.0f - 0.5f) * 0.5f;
              float speed = 0.6f + (random(0, 800) / 1000.0f);
              fsparks[numFSparks] = {rockets[i].x,
                                     rockets[i].y,
                                     cos(angle) * speed,
                                     sin(angle) * speed - 0.2f,
                                     rockets[i].hue,
                                     15 + random(0, 12),
                                     27};
              numFSparks++;
            }
          }
          for (int j = i; j < numRockets - 1; j++) {
            rockets[j] = rockets[j + 1];
          }
          numRockets--;
        }
      }

      for (int i = numFSparks - 1; i >= 0; i--) {
        fsparks[i].x += fsparks[i].vx;
        fsparks[i].y += fsparks[i].vy;
        fsparks[i].vy += 0.035f;
        fsparks[i].life--;

        if (fsparks[i].life <= 0 || fsparks[i].x < 0 ||
            fsparks[i].x >= MATRIX_WIDTH || fsparks[i].y >= MATRIX_HEIGHT) {
          for (int j = i; j < numFSparks - 1; j++) {
            fsparks[j] = fsparks[j + 1];
          }
          numFSparks--;
          continue;
        }

        float pOpacity =
            ((float)fsparks[i].life / fsparks[i].maxLife) * opacity / 100.0;
        if (pOpacity < 0.1)
          pOpacity = 0.1;
        uint32_t color = matrix.ColorHSV(fsparks[i].hue, 255, 255);
        uint8_t red = (((color >> 16) & 0xFF) * pOpacity);
        uint8_t grn = (((color >> 8) & 0xFF) * pOpacity);
        uint8_t blu = ((color & 0xFF) * pOpacity);
        setPixel(x + (int)fsparks[i].x, y + (int)fsparks[i].y, red, grn, blu);
      }
    }
  }

  else if (animId == "fireworks") {
    {
      struct Burst {
        float cx, cy, r;
        bool active;
        uint16_t hue;
      };
      static Burst burst = {40.0, 8.0, 0.1, false, 0};
      static bool fireworksInit = false;
      if (!fireworksInit) {
        burst.active = false;
        fireworksInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float tick = millis() / 50.0;

      if (!burst.active) {
        burst.cx = 15.0 + random(0, MATRIX_WIDTH - 30);
        burst.cy = 3.0 + random(0, MATRIX_HEIGHT - 6);
        burst.r = 0.1;
        burst.active = true;
        burst.hue = (random(0, 360) * 65535 / 360) & 65535;
      } else {
        burst.r += 0.4;
        if (burst.r > 12.0) {
          burst.active = false;
        } else {
          float bOpacity = (1.0 - burst.r / 12.0) * opacity / 100.0;
          if (bOpacity < 0)
            bOpacity = 0;
          int numSparks = (int)(burst.r * 6.0) + 4;
          for (int i = 0; i < numSparks; i++) {
            float angle = (i / (float)numSparks) * 6.28318 + tick * 0.02;
            int sx = round(burst.cx + cos(angle) * burst.r);
            int sy = round(burst.cy + sin(angle) * burst.r +
                           burst.r * burst.r * 0.02);

            if (sx >= 0 && sx < MATRIX_WIDTH && sy >= 0 && sy < MATRIX_HEIGHT) {
              uint32_t color = matrix.ColorHSV(burst.hue, 255, 255);
              uint8_t red = (((color >> 16) & 0xFF) * bOpacity);
              uint8_t grn = (((color >> 8) & 0xFF) * bOpacity);
              uint8_t blu = ((color & 0xFF) * bOpacity);
              setPixel(x + sx, y + sy, red, grn, blu);
            }
          }
        }
      }
    }
  }

  else if (animId == "starburst") {
    {
      struct Burst {
        float cx, cy, r;
        uint16_t hue;
      };
      static Burst bursts[3];
      static int numBursts = 0;
      static bool sbInit = false;
      if (!sbInit) {
        numBursts = 0;
        sbInit = true;
      }

      if (numBursts < 3 && random(0, 100) < 6) {
        bursts[numBursts] = {15.0f + random(0, MATRIX_WIDTH - 30),
                             4.0f + random(0, MATRIX_HEIGHT - 8), 1.0f,
                             (uint16_t)(random(0, 360) * 65535 / 360)};
        numBursts++;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int i = numBursts - 1; i >= 0; i--) {
        bursts[i].r += 0.45;

        if (bursts[i].r > 20.0) {
          for (int j = i; j < numBursts - 1; j++) {
            bursts[j] = bursts[j + 1];
          }
          numBursts--;
          continue;
        }

        int numPoints = (int)(bursts[i].r * 5);
        float bOpacity = (1.0 - (bursts[i].r / 20.0)) * opacity / 100.0;
        if (bOpacity < 0.05)
          bOpacity = 0.05;

        for (int p = 0; p < numPoints; p++) {
          float angle = (p / (float)numPoints) * 6.28318;
          int px = round(bursts[i].cx + cos(angle) * bursts[i].r);
          int py = round(bursts[i].cy + sin(angle) * bursts[i].r);

          if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            uint16_t ringHue = (bursts[i].hue + p * 8 * 65535 / 360) & 65535;
            uint32_t color = matrix.ColorHSV(ringHue, 255, 255);
            uint8_t red = (((color >> 16) & 0xFF) * bOpacity);
            uint8_t grn = (((color >> 8) & 0xFF) * bOpacity);
            uint8_t blu = ((color & 0xFF) * bOpacity);
            setPixel(x + px, y + py, red, grn, blu);
          }
        }
      }
    }
  }

  else if (animId == "ripples") {
    {
      struct Ripple {
        float cx, cy, r, maxR;
      };
      static Ripple ripples[4];
      static int numRipples = 0;
      static bool rpInit = false;
      if (!rpInit) {
        numRipples = 0;
        rpInit = true;
      }

      if (numRipples < 4 && random(0, 100) < 8) {
        ripples[numRipples] = {(float)random(0, MATRIX_WIDTH),
                               (float)random(0, MATRIX_HEIGHT), 1.0f,
                               8.0f + random(0, 8)};
        numRipples++;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int i = numRipples - 1; i >= 0; i--) {
        ripples[i].r += 0.35;

        if (ripples[i].r >= ripples[i].maxR) {
          for (int j = i; j < numRipples - 1; j++) {
            ripples[j] = ripples[j + 1];
          }
          numRipples--;
          continue;
        }

        float rOpacity =
            (1.0 - (ripples[i].r / ripples[i].maxR)) * opacity / 100.0;
        float step = 0.1 / ripples[i].r;

        for (float theta = 0; theta < 6.28318; theta += step * 8.0) {
          int px = round(ripples[i].cx + cos(theta) * ripples[i].r);
          int py = round(ripples[i].cy + sin(theta) * ripples[i].r);

          if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            setPixel(x + px, y + py, 6 * rOpacity, 182 * rOpacity,
                     212 * rOpacity);
          }
        }
      }
    }
  }

  else if (animId == "metaballs") {
    {
      struct Blob {
        float x, y, vx, vy, r;
      };
      static Blob blobs[3];
      static bool mbInit = false;
      if (!mbInit) {
        blobs[0] = {20.0, 5.0, 0.45, 0.35, 4.5};
        blobs[1] = {50.0, 10.0, -0.35, 0.45, 5.5};
        blobs[2] = {40.0, 8.0, 0.55, -0.25, 3.5};
        mbInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int i = 0; i < 3; i++) {
        blobs[i].x += blobs[i].vx;
        blobs[i].y += blobs[i].vy;

        if (blobs[i].x - blobs[i].r < 0 ||
            blobs[i].x + blobs[i].r >= MATRIX_WIDTH)
          blobs[i].vx *= -1;
        if (blobs[i].y - blobs[i].r < 0 ||
            blobs[i].y + blobs[i].r >= MATRIX_HEIGHT)
          blobs[i].vy *= -1;
      }

      float tick = millis() / 50.0;
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float sum = 0;
          for (int i = 0; i < 3; i++) {
            float dx = px - blobs[i].x;
            float dy = py - blobs[i].y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist > 0) {
              sum += blobs[i].r / dist;
            }
          }

          if (sum > 1.25) {
            int hue = ((int)(sum * 45.0 + tick * 1.5)) % 360;
            if (hue < 0)
              hue += 360;
            uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 255);
            uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
            uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
            uint8_t blu = ((color & 0xFF) * opacity) / 100;
            setPixel(x + px, y + py, red, grn, blu);
          }
        }
      }
    }
  }

  else if (animId == "lavalamp") {
    {
      struct LavaBlob {
        float x, y, vx, vy, r;
      };
      static LavaBlob lavablobs[3];
      static bool llInit = false;
      if (!llInit) {
        lavablobs[0] = {15.0, 8.0, 0.15, 0.05, 6.0};
        lavablobs[1] = {45.0, 5.0, -0.1, -0.07, 7.0};
        lavablobs[2] = {60.0, 10.0, 0.08, 0.04, 5.5};
        llInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int i = 0; i < 3; i++) {
        lavablobs[i].x += lavablobs[i].vx;
        lavablobs[i].y += lavablobs[i].vy;

        if (lavablobs[i].x - lavablobs[i].r < 5.0 ||
            lavablobs[i].x + lavablobs[i].r >= MATRIX_WIDTH - 5.0)
          lavablobs[i].vx *= -1;
        if (lavablobs[i].y - lavablobs[i].r < 1.0 ||
            lavablobs[i].y + lavablobs[i].r >= MATRIX_HEIGHT - 1.0)
          lavablobs[i].vy *= -1;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          float val = 0;
          for (int i = 0; i < 3; i++) {
            float dx = px - lavablobs[i].x;
            float dy = py - lavablobs[i].y;
            float d = dx * dx + dy * dy;
            if (d > 0) {
              val += (lavablobs[i].r * lavablobs[i].r) / d;
            }
          }

          if (val > 1.2) {
            if (val > 2.2) {
              setPixel(x + px, y + py, 250 * opacity / 100, 204 * opacity / 100,
                       21 * opacity / 100);
            } else {
              setPixel(x + px, y + py, 249 * opacity / 100, 115 * opacity / 100,
                       22 * opacity / 100);
            }
          }
        }
      }
    }
  }

  // ==================== 8. Grid, Game & Block Scenarios ====================
  else if (animId == "snake") {
    {
      struct SnakePoint {
        int x, y;
      };
      static SnakePoint sbody[100];
      static int sbodyLen = 3;
      static SnakePoint sdir = {1, 0};
      static SnakePoint sfood = {10, 5};
      static int sscore = 0;
      static bool snakeInit = false;

      auto spawnFood = [&]() {
        int tries = 0;
        while (tries < 100) {
          int fx = random(0, MATRIX_WIDTH);
          int fy = random(0, MATRIX_HEIGHT);
          bool onSnake = false;
          for (int i = 0; i < sbodyLen; i++) {
            if (sbody[i].x == fx && sbody[i].y == fy) {
              onSnake = true;
              break;
            }
          }
          if (!onSnake) {
            sfood = {fx, fy};
            return;
          }
          tries++;
        }
        sfood = {1, 1};
      };

      auto initGame = [&]() {
        sbody[0] = {7, 5};
        sbody[1] = {6, 5};
        sbody[2] = {5, 5};
        sbodyLen = 3;
        sdir = {1, 0};
        spawnFood();
        sscore = 0;
      };

      if (!snakeInit) {
        initGame();
        snakeInit = true;
      }

      float tick = millis() / 50.0;
      static int lastUpdateTick = 0;
      int currentTickInt = (int)(tick / 2.0);

      if (currentTickInt != lastUpdateTick) {
        lastUpdateTick = currentTickInt;
        SnakePoint head = sbody[0];
        SnakePoint food = sfood;
        SnakePoint possibleDirs[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

        SnakePoint validDirs[4];
        int validCount = 0;
        for (int i = 0; i < 4; i++) {
          int nx = head.x + possibleDirs[i].x;
          int ny = head.y + possibleDirs[i].y;
          if (nx < 0 || nx >= MATRIX_WIDTH || ny < 0 || ny >= MATRIX_HEIGHT)
            continue;
          bool onSnake = false;
          for (int b = 0; b < sbodyLen; b++) {
            if (sbody[b].x == nx && sbody[b].y == ny) {
              onSnake = true;
              break;
            }
          }
          if (!onSnake) {
            validDirs[validCount++] = possibleDirs[i];
          }
        }

        if (validCount > 0) {
          int bestIdx = 0;
          int minDist = 9999;
          for (int i = 0; i < validCount; i++) {
            int dist = abs(head.x + validDirs[i].x - food.x) +
                       abs(head.y + validDirs[i].y - food.y);
            if (dist < minDist) {
              minDist = dist;
              bestIdx = i;
            }
          }
          sdir = validDirs[bestIdx];
        }

        SnakePoint newHead = {head.x + sdir.x, head.y + sdir.y};
        bool selfCollision = false;
        for (int b = 0; b < sbodyLen; b++) {
          if (sbody[b].x == newHead.x && sbody[b].y == newHead.y) {
            selfCollision = true;
            break;
          }
        }
        bool wallCollision = newHead.x < 0 || newHead.x >= MATRIX_WIDTH ||
                             newHead.y < 0 || newHead.y >= MATRIX_HEIGHT;

        if (selfCollision || wallCollision) {
          initGame();
        } else {
          for (int i = sbodyLen; i > 0; i--) {
            sbody[i] = sbody[i - 1];
          }
          sbody[0] = newHead;
          if (newHead.x == food.x && newHead.y == food.y) {
            sbodyLen++;
            if (sbodyLen > 99)
              sbodyLen = 99;
            spawnFood();
          } else {
            // keep length same
          }
        }
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float foodTwinkle = abs(sin(tick * 0.2)) * 30.0 + 70.0;
      uint32_t foodColor =
          matrix.ColorHSV(45 * 65535 / 360, 255, (uint8_t)foodTwinkle);
      setPixel(x + sfood.x, y + sfood.y,
               (((foodColor >> 16) & 0xFF) * opacity) / 100,
               (((foodColor >> 8) & 0xFF) * opacity) / 100,
               ((foodColor & 0xFF) * opacity) / 100);

      for (int i = 0; i < sbodyLen; i++) {
        int hue = ((int)(i * 15.0 + tick * 6.0)) % 360;
        if (hue < 0)
          hue += 360;
        uint32_t bodyColor =
            (i == 0) ? 0xFFFFFF : matrix.ColorHSV(hue * 65535 / 360, 255, 255);
        setPixel(x + sbody[i].x, y + sbody[i].y,
                 ((bodyColor >> 16) & 0xFF) * opacity / 100,
                 ((bodyColor >> 8) & 0xFF) * opacity / 100,
                 (bodyColor & 0xFF) * opacity / 100);
      }
    }
  }

  else if (animId == "sandworm") {
    {
      struct WormPoint {
        float x, y;
      };
      static WormPoint wormHistory[8];
      static bool wormInit = false;
      if (!wormInit) {
        for (int i = 0; i < 8; i++) {
          wormHistory[i] = {-20, 12};
        }
        wormInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      float tick = millis() / 50.0;
      float wx = fmod(tick * 0.5, MATRIX_WIDTH + 24.0) - 12.0;
      float wy = 12.0 + sin(wx * 0.14) * 4.5;

      for (int i = 7; i > 0; i--) {
        wormHistory[i] = wormHistory[i - 1];
      }
      wormHistory[0] = {wx, wy};

      auto getDuneBackY = [](float cx) { return 10.0 + sin(cx * 0.07) * 1.5; };
      auto getDuneFrontY = [](float cx) { return 12.0 + cos(cx * 0.09) * 1.5; };

      for (int cx = 0; cx < MATRIX_WIDTH; cx++) {
        int dy = (int)getDuneBackY(cx);
        for (int cy = dy; cy < MATRIX_HEIGHT; cy++) {
          setPixel(x + cx, y + cy, (180 * opacity) / 100, (83 * opacity) / 100,
                   (9 * opacity) / 100);
        }
      }

      for (int index = 7; index >= 0; index--) {
        int sizeIndex = 8 - index;
        float radius = 1.5 + (sizeIndex / 8.0) * 2.5; // Increased size

        int wxInt = (int)wormHistory[index].x;
        int wyInt = (int)wormHistory[index].y;

        uint8_t wr, wg, wb;
        if (index % 2 == 0) {
          wr = (31 * opacity) / 100;
          wg = (41 * opacity) / 100;
          wb = (55 * opacity) / 100;
        } else {
          wr = (217 * opacity) / 100;
          wg = (119 * opacity) / 100;
          wb = (6 * opacity) / 100;
        }

        for (int sx = -4; sx <= 4; sx++) {
          for (int sy = -4; sy <= 4; sy++) {
            if (sx * sx + sy * sy <= radius * radius) {
              int px = wxInt + sx;
              int py = wyInt + sy;

              if (px >= 0 && px < MATRIX_WIDTH) {
                float backDuneY = getDuneBackY(px);
                if (py < backDuneY + 2) {
                  float frontDuneY = getDuneFrontY(px);
                  if (py < frontDuneY) {
                    setPixel(x + px, y + py, wr, wg, wb);
                  }
                }
              }
            }
          }
        }
      }

      for (int cx = 0; cx < MATRIX_WIDTH; cx++) {
        int dy = (int)getDuneFrontY(cx);
        for (int cy = dy; cy < MATRIX_HEIGHT; cy++) {
          setPixel(x + cx, y + cy, (217 * opacity) / 100, (119 * opacity) / 100,
                   (6 * opacity) / 100);
        }
      }
    }
  }

  else if (animId == "blocks") {
    {
      float tick = millis() / 50.0;
      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int blockIdx = 0; blockIdx < 8; blockIdx++) {
        int bx = round(blockIdx * 10.0 + sin(tick * 0.04 + blockIdx) * 3.0);
        int by = ((int)(tick * 0.3 + blockIdx * 4.0)) % (MATRIX_HEIGHT + 6) - 4;

        int hue = ((int)(blockIdx * 45.0 + tick)) % 360;
        if (hue < 0)
          hue += 360;
        uint32_t color = matrix.ColorHSV(hue * 65535 / 360, 255, 140);

        uint8_t red = (((color >> 16) & 0xFF) * opacity) / 100;
        uint8_t grn = (((color >> 8) & 0xFF) * opacity) / 100;
        uint8_t blu = ((color & 0xFF) * opacity) / 100;

        for (int sx = 0; sx < 3; sx++) {
          for (int sy = 0; sy < 3; sy++) {
            int px = bx + sx;
            int py = by + sy;
            if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
              setPixel(x + px, y + py, red, grn, blu);
            }
          }
        }
      }
    }
  }

  else if (animId == "colorrain") {
    {
      struct Col {
        float headY;
        float speed;
        uint16_t hue;
      };
      static Col columns[40];
      static bool colInit = false;
      if (!colInit) {
        for (int i = 0; i < 40; i++) {
          columns[i].headY = -1.0 * random(0, 120) / 10.0;
          columns[i].speed = 0.2 + (random(0, 300) / 1000.0);
          columns[i].hue = (i * 18 * 65535 / 360) & 65535;
        }
        colInit = true;
      }

      for (int px = 0; px < MATRIX_WIDTH; px++) {
        for (int py = 0; py < MATRIX_HEIGHT; py++) {
          setPixel(x + px, y + py, 0, 0, 0);
        }
      }

      for (int i = 0; i < 40; i++) {
        columns[i].headY += columns[i].speed;
        if (columns[i].headY >= MATRIX_HEIGHT + 10) {
          columns[i].headY = -5;
          columns[i].speed = 0.2 + (random(0, 300) / 1000.0);
        }

        int colX = i * 2;
        int headInt = (int)columns[i].headY;

        for (int t = 0; t < 6; t++) {
          int trailY = headInt - t;
          if (trailY >= 0 && trailY < MATRIX_HEIGHT) {
            if (t == 0) {
              setPixel(x + colX, y + trailY, 255 * opacity / 100,
                       255 * opacity / 100, 255 * opacity / 100);
            } else {
              float tailOpacity = (1.0 - (t / 6.0)) * opacity / 100.0;
              uint32_t color = matrix.ColorHSV(columns[i].hue, 255, 255);
              uint8_t r = ((color >> 16) & 0xFF) * tailOpacity;
              uint8_t g = ((color >> 8) & 0xFF) * tailOpacity;
              uint8_t b = (color & 0xFF) * tailOpacity;
              setPixel(x + colX, y + trailY, r, g, b);
            }
          }
        }
      }
    }
  }
}

void drawWeatherTemp(int x, int y, int w, int h, uint32_t color, int size,
                     bool shadow, uint32_t shadowColor, String effect,
                     int speed, bool bold, bool rainbow) {
  String tempStr = String(weatherTemp, 1) + "C";
  drawText(tempStr, x, y, w, h, color, size, shadow, shadowColor, effect, speed,
           bold, rainbow);
}

void drawWeatherHumi(int x, int y, int w, int h, uint32_t color, int size,
                     bool shadow, uint32_t shadowColor, String effect,
                     int speed, bool bold, bool rainbow) {
  String humiStr = String(weatherHumidity) + "%";
  drawText(humiStr, x, y, w, h, color, size, shadow, shadowColor, effect, speed,
           bold, rainbow);
}

void drawWeatherBrief(int x, int y, int w, int h, uint32_t color, int size,
                      bool shadow, uint32_t shadowColor, String effect,
                      int speed, bool bold, bool rainbow) {
  String briefStr = weatherDesc;
  drawText(briefStr, x, y, w, h, color, size, shadow, shadowColor, effect,
           speed, bold, rainbow);
}
