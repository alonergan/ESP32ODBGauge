#ifndef ACCEL_METER 
#define ACCEL_METER

#include "gauge.h"
#include "config.h"

class AccelerationMeter : public Gauge {
public:
  AccelerationMeter(TFT_eSPI* display),
  Gauge(display),
  time(display),
  barGraph(display),
  message(display),
  startValue(0.00),
  endValue(60.00),
  timeValue(0.00) {}

  void initialize() override {
    display->fillScreen(DISPLAY_BG_COLOR);

    time.setColorDepth(8);
    time.setTextSize(4);
    time.setTextFont(1);
    time.setTextColor(TFT_RED);
    time.createSprite(220, 190);
    time.drawRect(0, 0, time.width(), time.height(), TFT_RED);
    int textWidth = time.textWidth("0.00");
    int textHeight = time.fontHeight();
    time.setCursor(time.width() - textWidth / 2, time.height() - textHeight / 2);
    time.println("0.00");
    time.pushSprite(50, 50);

    barGraph.createSprite(220, 40);
    barGraph.fillSprite(TFT_RED);
    barGraph.drawRect(0, 0, time.width(), time.height(), TFT_RED);
    barGraph.pushSprite(50, 0);
  }

  void render(double value) override {
    renderAccelerationMeter();
  }

  void displayStats(float fps, double frameAvg, double queryAvg) override {
    display->setTextColor(TFT_WHITE, DISPLAY_BG_COLOR);
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->printf("FPS: %.1f\nFrame: %.1f ms\nQuery: %.1f ms", fps, frameAvg, queryAvg);
  }

private:
  TFT_eSPI time, barGraph, message;
  double timeValue, starValue, endValue;


  void renderAccelerationMeter() {
    
  }
}




#endif //ACCEL_METER