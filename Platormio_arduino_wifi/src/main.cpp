#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <ESP32Time.h>
#include "secrets.h"
#define Builtin_LED 2 // Indicator LED for the MCU
#define data_line 17 // Pin for sending the activation signal

// NTP Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = (-5)*3600;  // offset for Central Standard Time in seconds (which is "-5 GMT")
const int daylightOffset_sec = 0; // Offset for daylight savings, set to 0 after changing time in spring, set to 3600 when changing time in the fall. 

// Target time to send activation signal (24-hour format) (currently set to 8:15 AM)
const int TARGET_HOUR = 6;    // 6 hours
const int TARGET_MINUTE = 45;  // 45 minutes
const int TARGET_SECOND = 0;   // 0 seconds

// Deep sleep duration (seconds)
const int CHECK_INTERVAL = 60;  // Number of seconds to check time
ESP32Time rtc(0); 

// RTC Data storage in RTC memory
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR time_t lastActivation = 0;

void connectToWiFi() {
  Serial.print("\n\rConnecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(800);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("  Connected to WiFi");
  } else {
    Serial.println("  Failed to connect to WiFi");
  }
}
void syncTimeWithNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  
  // Set RTC with NTP time
  time_t now;
  time(&now);
  rtc.setTime(now);
  
  Serial.print("\nTime synchronized: ");
  Serial.println(rtc.getTime("%Y-%m-%d %H:%M:%S"));
}

bool shouldActivate() {
  // Get current time from RTC
  int currentHour = rtc.getHour(true);  // 24-hour format
  int currentMinute = rtc.getMinute();
  int currentSecond = rtc.getSecond();
  
  Serial.printf("\rCurrent time: %02d:%02d:%02d\n", 
                currentHour, currentMinute, currentSecond);
  Serial.printf("\rTarget time: %02d:%02d:%02d\n", 
                TARGET_HOUR, TARGET_MINUTE, TARGET_SECOND);
  
  // Check if current time matches target time
  if (currentHour == TARGET_HOUR && 
      currentMinute == TARGET_MINUTE) {
    
    // Prevent multiple activations in the same minute
    time_t now = rtc.getEpoch();
    if ((now - lastActivation) > CHECK_INTERVAL) {
      lastActivation = now;
      return true;
    }
  }
  
  return false;
}

void triggerSignal() {
  Serial.println("\nActivating ...");
  digitalWrite(Builtin_LED, HIGH); 
  digitalWrite(data_line, HIGH);
  delay(500);
  Serial.println("Signal Trigger Complete");
}

void setup() {
  Serial.begin(115200);
  pinMode(Builtin_LED, OUTPUT);      // set the LED pin mode
  pinMode(data_line, OUTPUT);
  bootCount++;
  Serial.printf("\nBoot count: %d", bootCount);

  // Connect to WiFi and sync time (only when needed)
  connectToWiFi();
  
  if (WiFi.status() == WL_CONNECTED) {
    syncTimeWithNTP();
    // Disconnect WiFi to save power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
  
  // Check if it's time to activate the heater
  if (shouldActivate()) {
    triggerSignal();
  }
  
  // Prepare for deep sleep
  Serial.printf("\n\rGoing to deep sleep for %d seconds... ", CHECK_INTERVAL);
  
  // Configure wakeup sources for lowest power
  esp_sleep_enable_timer_wakeup(CHECK_INTERVAL * 1000000);
  
  // Enable RTC peripherals to keep time during sleep
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  
  // Enter deep sleep
  esp_deep_sleep_start();
  
}

void loop(){

}


