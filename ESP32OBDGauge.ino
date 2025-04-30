#include <TFT_eSPI.h>
#include "bluetooth.h"
#include "commands.h"
#include "needle_gauge.h"
#include "g_meter.h"
#include "command_test.h"

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

    currentGauge = new CommandTest(&display);
    currentGauge->initialize();

    // Attempt initial connection of OBD
    if(!TEST_MODE && connectToOBD()) {
        // Success, initialize
        //commands.initializeOBD();
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
    // No animation interval, query and update RPM sprite syncronously
    if (connected) {
        unsigned long start = millis();
        Serial.println("Querying OBD...");
        double reading = commands.getReading(0);
        unsigned long end = millis();
        unsigned long duration = millis() - start;
        Serial.println("Response time: " + String(duration) + " | Value: " + String(reading));
        querySum += duration;
        queryCount++;

        start = millis();
        currentGauge->render(reading);
        end = millis();
        frameSum += (end - start);
        frameCount++;
        fpsFrameCount++;
        
        // Display stats every 100 frames
        if (fpsFrameCount >= 100) {
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
    else {
        // Attempt reconnection
        Serial.println("Attempting reconnection...");
        if(connectToOBD()) {
            // Success, reinitialize
            //commands.initializeOBD();
            display.fillRect(300, 0, 20, 20, TFT_GREEN);
        } else {
            Serial.println("Connection failed...");
            display.fillRect(300, 0, 20, 20, TFT_RED);
        }
    }
}