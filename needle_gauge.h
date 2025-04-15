#ifndef NEEDLE_GAUGE_H
#define NEEDLE_GAUGE_H

#include <math.h>
#include "gauge.h"
#include "config.h"

class NeedleGauge : public Gauge {
public:
  NeedleGauge(TFT_eSPI* display, double minValue, double maxValue, String units)
    : Gauge(display), minValue(minValue), maxValue(maxValue), units(units) {}

  void render(double reading) override {
    // Draw test for now
    display->fillScreen(DISPLAY_BG_COLOR);
    display->drawRect(100, 100, 50, 50, TFT_RED);
  }

private:
  double minValue, maxValue;
  String units;

  double mapValue(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
};

#endif