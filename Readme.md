# Smart Cold Storage System

A compact, portable, and IoT-enabled thermoelectric cold storage prototype built around **ON/OFF feedback process control** and **ESP32 supervisory monitoring**. Developed for **21EIC302J – Process Control** at SRM Institute of Science & Technology.

## System Overview

- **Thermoelectric Cooling:** Uses a **TEC-12703 Peltier module** paired with a water-cooling block, 5V circulation pump, heatsink, and 12V cooling fans to cool a 5-liter insulated storage chamber (lined with thermocol and aluminum foil) from ~28 °C down to **10–12 °C** ($\pm 1\text{ }^\circ\text{C}$ stability).
- **Feedback Process Control:** A **W1209 temperature controller** acts as an independent ON/OFF relay thermostat with hysteresis to maintain the target temperature setpoint.
- **Supervisory Control & IoT:** An **ESP32** reads temperature and humidity from a **DHT11** sensor, displays live metrics and a **3-hour operation timer** on a **$16\times2$ I²C LCD**, and syncs telemetry and end-of-cycle alerts with the **Blynk IoT** mobile app.
- **Power Regulation:** Dual buck converter stage using **XL4016** (high-current 12V rail for the Peltier module) and **LM2596** (step-down rail for ESP32, sensors, and logic).

## Hardware Pinout (ESP32)

| Component | ESP32 Pin | Notes |
| :--- | :--- | :--- |
| **DHT11 Sensor** | `GPIO 4` | Single-wire digital interface |
| **$16\times2$ I²C LCD** | `GPIO 21` (SDA), `GPIO 22` (SCL) | Default I²C address `0x27` (or `0x3F`) |
| **Cycle Start Push Button** | `GPIO 14` | Wired to `GND` (`INPUT_PULLUP` enabled) |
| **W1209 Thermostat** | Independent 12V Relay | Switches TEC-12703 Peltier module |

## Repository Structure

```text
smart-cold-storage/
├── README.md
├── .gitignore
├── LICENSE
└── esp32/
    ├── config.example.h
    └── smart_cold_storage.ino
```

## Setup & Usage

1. Install **Arduino IDE** with the **ESP32 board package**.
2. Install the required libraries via Library Manager:
   - `DHT sensor library` (by Adafruit)
   - `Adafruit Unified Sensor`
   - `LiquidCrystal I2C` (by Frank de Brabander)
   - `Blynk` (by Volodymyr Shymanskyy)
3. Copy `esp32/config.example.h` to `esp32/config.h` and fill in your **Blynk Template ID**, **Auth Token**, and **Wi-Fi credentials**.
4. Flash `esp32/smart_cold_storage.ino` to the ESP32.
5. Press the tactile push button on `GPIO 14` to start a **3-hour cooling cycle**; the countdown appears on both the LCD and Blynk dashboard, triggering an alert when complete.

## Team & Guidance

- **Team:** Srija P, Sarveshwaran V, Hisham B
- **Guide:** Dr. Sharanya S, Dept. of Electronics and Instrumentation Engineering, SRMIST