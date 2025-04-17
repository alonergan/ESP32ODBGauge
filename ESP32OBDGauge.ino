#include "needle_gauge.h"
#include "bluetooth.h"
#include "commands.h"

TFT_eSPI display = TFT_eSPI();
Gauge* currentGauge;
Commands commands;
int selectedGauge = 0;

long lastSensorUpdate = 0;
long lastAnimationFrame = 0;
const unsigned long animationInterval = 15; // Target ~66 FPS
const unsigned long sensorInterval = 100;  // Query every 100 ms (10Hz)
double targetValue = 0.0;
double displayedValue = targetValue;
double alpha = 0.25;

unsigned long fpsStartTime = 0;
unsigned long querySum = 0;
unsigned long frameSum = 0;
int queryCount = 0;
int frameCount = 0;
int fpsFrameCount = 0;
float fps = 0;

void setup() {
    Serial.begin(115200);

    // Initialize display
    display.init();
    display.setRotation(1);
    display.fillScreen(DISPLAY_BG_COLOR);

    // Add splash screen startup - TODO

    // Intialize gauge - default to RPM
    currentGauge = new NeedleGauge(&display, 0); // RPM gauge
    currentGauge->initialize();
    currentGauge->displayStats(0, 0, 0);

    // Attempt initial connection of OBD
    if(connectToOBD()) {
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
}

void loop() {
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
            double reading = commands.getRPM();
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
        Serial.println("Attempting reconnection...");
        if(connectToOBD()) {
            // Success, reinitialize
            commands.initializeOBD();
            display.fillRect(300, 0, 20, 20, TFT_GREEN);
        } else {
            display.fillRect(300, 0, 20, 20, TFT_RED);
        }
    }
}