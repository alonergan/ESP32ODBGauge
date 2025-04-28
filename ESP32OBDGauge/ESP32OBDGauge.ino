#include <TFT_eSPI.h>
#include "bluetooth.h"
#include "commands.h"
#include "needle_gauge.h"
#include "g_meter.h"

#define TEST_MODE true

TFT_eSPI display = TFT_eSPI();
Gauge* currentGauge;
Commands commands;
int selectedGauge = 2;                      // [0: RPM, 1: Boost, 2: Torque, 3: G-Meter]        

long lastSensorUpdate = 0;
long lastAnimationFrame = 0;
const unsigned long animationInterval = 15; // Target ~66 FPS
const unsigned long sensorInterval = 100;   // Query every 100 ms (10Hz)
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

    // Add splash screen startup - TODO

    // Intialize gauge - default to RPM
    currentGauge = new NeedleGauge(&display, selectedGauge); // RPM gauge
    currentGauge->initialize();
    currentGauge->displayStats(0, 0, 0);

    // Attempt initial connection of OBD
    if(!TEST_MODE && connectToOBD()) {
        // Success, initialize
        commands.initializeOBD();
        display.fillRect(300, 0, 20, 20, TFT_GREEN);
    } else {
        display.fillRect(300, 0, 20, 20, TFT_RED);    
    }

    // Set animation params
    lastSensorUpdate = millis();
    lastAnimationFrame = millis();
    fpsStartTime = millis();

    Serial.print("Initial Button State: ");
    Serial.println(digitalRead(BUTTON_PIN));
}

void loop() {
    // Check for button input and switch screens
    if (digitalRead(BUTTON_PIN) == HIGH) {
        Serial.println(digitalRead(BUTTON_PIN));
        Serial.println("Read Button State");
        selectedGauge++;
        if (selectedGauge > 3) {
            selectedGauge = 0;
        }

        // If gauge is 3 then show G-Meter
        delete currentGauge;
        if (selectedGauge == 3) {
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
    if (selectedGauge == 3) {
        // Read meter
        currentGauge->render(0.0);
        delay(animationInterval); // Tie to animation target fps
        return;
    }

    if (connected) {
        unsigned long now = millis();
        // Render as many frames as needed
        while (now - lastAnimationFrame >= animationInterval) {
            displayedValue = displayedValue + alpha * (targetValue - displayedValue);
            unsigned long start = millis();
            currentGauge->render(displayedValue);
            unsigned long end = millis();
            frameSum += (end - start);
            frameCount++;
            fpsFrameCount++;
            lastAnimationFrame += animationInterval;

            // Display stats every 200 frames
            if (fpsFrameCount >= 200) {
                unsigned long fpsEndTime = millis();
                float elapsed = (fpsEndTime - fpsStartTime) / 1000.0;
                fps = fpsFrameCount / elapsed;
                fpsFrameCount = 0;
                fpsStartTime = millis();
                double frameAvg = frameCount > 0 ? (frameSum / (double)frameCount) : 0;
                double queryAvg = queryCount > 0 ? (querySum / (double)queryCount) : 0;
                currentGauge->displayStats(fps, frameAvg, queryAvg);
                querySum = 0;
                frameSum = 0;
                queryCount = 0;
                frameCount = 0;
            }
        }

        // Handle sensor query if needed
        if (now - lastSensorUpdate >= sensorInterval) {
            unsigned long start = millis();
            double reading = commands.getReading(selectedGauge);
            unsigned long end = millis();
            unsigned long duration = millis() - start;
            querySum += duration;
            queryCount++;
            if (reading >= 0.0) {
                targetValue = reading;
            }
            lastSensorUpdate = now;
        }
    } else {
        // Attempt reconnection
        /**
        Serial.println("Attempting reconnection...");
        if(connectToOBD()) {
            // Success, reinitialize
            commands.initializeOBD();
            display.fillRect(300, 0, 20, 20, TFT_GREEN);
        } else {
            display.fillRect(300, 0, 20, 20, TFT_RED);
        }
        */
    }
}