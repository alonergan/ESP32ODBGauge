#ifndef G_METER_H
#define G_METER_H

#include "gauge.h"

/*
  Notes:

  In Audi software (RS5) the gmeter works as follows:
  1) All values are zero initially
  2) While driving, the real time values are displayed on the gmeter
    2.1) Record maximum coordinate values and graph of previous values
  3) After coming to a stop, the graph of hit gvalues and maximum g values are displayed
  4) When leaving a stop go back to step 2 back to step (2) ** Retain maximum values until engine shutoff **
*/

class GMeter : public Gauge {
public:
  GMeter(TFT_eSPI* display) : 
  Gauge(display),
  mpu(),
  outline(display),
  point(display),
  eraser(display),
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

    // Centering lines for testing
    //display->drawLine(0, DISPLAY_CENTER_Y, DISPLAY_WIDTH, DISPLAY_CENTER_Y, TFT_BLUE);
    //display->drawLine(DISPLAY_CENTER_X, 0, DISPLAY_CENTER_X, DISPLAY_HEIGHT, TFT_BLUE);

    // First outline sprite
    Serial.println("GMETER_CENTER: " + String(GMETER_CENTER));
    outline.setColorDepth(8);
    outline.createSprite(GMETER_WIDTH + 2, GMETER_HEIGHT + 2);
    outline.fillSprite(TFT_TRANSPARENT);
    outline.drawSmoothCircle(GMETER_CENTER, GMETER_CENTER, GMETER_CENTER, TFT_WHITE, TFT_TRANSPARENT);
    outline.drawSmoothCircle(GMETER_CENTER, GMETER_CENTER, 3 * (GMETER_CENTER / 4), TFT_WHITE, TFT_TRANSPARENT);
    outline.drawSmoothCircle(GMETER_CENTER, GMETER_CENTER, GMETER_CENTER / 2, TFT_WHITE, TFT_TRANSPARENT);
    outline.drawSmoothCircle(GMETER_CENTER, GMETER_CENTER, GMETER_CENTER / 4, TFT_WHITE, TFT_TRANSPARENT);
    outline.drawLine(0, GMETER_CENTER, GMETER_WIDTH, GMETER_CENTER, TFT_WHITE);                       // Horizontal line
    outline.drawLine(GMETER_CENTER, 0, GMETER_CENTER, GMETER_HEIGHT, TFT_WHITE);                      // Vertical line

    minX.setColorDepth(8);
    minX.setTextFont(1);
    minX.setTextSize(2);
    minX.setTextColor(TFT_RED);
    minX.setCursor(0, 0);
    minX.createSprite(50, 20);
    minX.println("0.00");

    maxX.setColorDepth(8);
    maxX.setTextFont(1);
    maxX.setTextSize(2);
    maxX.setTextColor(TFT_RED);
    maxX.setCursor(0, 0);
    maxX.createSprite(50, 20);
    maxX.println("0.00");

    minY.setColorDepth(8);
    minY.setTextFont(1);
    minY.setTextSize(2);
    minY.setTextColor(TFT_RED);
    minY.createSprite(55, 20);
    int textWidth = minY.textWidth("0.00");
    int textPadding = (55 - textWidth) / 2;
    minY.setCursor(textPadding, 0);
    minY.println("0.00");    

    maxY.setColorDepth(8);
    maxY.setTextFont(1);
    maxY.setTextSize(2);
    maxY.setTextColor(TFT_RED);
    maxY.createSprite(55, 20);
    maxY.setCursor(textPadding, 0);
    maxY.println("0.00");

    // Create point to plot
    point.setColorDepth(8);
    point.createSprite(11, 11);
    point.fillSprite(TFT_TRANSPARENT);
    point.fillCircle(5, 5, 5, TFT_RED);
    eraser.setColorDepth(1);
    eraser.createSprite(11, 11);

    // Push sprites
    outline.pushSprite((DISPLAY_WIDTH - GMETER_WIDTH) / 2, (GMETER_PADDING / 2));
    point.pushSprite(DISPLAY_CENTER_X, DISPLAY_CENTER_Y);
    minX.pushSprite(10, DISPLAY_CENTER_Y - 6);
    maxX.pushSprite(DISPLAY_WIDTH - 55, DISPLAY_CENTER_Y - 6);
    minY.pushSprite(DISPLAY_CENTER_X - 25, 2);
    maxY.pushSprite(DISPLAY_CENTER_X - 25, DISPLAY_HEIGHT - 20);

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
  TFT_eSprite outline, point, eraser, maxX, maxY, minX, minY;
  int minValue, maxValue, oldX, oldY;
  double oldGForceX, oldGForceY, minValueX, maxValueX, minValueY, maxValueY;

  void renderGMeter() {
    // First get new readings (X: Forward, Y: Lateral)
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    double gForceX = accel.acceleration.y / 9.80665;
    double gForceY = accel.acceleration.x / 9.80665;

    Serial.println("---------------");
    Serial.println("GForceX: " + String(gForceX));
    Serial.println("GForceY: " + String(gForceY));
    Serial.println("---------------");

    // Then calculate where they are on the plot  
    int posY = DISPLAY_CENTER_Y;
    posY += GMETER_CENTER * (gForceY / 2);
    int posX = DISPLAY_CENTER_X;
    posX += GMETER_CENTER * (gForceX / 2);
    
    // Adjust for radius of point
    posX -= 4;
    posY -= 4;

    // Dont draw if in not greater than .03g
    if ((abs(posX - oldX) <= 0.03) && (abs(posY - oldY) <= 0.03)) {
      oldX = posX;
      oldY = posY;
      return;
    }

    // Check value and updated min/max values
    if (gForceX > maxValueX) {
      maxValueX = gForceX;
      maxX.fillSprite(TFT_TRANSPARENT);
      maxX.setCursor(0, 0);
      maxX.println(String(maxValueX));
    }
    else if (gForceX < minValueX) {
      minValueX = gForceX;
      minX.fillSprite(TFT_TRANSPARENT);
      minX.setCursor(0, 0);
      minX.println(String(abs(minValueX)));
    }

    if (gForceY > maxValueY) {
      maxValueY = gForceY;
      maxY.fillSprite(TFT_TRANSPARENT);
      int textWidth = maxY.textWidth(String(maxValueY));
      int textOffset = (55 - textWidth) / 2;
      maxY.setCursor(textOffset, 0);
      maxY.println(String(maxValueY));
    }
    else if (gForceY < minValueY) {
      minValueY = gForceY;
      minY.fillSprite(TFT_TRANSPARENT);
      int textWidth = minY.textWidth(String(abs(minValueY)));
      int textOffset = (55 - textWidth) / 2;
      minY.setCursor(textOffset, 0);
      minY.println(String(abs(minValueY)));
    }

    // Erase point and draw new one
    eraser.pushSprite(oldX, oldY);
    outline.pushSprite((DISPLAY_WIDTH - GMETER_WIDTH) / 2, (GMETER_PADDING / 2));
    point.pushSprite(posX, posY, TFT_TRANSPARENT);
    minX.pushSprite(10, DISPLAY_CENTER_Y - 6);
    maxX.pushSprite(DISPLAY_WIDTH - 55, DISPLAY_CENTER_Y - 6);
    minY.pushSprite(DISPLAY_CENTER_X - 25, 2);
    maxY.pushSprite(DISPLAY_CENTER_X - 25, DISPLAY_HEIGHT - 20);

    oldX = posX;
    oldY = posY;
    delay(25); // Delay to avoid flickering
  }
};

#endif


