#ifndef G_METER_H
#define G_METER_H

#include <math.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "gauge.h"
#include "config.h"

class GMeter : public Gauge {
public:
  GMeter(TFT_eSPI* display) : 
  Gauge(display),
  mpu(),
  outline(display),
  point(display),
  eraser(display),
  minValue(-2),
  maxValue(2),
  oldX(0),
  oldY(0) {}

  void initialize() override {
    display->fillScreen(DISPLAY_BG_COLOR);

    // First create sprite
    outline.setColorDepth(1);
    outline.createSprite(DISPLAY_HEIGHT, DISPLAY_HEIGHT);
    outline.fillSprite(DISPLAY_BG_COLOR);
    int radius = DISPLAY_HEIGHT / 2;
    outline.drawSmoothArc(radius, radius, radius - 10, radius - 12, 0, 360, TFT_WHITE, DISPLAY_BG_COLOR);
    outline.drawLine(0, radius, 10, radius, TFT_WHITE);
    outline.drawLine(DISPLAY_HEIGHT, radius, DISPLAY_HEIGHT - 10, radius, TFT_WHITE);
    outline.drawLine(radius, 0, radius, 10, TFT_WHITE);
    outline.drawLine(radius, DISPLAY_HEIGHT, radius, DISPLAY_HEIGHT - 10, TFT_WHITE);

    point.setColorDepth(8);
    point.createSprite(10, 10);
    point.fillCircle(5, 5, 5, TFT_RED);

    eraser.setColorDepth(1);
    eraser.createSprite(10, 10);

    outline.pushSprite(40, 0);
    point.pushSprite(DISPLAY_CENTER_X, DISPLAY_CENTER_Y);

    // Initilize MPU
    mpu.begin();
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  }

  void render(double val) override {
    renderGMeter();
  }

private:
  Adafruit_MPU6050 mpu;
  TFT_eSprite outline, point, eraser;
  int minValue, maxValue, oldX, oldY;

  void renderGMeter() {
    // First get new readings (X: Forward, Y: Lateral)
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    Serial.println("GX: " + String(accel.acceleration.x));
    Serial.println("GY: " + String(accel.acceleration.y));
    Serial.println("GZ: " + String(accel.acceleration.z));

    // Then calculate where they are on the plot
    int posY = DISPLAY_CENTER_Y / 2;
    posY += (DISPLAY_HEIGHT / 2) * (accel.acceleration.x / 2);

    // Use y for x position ( <-- y direction --> )
    int posX = DISPLAY_CENTER_X / 2;
    posX += (DISPLAY_HEIGHT / 2) * (accel.acceleration.y / 2);
    
    // Adjust for radius of point
    posX -= 5;
    posY -= 5;

    eraser.pushSprite(oldX, oldY);
    outline.pushSprite(40, 0);
    point.pushSprite(posX, posY);
    oldX = posX;
    oldY = posY;
  }
}



#endif