#ifndef G_METER_H
#define G_METER_H

#include "gauge.h"
#include "config.h"

class GMeter : public Gauge {
public:
  GMeter(TFT_eSPI* display) : 
    Gauge(display),
    mpu(),
    combined(display),
    history(display),
    maxX(display),
    maxY(display),
    minX(display),
    minY(display),
    minValueX(0.0),
    minValueY(0.0),
    maxValueX(0.0),
    maxValueY(0.0),
    minValue(-2),
    maxValue(2),
    oldGForceX(0.0),
    oldGForceY(0.0),
    oldX(0),
    oldY(0) {}

  void initialize() override {
    display->fillScreen(DISPLAY_BG_COLOR);

    // Calculate gauge center and outline position
    int gaugeCenterX = DISPLAY_CENTER_X;
    int gaugeCenterY = DISPLAY_CENTER_Y;
    int outlineX = gaugeCenterX - GMETER_RADIUS;
    int outlineY = gaugeCenterY - GMETER_RADIUS;

    // Combined sprite for outline and point
    combined.setColorDepth(8);
    combined.createSprite(GMETER_WIDTH + 2, GMETER_HEIGHT + 2);
    combined.fillSprite(TFT_TRANSPARENT);

    // Create history sprite
    history.setColorDepth(8);
    history.createSprite(GMETER_WIDTH + 2, GMETER_HEIGHT + 2);
    history.fillSprite(TFT_TRANSPARENT);

    // Draw initial outline and point
    drawOutline();
    combined.fillCircle(GMETER_RADIUS, GMETER_RADIUS, GMETER_POINT_RADIUS, GMETER_POINT_COLOR);

    // Create sprites for min and max values
    createTextSprite(minX, "0.00");
    createTextSprite(maxX, "0.00");
    createTextSprite(minY, "0.00");
    createTextSprite(maxY, "0.00");

    // Push combined sprite and initial text sprites
    combined.pushSprite(outlineX, outlineY);
    pushCenteredSprite(minX, outlineX + GMETER_WIDTH + GMETER_TEXT_OFFSET_X, gaugeCenterY);  // Right
    pushCenteredSprite(maxX, outlineX - GMETER_TEXT_OFFSET_X, gaugeCenterY);  // Left
    pushCenteredSprite(minY, gaugeCenterX, outlineY - GMETER_TEXT_OFFSET_Y);  // Top
    pushCenteredSprite(maxY, gaugeCenterX, outlineY + GMETER_HEIGHT + GMETER_TEXT_OFFSET_Y);  // Bottom

    // Initialize MPU
    mpu.begin();
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

    // Set initial point position
    oldX = gaugeCenterX - GMETER_POINT_RADIUS;
    oldY = gaugeCenterY - GMETER_POINT_RADIUS;
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
  TFT_eSprite combined, history, maxX, maxY, minX, minY;
  int minValue, maxValue, oldX, oldY;
  double oldGForceX, oldGForceY, minValueX, maxValueX, minValueY, maxValueY;

  void drawOutline() {
    combined.fillSprite(TFT_TRANSPARENT);
    combined.drawSmoothCircle(GMETER_RADIUS, GMETER_RADIUS, GMETER_RADIUS, GMETER_OUTLINE_COLOR, TFT_TRANSPARENT);
    combined.drawSmoothCircle(GMETER_RADIUS, GMETER_RADIUS, 3 * (GMETER_RADIUS / 4), GMETER_OUTLINE_COLOR, TFT_TRANSPARENT);
    combined.drawSmoothCircle(GMETER_RADIUS, GMETER_RADIUS, GMETER_RADIUS / 2, GMETER_OUTLINE_COLOR, TFT_TRANSPARENT);
    combined.drawSmoothCircle(GMETER_RADIUS, GMETER_RADIUS, GMETER_RADIUS / 4, GMETER_OUTLINE_COLOR, TFT_TRANSPARENT);
    combined.drawLine(0, GMETER_RADIUS, GMETER_WIDTH, GMETER_RADIUS, GMETER_OUTLINE_COLOR);  // Horizontal line
    combined.drawLine(GMETER_RADIUS, 0, GMETER_RADIUS, GMETER_HEIGHT, GMETER_OUTLINE_COLOR); // Vertical line
  }

  void createTextSprite(TFT_eSprite& sprite, const char* text) {
    sprite.setColorDepth(8);
    sprite.setTextFont(GMETER_TEXT_FONT);
    sprite.setTextSize(GMETER_TEXT_SIZE);
    sprite.setTextColor(GMETER_TEXT_COLOR, TFT_TRANSPARENT);
    int textWidth = sprite.textWidth("2.00"); // Fixed size for consistency
    int textHeight = sprite.fontHeight();
    sprite.createSprite(textWidth + 10, textHeight + 10);
    sprite.fillSprite(TFT_TRANSPARENT);
    sprite.setCursor(5, 5);
    sprite.println(text);
  }

  void pushCenteredSprite(TFT_eSprite& sprite, int x, int y) {
    int spriteWidth = sprite.width();
    int spriteHeight = sprite.height();
    sprite.pushSprite(x - spriteWidth / 2, y - spriteHeight / 2);
  }

  void renderGMeter() {
    // Get new readings (X: Forward, Y: Lateral)
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    double gForceX = accel.acceleration.y / 9.80665;
    double gForceY = accel.acceleration.x / 9.80665;

    // Calculate position on the plot
    int gaugeCenterX = DISPLAY_CENTER_X;
    int gaugeCenterY = DISPLAY_CENTER_Y;
    int posX = gaugeCenterX - (GMETER_RADIUS * gForceX / 2);
    int posY = gaugeCenterY - (GMETER_RADIUS * gForceY / 2);

    // Only redraw if position changes significantly
    if (abs(posX - oldX) > 1 || abs(posY - oldY) > 1) {
      // Prerender outline and point into combined sprite
      drawOutline();
      combined.fillCircle(posX - (gaugeCenterX - GMETER_RADIUS), posY - (gaugeCenterY - GMETER_RADIUS), GMETER_POINT_RADIUS, GMETER_POINT_COLOR);

      // Store point in history sprite
      history.fillCircle(posX - (gaugeCenterX - GMETER_RADIUS), posY - (gaugeCenterY - GMETER_RADIUS), GMETER_POINT_RADIUS - 3, GMETER_HISTORY_COLOR);

      // Push combined sprite over history
      history.pushSprite(gaugeCenterX - GMETER_RADIUS, gaugeCenterY - GMETER_RADIUS, TFT_TRANSPARENT);
      combined.pushSprite(gaugeCenterX - GMETER_RADIUS, gaugeCenterY - GMETER_RADIUS, TFT_TRANSPARENT);

      // Update old positions
      oldX = posX;
      oldY = posY;
    }

    // Update min and max values
    updateMinMaxValues(gForceX, gForceY);
  }

  void updateMinMaxValues(double gForceX, double gForceY) {
    int outlineX = DISPLAY_CENTER_X - GMETER_RADIUS;
    int outlineY = DISPLAY_CENTER_Y - GMETER_RADIUS;

    // Update min and max for X
    if (gForceX > maxValueX) {
      maxValueX = gForceX;
      updateTextSprite(maxX, String(maxValueX, 2));
      pushCenteredSprite(maxX, outlineX - GMETER_TEXT_OFFSET_X, DISPLAY_CENTER_Y);
    }
    if (gForceX < minValueX) {
      minValueX = gForceX;
      updateTextSprite(minX, String(abs(minValueX), 2));
      pushCenteredSprite(minX, outlineX + GMETER_WIDTH + GMETER_TEXT_OFFSET_X, DISPLAY_CENTER_Y);
    }
    // Update min and max for Y
    if (gForceY > maxValueY) {
      maxValueY = gForceY;
      updateTextSprite(maxY, String(maxValueY, 2));
      pushCenteredSprite(maxY, DISPLAY_CENTER_X, outlineY - GMETER_TEXT_OFFSET_Y);
    }
    if (gForceY < minValueY) {
      minValueY = gForceY;
      updateTextSprite(minY, String(abs(minValueY), 2));
      pushCenteredSprite(minY, DISPLAY_CENTER_X, outlineY + GMETER_HEIGHT + GMETER_TEXT_OFFSET_Y);
    }
  }

  void updateTextSprite(TFT_eSprite& sprite, String text) {
    sprite.fillSprite(TFT_TRANSPARENT);
    sprite.setCursor(5, 5);
    sprite.println(text);
  }
};

#endif