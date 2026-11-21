/*
 * Smart Cold Storage System - ESP32 Supervisory Controller
 * Course: 21EIC302J - Process Control (SRMIST)
 *
 * Monitors DHT11 temperature & humidity, displays live values and a 3-hour
 * cooling cycle countdown on a 16x2 I2C LCD, and syncs telemetry & alerts
 * with the Blynk IoT platform.
 */

#include "config.h"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Hardware Pin Definitions
#define DHT_PIN         4
#define DHT_TYPE        DHT11
#define BUTTON_PIN      14
#define LCD_I2C_ADDR    0x27

// Blynk Virtual Pins
#define VPIN_TEMP       V0
#define VPIN_HUMIDITY   V1
#define VPIN_TIMER      V2
#define VPIN_STATUS     V3

// 3-Hour Cooling Cycle Duration (in milliseconds)
const unsigned long CYCLE_DURATION_MS = 3UL * 60UL * 60UL * 1000UL;
const unsigned long SENSOR_INTERVAL_MS = 2000UL;
const unsigned long DEBOUNCE_DELAY_MS = 50UL;

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);
BlynkTimer blynkTimer;

// Timer & Button State
bool cycleActive = false;
bool cycleCompletedAlertShown = false;
unsigned long cycleStartMillis = 0;

int lastButtonReading = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;

float currentTemp = 0.0f;
float currentHumidity = 0.0f;

String formatRemainingTime(unsigned long remainingMs) {
  unsigned long totalSec = remainingMs / 1000UL;
  unsigned int hours = totalSec / 3600UL;
  unsigned int minutes = (totalSec % 3600UL) / 60UL;
  unsigned int seconds = totalSec % 60UL;

  char buf[12];
  snprintf(buf, sizeof(buf), "%02u:%02u:%02u", hours, minutes, seconds);
  return String(buf);
}

void startCoolingCycle() {
  cycleActive = true;
  cycleCompletedAlertShown = false;
  cycleStartMillis = millis();
  Serial.println("[CYCLE] 3-Hour cooling cycle started.");
  Blynk.virtualWrite(VPIN_STATUS, "Cooling Cycle Active");
}

void updateDisplay() {
  lcd.setCursor(0, 0);
  char line1[17];
  snprintf(line1, sizeof(line1), "T:%4.1fC H:%3.0f%%  ", currentTemp, currentHumidity);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  if (cycleActive) {
    unsigned long elapsed = millis() - cycleStartMillis;
    unsigned long remaining = (elapsed < CYCLE_DURATION_MS) ? (CYCLE_DURATION_MS - elapsed) : 0;
    String timeStr = "Time: " + formatRemainingTime(remaining) + "  ";
    lcd.print(timeStr.substring(0, 16));
  } else if (cycleCompletedAlertShown) {
    lcd.print("Cycle Complete! ");
  } else {
    lcd.print("Press Btn: 3h   ");
  }
}

void readAndSyncSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    currentHumidity = h;
    currentTemp = t;

    Blynk.virtualWrite(VPIN_TEMP, currentTemp);
    Blynk.virtualWrite(VPIN_HUMIDITY, currentHumidity);
  } else {
    Serial.println("[WARN] Failed to read from DHT11 sensor.");
  }

  if (cycleActive) {
    unsigned long elapsed = millis() - cycleStartMillis;
    if (elapsed >= CYCLE_DURATION_MS) {
      cycleActive = false;
      cycleCompletedAlertShown = true;
      Serial.println("[CYCLE] Cooling Cycle Complete!");

      Blynk.virtualWrite(VPIN_TIMER, "00:00:00");
      Blynk.virtualWrite(VPIN_STATUS, "Cooling Cycle Complete");
      Blynk.logEvent("cycle_complete", "Smart Cold Storage: 3-hour cooling cycle complete.");
    } else {
      String remStr = formatRemainingTime(CYCLE_DURATION_MS - elapsed);
      Blynk.virtualWrite(VPIN_TIMER, remStr);
    }
  }

  updateDisplay();
}

void handlePushButton() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        startCoolingCycle();
        updateDisplay();
      }
    }
  }

  lastButtonReading = reading;
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  dht.begin();

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Cold Box");
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi");

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

  lcd.clear();
  blynkTimer.setInterval(SENSOR_INTERVAL_MS, readAndSyncSensors);
  readAndSyncSensors();
}

void loop() {
  Blynk.run();
  blynkTimer.run();
  handlePushButton();
}