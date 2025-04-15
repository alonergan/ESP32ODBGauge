#include "bluetooth.h"
#include "needle_gauge.h"

// Initializes the display and BLE, manages the main loop to send OBD commands, processes responses, and renders the current gauge.
TFT_eSPI display = TFT_eSPI();
Gauge* currentGauge;

void setup() {
  // Initialize display and set initial gauge
  display.init();
  display.setRotation(1);
  display.fillScreen(DISPLAY_BG_COLOR);
  currentGauge = new NeedleGauge(&display, 0, 200, "km/h");

  // Initialize BLE device
  BLEDevice::init("ESP32_OBD");
  if (!connectToOBD()) {
    // Handle connection failure (e.g., display error)
    display.setCursor(0, 0);
    display.print("BLE Connection Failed");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  while (true) {
    currentGauge->render(0.0);
  }
}
