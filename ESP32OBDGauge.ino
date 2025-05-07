#include <TFT_eSPI.h>
#include "touch.h"
#include "bluetooth.h"
#include "commands.h"
#include "needle_gauge.h"
#include "g_meter.h"
#include "acceleration_meter.h"
#include "options_screen.h"
#include "config.h"

bool TESTMODE = true;

TFT_eSPI display = TFT_eSPI();
Gauge* gauges[5];
int currentGauge = 0;
TaskHandle_t dataTaskHandle;
SemaphoreHandle_t gaugeMutex;
unsigned long lastTouchTime = 0;
bool obdConnected = false;
OptionsScreen* optionsScreen = nullptr;
bool inOptionsScreen = false;

void setup() {
    Serial.begin(115200);
    display.begin();
    display.setRotation(1); // Adjust as needed
    touch_init(DISPLAY_WIDTH, DISPLAY_HEIGHT, display.getRotation());

    // Display OBD connecting message
    display.fillScreen(DISPLAY_BG_COLOR);
    display.setCursor(50, 100);
    display.setTextSize(2);
    display.println("Connecting to OBD...");
    delay(500); // Brief delay for visibility

    // Attempt OBD connection
    if (!TESTMODE) {
      obdConnected = connectToOBD();
    } else {
      obdConnected = true;
    }

    // Show splash screen
    display.fillScreen(DISPLAY_BG_COLOR);
    display.setCursor(50, 100);
    display.setTextSize(2);
    display.println("Car Gauge Starting...");
    delay(1000);

    // Initialize mutex
    gaugeMutex = xSemaphoreCreateMutex();

    // Create all gauges
    gauges[0] = new NeedleGauge(&display, 0); // RPM
    gauges[1] = new NeedleGauge(&display, 1); // Boost
    gauges[2] = new NeedleGauge(&display, 2); // Torque
    gauges[3] = new GMeter(&display);
    gauges[4] = new AccelerationMeter(&display);

    // Initialize first gauge (RPM or GMeter if no OBD)
    gauges[obdConnected ? 0 : 3]->initialize();
    if (!obdConnected) {
        currentGauge = 3; // Default to GMeter if OBD fails
    }

    // Start data fetching task on core 0
    xTaskCreatePinnedToCore(
        dataFetchingTask,
        "DataFetching",
        10000,
        NULL,
        1,
        &dataTaskHandle,
        0
    );
}

void dataFetchingTask(void* parameter) {
    Commands commands;
    Adafruit_MPU6050 mpu;
    mpu.begin();
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
    unsigned long lastReconnectAttempt = 0;
    const unsigned long RECONNECT_INTERVAL = 5000; // 5 seconds

    while (true) {
        int gaugeIndex;
        xSemaphoreTake(gaugeMutex, portMAX_DELAY);
        gaugeIndex = currentGauge;
        xSemaphoreGive(gaugeMutex);

        Gauge* gauge = gauges[gaugeIndex];
        switch (gauge->getType()) {
            case Gauge::NEEDLE_GAUGE:
                if (obdConnected) {
                    double reading = commands.getReading(gaugeIndex);
                    NeedleGauge* ng = static_cast<NeedleGauge*>(gauge);
                    ng->setReading(reading);
                    vTaskDelay(100 / portTICK_PERIOD_MS); // 10Hz
                } else {
                    if (millis() - lastReconnectAttempt > RECONNECT_INTERVAL) {
                        obdConnected = reconnectToOBD();
                        lastReconnectAttempt = millis();
                    }
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
                break;
            case Gauge::G_METER:
                {
                    sensors_event_t accel, gyro, temp;
                    mpu.getEvent(&accel, &gyro, &temp);
                    GMeter* gm = static_cast<GMeter*>(gauge);
                    gm->setAccelData(&accel, &gyro, &temp);
                    vTaskDelay(20 / portTICK_PERIOD_MS); // 50Hz
                }
                break;
            case Gauge::ACCELERATION_METER:
                if (obdConnected) {
                    double speed = commands.getReading(3); // Speed for AccelerationMeter
                    AccelerationMeter* am = static_cast<AccelerationMeter*>(gauge);
                    am->setSpeed(speed);
                    vTaskDelay(100 / portTICK_PERIOD_MS); // 10Hz
                } else {
                    if (millis() - lastReconnectAttempt > RECONNECT_INTERVAL) {
                        obdConnected = reconnectToOBD();
                        lastReconnectAttempt = millis();
                    }
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
                break;
        }
    }
}

void loop() {
    static bool wasTouched = false;
    static unsigned long touchStartTime = 0;
    const unsigned long LONG_PRESS_THRESHOLD = 1000; // 1 second
    const unsigned long DEBOUNCE_MS = 50; // Debounce period

    // Handle touch input
    if (touch_touched()) {
        if (millis() - lastTouchTime > DEBOUNCE_MS) {
            lastTouchTime = millis();
            if (!wasTouched) {
                wasTouched = true;
                touchStartTime = millis();
            }
            if (inOptionsScreen) {
                // Process touch immediately in options screen
                if (touch_last_x >= 0 && touch_last_y >= 0) {
                    Serial.print("Touch at: "); Serial.print(touch_last_x); Serial.print(", "); Serial.println(touch_last_y);
                    if (!optionsScreen->handleTouch(touch_last_x, touch_last_y)) {
                        exitOptions();
                    }
                }
            } else if (millis() - touchStartTime > LONG_PRESS_THRESHOLD) {
                showOptions();
                wasTouched = false;
            }
        }
    } else if (wasTouched && !touch_touched()) {
        wasTouched = false;
        if (!inOptionsScreen && millis() - touchStartTime < LONG_PRESS_THRESHOLD) {
            switchToNextGauge();
        }
    }

    // Render
    if (!inOptionsScreen) {
        xSemaphoreTake(gaugeMutex, portMAX_DELAY);
        gauges[currentGauge]->render(0.0);
        xSemaphoreGive(gaugeMutex);
    }

    // Limit to ~30 FPS
    delay(33);
}

void switchToNextGauge() {
    xSemaphoreTake(gaugeMutex, portMAX_DELAY);
    if (obdConnected) {
        currentGauge = (currentGauge + 1) % 5;
    } else {
        currentGauge = 3; // Stay on GMeter if OBD not connected
    }
    gauges[currentGauge]->initialize();
    xSemaphoreGive(gaugeMutex);
}

void showOptions() {
    inOptionsScreen = true;
    optionsScreen = new OptionsScreen(&display, gauges, 5);
    optionsScreen->initialize();
}

void exitOptions() {
    if (optionsScreen) {
        delete optionsScreen;
        optionsScreen = nullptr;
    }
    inOptionsScreen = false;
    xSemaphoreTake(gaugeMutex, portMAX_DELAY);
    gauges[currentGauge]->initialize();
    xSemaphoreGive(gaugeMutex);
}