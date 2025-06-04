#ifndef DUAL_GAUGE_H
#define DUAL_GAUGE_H

#include "gauge.h"

class DualGauge : public Gauge {
public:
    DualGauge(TFT_eSPI* display, int gaugeType) : 
    Gauge(display),
    gaugeOutlineLeft(display),
    gaugeOutlineRight(display),
    gaugeMarkerLeft(display),
    gaugeMarkerRight(display),
    gaugeValueLeft(display),
    gaugeValueRight(display),
    gaugeEraserLeft(display),
    gaugeEraserRight(display),
    stats(display),
    valueLabelLeft(gaugeTypes[gaugeType][0]),
    valueLabelRight(gaugeTypes[gaugeType][0]),
    valueUnitsLeft(gaugeTypes[gaugeType][1]),
    valueUnitsRight(gaugeTypes[gaugeType][1]),
    minValueLeft(gaugeTypes[gaugeType][2].toDouble()),
    minValueRight(gaugeTypes[gaugeType][2].toDouble()),
    maxValueLeft(gaugeTypes[gaugeType][3].toDouble()),
    maxValueRight(gaugeTypes[gaugeType][3].toDouble()),
    valueTypeLeft(gaugeTypes[gaugeType][4]),
    valueTypeRight(gaugeTypes[gaugeType][4]),
    currentValueLeft(0),
    currentValueRight(0),
    oldValueLeft(0),
    oldValueRight(0) {}

    void initialize() override {
      // Clear screen
      display->fillScreen(DISPLAY_BG_COLOR);

      // Initialize left gauge
      createBarGraph();
      createLabel();
      createValue();
      plotValue(0.0);

      // Initialize right gauge - TODO
    }

    void render() override {

    }

    void displayStats() override {

    }

    GaugeType getType() const override {
        return NEEDLE_GAUGE;
    }

private:

    TFT_eSprite gaugeOutlineLeft, gaugeOutlineRight, gaugeMarkerLeft, gaugeMarkerRight, gaugeValueLeft, gaugeValueRight, gaugeEraserLeft, gaugeEraserRight, stats;
    String valueLabelLeft, valueLableRight, valueUnitsLeft, valueUnitsRight, valueTypeLeft, valueTypeRight;
    int16_t currentValueLeft, currentValueRight, oldValueLeft, oldValueRight;
    double minValueLeft, minValueRight, maxValueLeft, maxValueRight;
    static String gaugeTypes[4][5];

    void createBarGraph() {
      // Create sprite
      if (!gaugeOutlineLeft.createSprite(DG_BAR_WIDTH, DG_BAR_HEIGHT)) {
        Serial.println("Failed to create gaugeOutlineLeft sprite");
      }
      gaugeOutlineLeft.fillSprite(DISPLAY_BG_COLOR);


      // Draw bar graph outline
      gaugeOutlineLeft.drawRect(0, 0, DG_BAR_WIDTH, DG_BAR_HEIGHT, GAUGE_FG_COLOR);
      
      // Draw initial bar graph value
      gaugeOutlineLeft.fillRect(1, DG_BAR_HEIGHT - 1, DG_BAR_WIDTH - 2, 3, NEEDLE_COLOR_PRIMARY);

      // Push sprite at the center of the thirds of the rectangle
      int x = (DISPLAY_WIDTH / 3) - (DG_BAR_WIDTH / 2);
      int y = (DISPLAY_HEIGHT / 2) - (DG_BAR_HEIGHT / 2);
      gaugeOutlineLeft.pushSprite(x, y);
    }

    void createLabel() {
      gaugeOutlineLeft.setFreeFont(FONT_BOLD_18);
      gaugeOutlineLeft.setTextColor(GAUGE_FG_COLOR);
      int w = gaugeOutlineLeft.textWidth(valueLabelLeft);
      int h = gaugeOutlineLeft.fontHeight(valueLabelLeft);
      if (!valueLabelLeft.createSprite(w, h)) {
        Serial.println("Could not create valueLabelLeft sprite");
      }
      valueLabelLeft.fillSprite(DISPLAY_BG_COLOR);

      valueLabelLeft.setFreeFont(FONT_BOLD_18);
      valueLabelLeft.setTextColor(GAUGE_FG_COLOR);
      valueLabelLeft.drawString(valueLabelLeft, 0, 0);

      int xPos = (DISPLAY_WIDTH / 3)  - (w / 2);
      int yPos = (DISPLAY_HEIGHT / 2) - (DG_BAR_HEIGHT / 2) - h - 10;
      valueLabelLeft.pushSprite(xPos, yPos);
      valueLabelLeft.unloadFont();
    }

    void createValue() {
      // Create sprite
      if (!gaugeValueLeft.createSprite(VALUE_WIDTH, VALUE_HEIGHT)) {
        Serial.println("Could not create gaugeValueLabelLeft sprite");
      }
    }

    void plotValue(double val) {
        gaugeValueLeft.fillSprite(VALUE_BG_COLOR);
        gaugeValueLeft.setFreeFont(FONT_BOLD_18);
        gaugeValueLeft.setTextColor(VALUE_TEXT_COLOR, DISPLAY_BG_COLOR);
        int textWidth = 0;

        if (valueTypeLeft == "int") {
            int intVal = (int)round(val);
            textWidth = gaugeValueLeft.textWidth(String(intVal));
            int x = (VALUE_WIDTH - textWidth) / 2;
            gaugeValueLeft.drawNumber(intVal, x, 0);
        } else {
            textWidth = gaugeValueLeft.textWidth(String(val, 1));
            int x = (VALUE_WIDTH - textWidth) / 2;
            gaugeValueLeft.drawFloat(val, 1, x, 0);
        }

        int xPos = (DISPLAY_WIDTH / 3)  - (textWidth / 2);
        int yPos = (DISPLAY_HEIGHT / 2) + (DG_BAR_HEIGHT / 2) + 10;
        gaugeValueLeft.pushSprite(xPos, yPos);
        gaugeValueLeft.unloadFont();
    }
};

// Define and initialize static member outside class
String NeedleGauge::gaugeTypes[4][5] = {
    {"RPM", "", "0", "7000", "int"},
    {"BOOST", "psi", "0.0", "22.0", "double"},
    {"TORQUE", "lb-ft", "0", "445", "int"},
    {"HORSEPOWER", "hp", "0", "450", "int"}
};

#endif





