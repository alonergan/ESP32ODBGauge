#include <TFT_eSPI.h>
#include "bluetooth.h"
#include "commands.h"
#include "needle_gauge.h"
#include "g_meter.h"
#include "acceleration_meter.h"

#define TEST_MODE true

TFT_eSPI display = TFT_eSPI();
Gauge* currentGauge;
Commands commands;
int selectedGauge = 2;                      // [0: RPM, 1: Boost, 2: Torque, 3: G-Meter, 4: Acceleration Meter]        

long lastSensorUpdate = 0;
long lastAnimationFrame = 0;
const unsigned long animationInterval = 15; // Target ~66 FPS
const unsigned long sensorInterval = 80;   // Query every 80 ms (12.5Hz)
double targetValue = 0.0;
double displayedValue = targetValue;
double alpha = 0.2;

unsigned long fpsStartTime = 0;
unsigned long querySum = 0;
unsigned long frameSum = 0;
int queryCount = 0;
int frameCount = 0;
int fpsFrameCount = 0;
float fps = 0;

void setup() {
    Serial.begin(115200);

    // Initialize buttons
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);

    // Initialize display
    display.init();
    display.setRotation(3);
    display.fillScreen(DISPLAY_BG_COLOR);

    // Intialize gauge - default to RPM
    currentGauge = new NeedleGauge(&display, selectedGauge); // RPM gauge
    currentGauge->initialize();
    currentGauge->displayStats(0, 0, 0);

    // Attempt initial connection of OBD
    if(!TEST_MODE && connectToOBD()) {
        // Success, initialize
        display.fillRect(300, 0, 20, 20, TFT_GREEN);
    } else {
        display.fillRect(300, 0, 20, 20, TFT_RED);    
    }

    // Set animation params
    lastSensorUpdate = millis();
    lastAnimationFrame = millis();
    fpsStartTime = millis();
}

void loop() {
    // Check for button input and switch screens
    if (digitalRead(BUTTON_PIN) == HIGH) {
        selectedGauge++;
        if (selectedGauge > 4) {
            selectedGauge = 0;
        }

        // If gauge is 3 then show G-Meter
        delete currentGauge;
        if (selectedGauge == 4) {
            currentGauge = new AccelerationMeter(&display);
            currentGauge->initialize();
        }
        else if (selectedGauge == 3) {
            currentGauge = new GMeter(&display);
            currentGauge->initialize();
        } else {
            currentGauge = new NeedleGauge(&display, selectedGauge);
            currentGauge->initialize();
        }
        
        // Loop to hold while button is pressed so other code doesnt execute
        while(digitalRead(BUTTON_PIN) == HIGH) {}

        lastSensorUpdate = millis();
        lastAnimationFrame = millis();
    }

    // If GMeter dont bother trying to run queries
    if (selectedGauge == 4) {
        for (double i = 0.0; i < 70.00; i += 1.0) {
            unsigned long start = millis();
            currentGauge->render(i);
            unsigned long end = millis();
            Serial.println(String(end - start));
            frameSum += (end - start);
            frameCount++;
            fpsFrameCount++;
            delay(62); // Target 3.8 0-60 for testing

            // Display stats every 200 frames
            if (fpsFrameCount >= 50) {
                unsigned long fpsEndTime = millis();
                float elapsed = (fpsEndTime - fpsStartTime) / 1000.0;
                fps = fpsFrameCount / elapsed;
                fpsFrameCount = 0;
                fpsStartTime = millis();
                double frameAvg = frameCount > 0 ? (frameSum / (double)frameCount) : 0;
                currentGauge->displayStats(fps, frameAvg, -1.0);
                querySum = 0;
                frameSum = 0;
                queryCount = 0;
                frameCount = 0;
            }
        }

        while (true) {
            
        }
    }
}