
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "time.h"
//TODO figure out how to install this thing
//#include <sunset.h>


#define LED_PIN     0

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  15
// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 80 // Set BRIGHTNESS to about 1/5 (max = 255)

#define uS_TO_S_FACTOR 1000000UL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  120        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
const char* ssid       = "<>";
const char* password   = "<>";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;


uint32_t red;
uint32_t white;
uint32_t green;
uint32_t blue;
uint32_t domo_blue;
uint32_t purple;
uint32_t orange;
uint32_t yellow;
uint32_t black;



// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:


void setup() {
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();  

  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
  red = strip.Color(255, 0, 0);
  white = strip.Color(255, 255, 255);
  green = strip.Color(1, 145, 18);
  blue = strip.Color(0, 42, 255);
  domo_blue = strip.Color(139, 191, 229);
  purple = strip.Color(132, 0, 255);
  orange = strip.Color(255, 98, 0);
  yellow = strip.Color(255, 242, 0);
  black = strip.Color(0,0,0);

  colorWipe(red, 50);
  colorWipe(white, 50);
  colorWipe(green, 50);
  colorWipe(blue, 50);
  colorWipe(domo_blue, 50);
  colorWipe(purple, 50);
  colorWipe(orange, 50);
  colorWipe(yellow, 50);
  
  delay(100);  

  getCurrentTime();

  Serial.println("Time" + gettimeofday());

  

  nightyNight(120, true);
}


void loop() { 
}



void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void offWipe(int wait) {
  Serial.println("Going to call clear");
  strip.clear();
  Serial.println("Called Clear");
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(black, black);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}



void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void nightyNight(uint32_t sleep_length_seconds, bool lights_off){
  esp_sleep_enable_timer_wakeup(sleep_length_seconds * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(sleep_length_seconds/60) +" MINUTES");
  Serial.println("Going to sleep now");
  if (lights_off){
    offWipe(200);
  }
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();

}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void getCurrentTime(){
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
}
