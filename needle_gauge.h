#ifndef NEEDLE_GAUGE_H
#define NEEDLE_GAUGE_H

#include <math.h>
#include "gauge.h"
#include "config.h"
#include <Arduino.h>

class NeedleGauge : public Gauge {
public:
    NeedleGauge(TFT_eSPI* display, int gaugeType) : 
        Gauge(display),
        gaugeOutline(display),
        gaugeNeedle(display),
        gaugeValue(display),
        gaugeEraser(display),
        stats(display),
        valueLabel(gaugeTypes[gaugeType][0]),
        valueUnits(gaugeTypes[gaugeType][1]),
        minValue(gaugeTypes[gaugeType][2].toDouble()),
        maxValue(gaugeTypes[gaugeType][3].toDouble()),
        valueType(gaugeTypes[gaugeType][4]),
        currentAngle(-130),
        oldAngle(-130) {}

    void initialize() override {
        display->fillScreen(DISPLAY_BG_COLOR);
        display->setPivot(DISPLAY_CENTER_X, DISPLAY_CENTER_Y);
        createOutline();
        createNeedle();
        createValue();
        createEraser();
        plotNeedle(0.0);
        plotValue(0.0);

        if (!stats.createSprite(40, 120)) {
            Serial.println("Could not create stats");
        }
        Serial.println("Created stats, exiting intialization");
    }

    void render(double reading) override {
        reading = constrain(reading, minValue, maxValue);
        plotNeedle(reading);
        plotValue(reading);
    }

    void displayStats(float fps, double frameAvg, double queryAvg) {
        stats.fillSprite(TFT_BLACK);
        stats.setTextFont(1);
        stats.setTextSize(1);
        stats.setTextColor(TFT_WHITE);
        stats.setRotation(0);
        stats.setCursor(0, 0);
        stats.println("FPS");
        stats.println(String(fps));
        stats.println("FrmAvg");
        stats.println(String(frameAvg));
        stats.println("QryAvg");
        stats.println(String(queryAvg));
        stats.pushSprite(0, 0);
        Serial.println("Pushed stats");
    }

private:
    TFT_eSprite gaugeOutline, gaugeNeedle, gaugeValue, gaugeEraser, stats;
    int16_t currentAngle, oldAngle;
    double minValue, maxValue;
    String valueLabel, valueUnits, valueType;

    static String gaugeTypes[4][5];

    void createOutline() {
        if (!gaugeOutline.createSprite(GAUGE_WIDTH, GAUGE_HEIGHT)) {
            Serial.println("Failed to create gauge outline");
            return;
        }
        gaugeOutline.fillSprite(GAUGE_BG_COLOR);
        gaugeOutline.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS - GAUGE_LINE_WIDTH, GAUGE_START_ANGLE, GAUGE_END_ANGLE, GAUGE_FG_COLOR, GAUGE_BG_COLOR, true);
        gaugeOutline.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS - GAUGE_LINE_WIDTH - GAUGE_ARC_WIDTH, GAUGE_RADIUS - (GAUGE_LINE_WIDTH * 2) - GAUGE_ARC_WIDTH, GAUGE_START_ANGLE, GAUGE_END_ANGLE, GAUGE_FG_COLOR, GAUGE_BG_COLOR, true);

        gaugeOutline.setTextFont(1);
        gaugeOutline.setTextSize(GAUGE_LABEL_SIZE);
        int textWidth = gaugeOutline.textWidth(valueLabel);
        int x = (GAUGE_WIDTH - textWidth) / 2;
        gaugeOutline.drawString(valueLabel, x, GAUGE_RADIUS - 20);

        gaugeOutline.setTextSize(GAUGE_UNITS_SIZE);
        textWidth = gaugeOutline.textWidth(valueUnits);
        x = (GAUGE_WIDTH - textWidth) / 2;
        gaugeOutline.drawString(valueUnits, x, GAUGE_RADIUS + 20);

        gaugeOutline.pushSprite(DISPLAY_CENTER_X - GAUGE_RADIUS, 0);
    }

    void createNeedle() {
        if (!gaugeNeedle.createSprite(NEEDLE_WIDTH, NEEDLE_LENGTH)) {
            Serial.println("Failed to create needle sprite");
            return;
        }
        gaugeNeedle.fillSprite(GAUGE_BG_COLOR);
        uint16_t pivX = NEEDLE_WIDTH / 2;
        uint16_t pivY = NEEDLE_RADIUS;
        gaugeNeedle.setPivot(pivX, pivY);
        gaugeNeedle.fillRect(pivX - 1, 2, 3, NEEDLE_LENGTH, NEEDLE_COLOR_PRIMARY);
    }

    void createValue() {
        if (!gaugeValue.createSprite(VALUE_WIDTH, VALUE_HEIGHT)) {
            Serial.println("Failed to create value sprite");
            return;
        }
        gaugeValue.fillSprite(VALUE_BG_COLOR);
        gaugeValue.setTextFont(1);
        gaugeValue.setTextColor(VALUE_TEXT_COLOR);
        gaugeValue.setTextSize(VALUE_FONT_SIZE);
    }

    void createEraser() {
        if (!gaugeEraser.createSprite(NEEDLE_WIDTH, NEEDLE_LENGTH)) {
            Serial.println("Failed to create needle sprite");
            return;
        }
        gaugeEraser.fillSprite(GAUGE_BG_COLOR);
        int pivX = NEEDLE_WIDTH / 2;
        int pivY = NEEDLE_RADIUS;
        gaugeEraser.setPivot(pivX, pivY);
    }

    void plotNeedle(double reading) {
        uint16_t targetAngle = (uint16_t)ceil((reading / maxValue) * 260.0);
        targetAngle = constrain(targetAngle, 0, 260);
        targetAngle -= 120;

        if (oldAngle != targetAngle) {
            gaugeEraser.pushRotated(oldAngle);
        }
        currentAngle = targetAngle;
        gaugeNeedle.pushRotated(currentAngle);
        oldAngle = currentAngle;
    }

    void plotValue(double val) {
        gaugeValue.fillSprite(VALUE_BG_COLOR);
        if (valueType == "int") {
            int intVal = (int)round(val);
            int textWidth = gaugeValue.textWidth(String(intVal));
            int x = (VALUE_WIDTH - textWidth) / 2;
            gaugeValue.drawNumber(intVal, x, 0);
        } else {
            int textWidth = gaugeValue.textWidth(String(val, 1));
            int x = (VALUE_WIDTH - textWidth) / 2;
            gaugeValue.drawFloat(val, 1, x, 0);
        }
        gaugeValue.pushSprite(VALUE_X, VALUE_Y);
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