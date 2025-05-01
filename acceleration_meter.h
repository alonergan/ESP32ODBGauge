#ifndef ACCEL_METER_H
#define ACCEL_METER_H

#include "gauge.h"

class AccelerationMeter : public Gauge {
public:
  AccelerationMeter(TFT_eSPI* display) :
  Gauge(display),
  time(display),
  timeLabel(display),
  speed(display),
  speedLabel(display),
  message(display),
  startValue(0.00),
  endValue(60.00),
  timeValue(0.00),
  endSpeedReached(false),
  previousSpeed(0.00),
  startTime(0) {}

  void initialize() override {
    display->fillScreen(DISPLAY_BG_COLOR);
    //display->drawLine(DISPLAY_CENTER_X, 0, DISPLAY_CENTER_X, DISPLAY_HEIGHT, TFT_BLUE); // Vertically centered line for development
    int textWidth = 0;
    int textHeight = 0;

    speed.setColorDepth(8);
    speed.setTextSize(7);
    speed.setTextFont(1);
    speed.setTextColor(TFT_RED);
    textWidth = speed.textWidth("77");
    textHeight = speed.fontHeight();
    speed.createSprite(textWidth, textHeight);
    textWidth = speed.textWidth("0");
    textHeight = speed.fontHeight();
    Serial.printf("SPEED\ntextWidth: %.1d\ntextHeight: %.1d\nspriteWidth: %.1d\nspriteHeight: %.1d\n", textWidth, textHeight, speed.width(), speed.height());
    speed.setCursor((speed.width() - textWidth) / 2, (speed.height() - textHeight) / 2);
    speed.println("0");
    speed.pushSprite((DISPLAY_WIDTH - speed.width()) / 2, 60);

    speedLabel.setColorDepth(1);
    speedLabel.setTextFont(1);
    speedLabel.setTextSize(3);
    speedLabel.setTextColor(TFT_WHITE);
    speedLabel.createSprite(speedLabel.textWidth(AMETER_SPEED_LABEL), speedLabel.fontHeight());
    speedLabel.setCursor(0, 0);
    speedLabel.println(AMETER_SPEED_LABEL);
    speedLabel.pushSprite((DISPLAY_WIDTH - speedLabel.width()) / 2, 60 - speedLabel.height() - AMETER_V_PADDING);

    time.setColorDepth(8);
    time.setTextSize(5);
    time.setTextFont(1);
    time.setTextColor(TFT_RED);
    textWidth = time.textWidth("77.77"); // Largest numbers horizontally
    textHeight = time.fontHeight();
    time.createSprite(textWidth, textHeight);
    //time.drawRect(0, 0, time.width(), time.height(), TFT_RED); // Bound sprite with box for development
    textWidth = time.textWidth("0.00");
    textHeight = time.fontHeight();
    Serial.printf("TIME\ntextWidth: %.1d\ntextHeight: %.1d\nspriteWidth: %.1d\nspriteHeight: %.1d\n", textWidth, textHeight, time.width(), time.height());
    time.setCursor((time.width() - textWidth) / 2, (time.height() - textHeight) / 2);
    time.println("0.00");
    time.pushSprite((DISPLAY_WIDTH - time.width()) / 2, 175);

    timeLabel.setColorDepth(1);
    timeLabel.setTextFont(1);
    timeLabel.setTextSize(3);
    timeLabel.setTextColor(TFT_WHITE);
    timeLabel.createSprite(timeLabel.textWidth(AMETER_TIME_LABEL), timeLabel.fontHeight());
    timeLabel.setCursor(0, 0);
    timeLabel.println(AMETER_TIME_LABEL);
    timeLabel.pushSprite((DISPLAY_WIDTH - timeLabel.width()) / 2, 175 - timeLabel.height() - AMETER_V_PADDING);
  }

  void render(double value) override {
    renderAccelerationMeter(value);
  }

  void displayStats(float fps, double frameAvg, double queryAvg) override {
    display->setTextColor(TFT_WHITE, DISPLAY_BG_COLOR);
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->printf("FPS: %.1f\nFrame: %.1f ms\nQuery: %.1f ms", fps, frameAvg, queryAvg);
  }

private:
  TFT_eSprite time, speed, timeLabel, speedLabel, message;
  double timeValue, startValue, endValue, previousSpeed;
  int startTime;
  bool endSpeedReached;

  void renderAccelerationMeter(double speedVal) {
    // Grab current time
    unsigned int now = millis();

    // Start elapsed time if vehicle speed moves from standstill
    Serial.printf("PreviousSpeed: %.1d | CurrentSpeed: %.1d", previousSpeed, speedVal);
    if (previousSpeed == 0.0 && speedVal >= 0.0) {
      Serial.println("Starting timer");
      startTime = now;
    }

    // Return if end speed reached
    if (endSpeedReached) {
      return;
    }

    // Target speed reached, set flag and display result in green
    if (speedVal >= 60.0) {
      endSpeedReached = true;
      time.setTextColor(TFT_GREEN);
    }

    // Get elapsed time convert this to ss:ms format and display time
    unsigned int elapsedMilliseconds = now - startTime;
    int seconds = elapsedMilliseconds / 1000;
    int milliseconds = elapsedMilliseconds % 1000;
    Serial.printf("Elapsed: %.1d | Seconds: %.1d | MilliSeconds: %.1d | startTime: %.1d", elapsedMilliseconds, seconds, milliseconds, startTime);
    String currentTime = String(seconds) + "." + String(milliseconds);
    int textWidth = time.textWidth(currentTime);
    int textHeight = time.fontHeight();
    time.fillSprite(DISPLAY_BG_COLOR);
    time.setCursor((time.width() - textWidth) / 2, (time.height() - textHeight) / 2);
    time.println(currentTime);
    time.pushSprite((DISPLAY_WIDTH - time.width()) / 2, 175);      

    // Update speed
    String text = String((int) speedVal);
    textWidth = speed.textWidth(text);
    textHeight = speed.fontHeight();
    speed.fillSprite(DISPLAY_BG_COLOR);
    speed.setCursor((speed.width() - textWidth) / 2, (speed.height() - textHeight) / 2);
    speed.println(text);
    speed.pushSprite((DISPLAY_WIDTH - speed.width()) / 2, 60);
  }
};




#endif //ACCEL_METER_H