#ifndef G_METER_H
#define G_METER_H

#include "gauge.h"

class GMeter : public Gauge {
public:
  GMeter(TFT_eSPI* display) : 
  Gauge(display),
  mpu(),
  outline(display),
  point(display),
  eraser(display),
  maxValueX(display),
  maxValueY(display),
  minValue(-2),
  maxValue(2),
  oldGForceX(0.0),
  oldGForceY(0.0),
  oldX(0),
  oldY(0) {}

  void initialize() override {
    display->fillScreen(DISPLAY_BG_COLOR);

    // First outline
    outline.setColorDepth(1);
    outline.createSprite(DISPLAY_HEIGHT, DISPLAY_HEIGHT);
    outline.fillSprite(DISPLAY_BG_COLOR);
    int radius = DISPLAY_HEIGHT / 2;
    outline.drawSmoothArc(radius, radius, radius - 10, radius - 12, 0, 360, TFT_WHITE, DISPLAY_BG_COLOR);
    outline.drawLine(0, radius, 10, radius, TFT_WHITE);
    outline.drawLine(DISPLAY_HEIGHT, radius, DISPLAY_HEIGHT - 10, radius, TFT_WHITE);
    outline.drawLine(radius, 0, radius, 10, TFT_WHITE);
    outline.drawLine(radius, DISPLAY_HEIGHT, radius, DISPLAY_HEIGHT - 10, TFT_WHITE);

    // Create point to plot
    point.setColorDepth(8);
    point.createSprite(13, 13);
    point.fillCircle(5, 5, 5, TFT_RED);
    eraser.setColorDepth(1);
    eraser.createSprite(13, 13);

    // Create max value icons
    maxValueX.createSprite(50, 50);
    maxValueY.createSprite(50, 50);
    Serial.println("CREATED MMAX VAL SPRITES");
    maxValueX.drawRect(0, 0, 40, 40, TFT_WHITE);
    maxValueY.drawRect(0, 0, 40, 40, TFT_WHITE);
    maxValueX.setTextFont(1);
    maxValueY.setTextFont(1);
    maxValueX.setTextSize(1);
    maxValueY.setTextSize(1);
    maxValueX.setTextColor(TFT_WHITE);
    maxValueY.setTextColor(TFT_WHITE);
    maxValueX.setCursor(3, 3);
    maxValueY.setCursor(3, 3);
    Serial.println("MADE IT HERE");
    maxValueX.println("X-MAX");
    maxValueX.println("0.0");
    maxValueY.println("Y-MAX");
    maxValueY.println("0.0");
    Serial.println("FINISHED MMAX VAL SPRITES");

    // Push sprites
    outline.pushSprite(40, 0);
    point.pushSprite(DISPLAY_CENTER_X, DISPLAY_CENTER_Y);
    maxValueX.pushSprite(0, 0);
    maxValueY.pushSprite(280, 0);

    // Initilize MPU
    mpu.begin();
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  }

  void render(double val) override {
    renderGMeter();
  }

  void displayStats(float fps, double frameAvg, double queryAvg) override {
    display->setTextColor(TFT_WHITE, DISPLAY_BG_COLOR);
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->printf("FPS: %.1f\nFrame: %.1f ms\nQuery: %.1f ms", fps, frameAvg, queryAvg);
  }

private:
  Adafruit_MPU6050 mpu;
  TFT_eSprite outline, point, eraser, maxValueX, maxValueY;
  int minValue, maxValue, oldX, oldY;
  double oldGForceX, oldGForceY;

  void renderGMeter() {
    // First get new readings (X: Forward, Y: Lateral)
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    double gForceX = accel.acceleration.y / 9.80665;
    double gForceY = accel.acceleration.x / 9.80665;

    Serial.println("------------------------------------");
    Serial.println("GforceX: " + String(gForceX));
    Serial.println("GforceY: " + String(gForceY));
    Serial.println("------------------------------------");

    // Then calculate where they are on the plot  
    int posY = DISPLAY_CENTER_Y;
    posY += (DISPLAY_HEIGHT / 2) * (gForceY / 2);
    int posX = DISPLAY_CENTER_X;
    posX += (DISPLAY_HEIGHT / 2) * (gForceX / 2);
    
    // Adjust for radius of point
    posX -= 5;
    posY -= 5;

    // Dont draw if in the same position
    if (posX == oldX && posY == oldY) {
      return;
    }

    // Redraw outline if sprite overlapped
    if ((gForceX >= 1.9 || gForceX <= -1.9) || (gForceY >= 1.9 || gForceY <= 1.9)) {
      outline.pushSprite(40, 0);
    }

    // Update maxValues if needed
    if (abs(gForceY) > oldGForceY) {
        maxValueY.fillSprite(TFT_BLACK);
        maxValueY.drawRect(0, 0, 40, 40, TFT_WHITE);
        maxValueY.setCursor(3, 3);
        maxValueY.printf("Y-MAX\n");
        maxValueY.println(String(gForceY));
        oldGForceY = abs(gForceY);
    }

    if (abs(gForceX) > oldGForceX) {
        maxValueX.fillSprite(TFT_BLACK);
        maxValueX.drawRect(0, 0, 40, 40, TFT_WHITE);
        maxValueX.setCursor(3, 3);
        maxValueX.println("X-MAX\n");
        maxValueX.println(String(gForceX));
        oldGForceX = abs(gForceX);
    }

    // Erase point and draw new one
    eraser.pushSprite(oldX, oldY);
    point.pushSprite(posX, posY);
    oldX = posX;
    oldY = posY;
  }
};

#endif


