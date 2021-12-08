#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "time.h"
//#include <TimeLib.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>


#define LED_PIN     0

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  15
// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 80 // Set BRIGHTNESS to about 1/5 (max = 255)

//timing for normal sleep
#define uS_TO_S_FACTOR 1000000LL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  120        /* Time ESP32 will go to sleep (in seconds) */

//Location info for sunset
String openWeatherMapApiKey = "";
String LATITUDE = "40.3093";
String LONGITUDE = "-112.0120";
String jsonBuffer;

//HolidayApi
String host_headers_key = "x-rapidapi-host";
String host_headers_value = "public-holiday.p.rapidapi.com";
String api_key_header_key = "x-rapidapi-key";
String api_key_header_value ="";

RTC_DATA_ATTR int bootCount = 0;
const char* ssid       = "";
const char* password   = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

const int connection_count_limit = 20;

uint32_t red;
uint32_t white;
uint32_t green;
uint32_t blue;
uint32_t domo_blue;
uint32_t purple;
uint32_t orange;
uint32_t yellow;
uint32_t black;


int wake_up_time = 3600;
int light_on_duration_seconds = 7200;
bool toggle = false;



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
//  strip.show();            // Turn OFF all pixels ASAP
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

//  colorWipe(red, 50);
//  colorWipe(white, 50);
//  colorWipe(green, 50);
//  colorWipe(blue, 50);
//  colorWipe(domo_blue, 50);
//  colorWipe(purple, 50);
//  colorWipe(orange, 50);
//  colorWipe(yellow, 50);
  
  delay(100);  

  determineTiming();

  determineLightMode();

//  Serial.println("Time" + gettimeofday());

  
  nightyNight(wake_up_time, toggle);

  
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

void colorWipe2Colors(uint32_t color1,uint32_t color2, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    if ( i % 2 == 0){
      strip.setPixelColor(i, color1);
    }

    else{
      strip.setPixelColor(i, color2);
    }
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void christmasWipe(uint32_t color1,uint32_t color2, int wait) {
  for(int i=0; i<3; i++) { // For each pixel in strip...
      strip.setPixelColor(i, color1);
      strip.show();                          //  Update strip to match
      delay(wait);  
  }
  for(int i=4; i<6; i++) { // For each pixel in strip...
      strip.setPixelColor(i, color2);
      strip.show();                          //  Update strip to match
      delay(wait);  
  }
  for(int i=7; i<10; i++) { // For each pixel in strip...
      strip.setPixelColor(i, color1);
      strip.show();                          //  Update strip to match
      delay(wait);  
  }
  for(int i=11; i<14; i++) { // For each pixel in strip...
      strip.setPixelColor(i, color2);
      strip.show();                          //  Update strip to match
      delay(wait);  
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

void nightyNight(uint32_t sleep_length_seconds, bool lights_on){
  Serial.println("Sleep length seconds");
  Serial.println(sleep_length_seconds);
  Serial.println("What I passed to wakeup timer");
  Serial.println(sleep_length_seconds * uS_TO_S_FACTOR);
  esp_sleep_enable_timer_wakeup(sleep_length_seconds * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(sleep_length_seconds/60) +" MINUTES");
  Serial.println("Going to sleep now");
  if (lights_on==false){
    offWipe(200);
  }
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();

}

int getEpochCurrentTime(){
  int epoch_time = 0;
  int attempt_counter_limit = 20;
  int attempt_counter = 0;
  struct tm timeinfo;
  while(attempt_counter<attempt_counter_limit){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");    
    }
    else{
      epoch_time = mktime(&timeinfo);
      return epoch_time;
      break;
      }
  }
  return epoch_time;
  }


void determineLightMode(){
  if(true and toggle){
    Serial.println("Christmas Livghts Mode On");
    christmasWipe(green,red, 100); 
  }
}

void determineTiming(){
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
 
  int connection_counter = 0;
  while ((WiFi.status() != WL_CONNECTED) or connection_counter==connection_count_limit) {
      connection_counter++;
      delay(500);
      Serial.print(".");
  }
  if(connection_counter==connection_count_limit){
    Serial.println("Unable to Connect after"+connection_count_limit);
  }
  else{
    Serial.println(" CONNECTED");
    delay(500);
    
    int epoch_current_time = getEpochCurrentTime();
    Serial.println("Epoch Current time is");
    Serial.println(epoch_current_time);
    
    int sunset_time = getSunset();
    Serial.println("Sunset time is");
    //testing make the sunset time way later
//    sunset_time=sunset_time +23900;
    Serial.println(sunset_time);
    int now_and_sunset_delta = epoch_current_time - sunset_time;

    Serial.println("delattime");
    Serial.println(now_and_sunset_delta);
//    getHolidays();
    //TODO THIS doesn't work
    if(now_and_sunset_delta < 0){
      Serial.println("Condition 1, it is before sunset, delta set to sunset time");
      wake_up_time = now_and_sunset_delta * -1;
      toggle = false;
      }
    else if(now_and_sunset_delta >= 0  and now_and_sunset_delta < light_on_duration_seconds){
      Serial.println("Condition 2, sunset has occurred, but only just, wake up time set to how long we want the lights to be one for");
      wake_up_time = light_on_duration_seconds;
      toggle = true;
      }
    else if(now_and_sunset_delta >= light_on_duration_seconds){
      Serial.println("Condition 3, sunset has occurred, but it is now time to shut off the lights. Sleep a long time till we can get the next sunset time.");
      wake_up_time = 21600;
      toggle = false;
      }
    Serial.println("wake_up_time");
    Serial.println(wake_up_time);
    Serial.println("toggle");
    Serial.println(toggle);
    
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
}

int getSunset(){
  int sunset_time = 0;
  if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?lat=" + LATITUDE + "&lon=" + LONGITUDE + "&APPID=" + openWeatherMapApiKey + "&units=imperial";
      String jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Sunset Data Parsing input failed!");
        return sunset_time;
      }
      sunset_time = myObject["sys"]["sunset"];
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  return sunset_time;
}

  
JSONVar getHolidays(){
  Serial.println("I am over here dammit");
  JSONVar holidays = null;
  int year_num = 0;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");    
    }
    else{
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
      year_num = timeinfo.tm_year + 1900;
      
      Serial.println(year_num);
    }
    
  if(WiFi.status()== WL_CONNECTED){
    String serverPath = "https://public-holiday.p.rapidapi.com/"+ String(year_num) +"/US";
    Serial.println(serverPath);
    String jsonBuffer = httpGETRequestWithHeaders(serverPath.c_str(),host_headers_key, host_headers_value, api_key_header_key, api_key_header_value);
    Serial.println(jsonBuffer);
    JSONVar myObject = JSON.parse(jsonBuffer);

    // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
         Serial.println("Holiday Parsing input failed!");
         
      }
      holidays = myObject;
      Serial.println(myObject);
  }
  else {
        Serial.println("WiFi Disconnected");
  }
  return holidays;
}

void printTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");    
    }
    else{
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    }
  
  }

String httpGETRequestWithHeaders(const char* serverName, String header_1_key, String header_1_value, String header_2_key, String header_2_value) {
  WiFiClient client;
  HTTPClient http;
  int httpResponseCode = 0;
  String payload = "{}"; 
  int attempt_counter_limit = 20;
  int attempt_counter = 0;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  http.addHeader(header_1_value,header_1_value);
  http.addHeader(header_2_value,header_2_value);
  http.addHeader("CONTENT","application/json");

  while(attempt_counter<attempt_counter_limit){
    attempt_counter++;
    httpResponseCode = http.GET();
    if (httpResponseCode=200) {
//      #TODO I am getting 200s but am not getting the data right. 
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
      Serial.println("I got to the 200 code area");
      Serial.println("Printing payload");
      Serial.println(payload);
      break;
    }
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    delay(500);
    // Free resources
    http.end();
  }
 return payload;
}

String httpGETRequest(String serverName) {
  WiFiClient client;
  HTTPClient http;
  int httpResponseCode = 0;
  String payload = "{}"; 
  int attempt_counter_limit = 20;
  int attempt_counter = 0;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);

  while(attempt_counter<attempt_counter_limit){
    attempt_counter++;
    httpResponseCode = http.GET();
    if (httpResponseCode=200) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
      break;
    }
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    delay(500);
    // Free resources
    http.end();
  }

 return payload;
}
