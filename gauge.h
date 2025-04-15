#ifndef GAUGE_H
#define GAUGE_H

#include <TFT_eSPI.h>
#include "config.h"

class Gauge {
public:
  Gauge(TFT_eSPI* display) : display(display) {}
  virtual void render(double reading) = 0;  // Pure virtual function
protected:
  TFT_eSPI* display;
};

#endif