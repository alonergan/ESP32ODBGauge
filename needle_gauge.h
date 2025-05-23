#ifndef NEEDLE_GAUGE_H
#define NEEDLE_GAUGE_H

#include "gauge.h"

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
        targetValue(0.0),
        currentAngle(-120),
        oldAngle(-120),
        sweepState(SWEEP_UP),
        sweepStartTime(0),
        sweepValue(0.0),
        needleColor(NEEDLE_COLOR_PRIMARY),
        outlineColor(GAUGE_FG_COLOR),
        valueColor(VALUE_TEXT_COLOR) {}

    void initialize() override {
        display->fillScreen(DISPLAY_BG_COLOR);
        display->setPivot(DISPLAY_CENTER_X, DISPLAY_CENTER_Y);
        createOutline();
        createNeedle();
        createValue();
        createEraser();
        gaugeOutline.pushSprite(DISPLAY_CENTER_X - GAUGE_RADIUS, 0);
        plotNeedle(-120);
        plotValue(0.0);

        if (!stats.createSprite(40, 120)) {
            Serial.println("Could not create stats");
        }
        Serial.println("Created stats, exiting initialization");

        // Initialize sweep
        sweepState = SWEEP_UP;
        sweepStartTime = millis();
        sweepValue = minValue;
        currentAngle = -120;
        oldAngle = -120;
    }

    void setReading(double reading) {
        targetValue = constrain(reading, minValue, maxValue);
    }

    void setNeedleColor(uint16_t color) {
        needleColor = color;
        // Recreate needle sprite with new color
        gaugeNeedle.deleteSprite();
        createNeedle();
        plotNeedle(currentAngle);
    }

    void setOutlineColor(uint16_t color) {
        outlineColor = color;
        gaugeOutline.deleteSprite();
        createOutline();
    }

    void setValueColor(uint16_t color) {
        valueColor = color;
        gaugeValue.deleteSprite();
        createValue();
        plotValue(0.0);
    }

    void render(double) override {
        const unsigned long SWEEP_UP_DURATION = 1500; // 1 second up
        const unsigned long SWEEP_DOWN_DURATION = 1500; // 1 second down
        unsigned long currentTime = millis();

        if (sweepState == SWEEP_UP) {
            double progress = (double)(currentTime - sweepStartTime) / SWEEP_UP_DURATION;
            if (progress >= 1.0) {
                sweepValue = maxValue;
                currentAngle = 120;
                sweepState = SWEEP_DOWN;
                sweepStartTime = currentTime;
            } else {
                sweepValue = minValue + (maxValue - minValue) * progress;
                currentAngle = -120 + 240 * progress;
            }
            plotNeedle(currentAngle);
            plotValue(sweepValue);
        } else if (sweepState == SWEEP_DOWN) {
            double target = targetValue > 0.0 ? targetValue : 0.0;
            double progress = (double)(currentTime - sweepStartTime) / SWEEP_DOWN_DURATION;
            if (progress >= 1.0) {
                sweepValue = target;
                currentAngle = calculateAngle(target);
                sweepState = SWEEP_COMPLETE;
            } else {
                sweepValue = maxValue - (maxValue - target) * progress;
                currentAngle = 120 - (120 - calculateAngle(target)) * progress;
            }
            plotNeedle(currentAngle);
            plotValue(sweepValue);
        } else {
            double targetAngle = calculateAngle(targetValue);
            currentAngle += 0.3 * (targetAngle - currentAngle); // Smoothing factor
            if (abs(currentAngle - oldAngle) > 1) {
                gaugeEraser.pushRotated(oldAngle);
                plotNeedle(currentAngle);
                oldAngle = currentAngle;
            }
            plotValue(targetValue);
        }
    }

    void reset() {
        display->fillScreen(DISPLAY_BG_COLOR);
        gaugeOutline.deleteSprite();
        gaugeValue.deleteSprite();
        gaugeNeedle.deleteSprite();
        gaugeEraser.deleteSprite();
        initialize();
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

    GaugeType getType() const override {
        return NEEDLE_GAUGE;
    }

private:
    TFT_eSprite gaugeOutline, gaugeNeedle, gaugeValue, gaugeEraser, stats;
    double targetValue, currentAngle;
    int16_t oldAngle;
    double minValue, maxValue;
    String valueLabel, valueUnits, valueType;
    uint16_t needleColor, outlineColor, valueColor; // Runtime needle color

    // Sweep state
    enum SweepState { SWEEP_UP, SWEEP_DOWN, SWEEP_COMPLETE };
    SweepState sweepState;
    unsigned long sweepStartTime;
    double sweepValue;

    static String gaugeTypes[4][5];

    void createOutline() {
        if (!gaugeOutline.createSprite(GAUGE_WIDTH, GAUGE_HEIGHT)) {
            Serial.println("Failed to create gauge outline");
            return;
        }
        gaugeOutline.fillSprite(GAUGE_BG_COLOR);
        gaugeOutline.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS - GAUGE_LINE_WIDTH, GAUGE_START_ANGLE, GAUGE_END_ANGLE, outlineColor, GAUGE_BG_COLOR, true);
        gaugeOutline.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS - GAUGE_LINE_WIDTH - GAUGE_ARC_WIDTH, GAUGE_RADIUS - (GAUGE_LINE_WIDTH * 2) - GAUGE_ARC_WIDTH, GAUGE_START_ANGLE, GAUGE_END_ANGLE, outlineColor, GAUGE_BG_COLOR, true);

        gaugeOutline.setFreeFont(FONT_BOLD_14);
        gaugeOutline.setTextColor(outlineColor);
        int textWidth = gaugeOutline.textWidth(valueLabel);
        int x = (GAUGE_WIDTH - textWidth) / 2;
        gaugeOutline.drawString(valueLabel, x, GAUGE_RADIUS - 20);
        gaugeOutline.unloadFont();

        gaugeOutline.setFreeFont(FONT_NORMAL_8);
        textWidth = gaugeOutline.textWidth(valueUnits);
        x = (GAUGE_WIDTH - textWidth) / 2;
        gaugeOutline.drawString(valueUnits, x, GAUGE_RADIUS + 20);
        gaugeOutline.unloadFont();
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
        gaugeNeedle.fillRect(pivX - 1, 2, 3, NEEDLE_LENGTH, needleColor);
    }

    void createValue() {
        if (!gaugeValue.createSprite(VALUE_WIDTH, VALUE_HEIGHT)) {
            Serial.println("Failed to create value sprite");
            return;
        }
        gaugeValue.fillSprite(VALUE_BG_COLOR);
        gaugeValue.setFreeFont(FONT_BOLD_18);
        gaugeValue.setTextColor(valueColor);
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

    void plotNeedle(double angle) {
        gaugeEraser.pushRotated(oldAngle);
        gaugeNeedle.pushRotated((int16_t)angle);
        oldAngle = (int16_t)angle;
    }

    void plotValue(double val) {
        gaugeValue.fillSprite(VALUE_BG_COLOR);
        gaugeValue.setFreeFont(FONT_BOLD_18);
        gaugeValue.setTextColor(valueColor, DISPLAY_BG_COLOR);

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
        gaugeValue.unloadFont();
    }

    double calculateAngle(double value) {
        double angle = (value / maxValue) * 240.0 - 120;
        return constrain(angle, -120, 120);
    }
};

String NeedleGauge::gaugeTypes[4][5] = {
    {"RPM", "", "0", "7000", "int"},
    {"BOOST", "psi", "0.0", "22.0", "double"},
    {"TORQUE", "lb-ft", "0", "445", "int"},
    {"HORSEPOWER", "hp", "0", "450", "int"}
};

#endif