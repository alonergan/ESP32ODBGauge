#ifndef OPTIONS_SCREEN_H
#define OPTIONS_SCREEN_H

#include <TFT_eSPI.h>

class OptionsScreen {
public:
    OptionsScreen(TFT_eSPI* display, Gauge** gauges, int numGauges) : 
        display(display), 
        gauges(gauges), 
        numGauges(numGauges), 
        screenSprite(display),
        state(MAIN_MENU) {}

    void initialize() {
        display->fillScreen(TFT_BLACK);
        if (!screenSprite.createSprite(DISPLAY_WIDTH, DISPLAY_HEIGHT)) {
            Serial.println("Failed to create options screen sprite");
            return;
        }
        drawMainMenu();
    }

    bool handleTouch(uint16_t x, uint16_t y) {
        if (state == MAIN_MENU) {
            // Check main menu buttons
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    int bx = BUTTON_MARGIN + j * (BUTTON_WIDTH + BUTTON_SPACING);
                    int by = BUTTON_MARGIN + i * (BUTTON_HEIGHT + BUTTON_SPACING);
                    if (x >= bx && x < bx + BUTTON_WIDTH && y >= by && y < by + BUTTON_HEIGHT) {
                        if (i == 0 && j == 0) { // Change needle color
                            state = COLOR_PICKER;
                            drawColorPicker();
                            return true;
                        } else if (i == 1 && j == 1) { // Exit
                            return false; // Signal to exit options screen
                        }
                    }
                }
            }
        } else if (state == COLOR_PICKER) {
            // Check color swatches
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    int cx = BUTTON_MARGIN + j * (COLOR_SWATCH_SIZE + BUTTON_SPACING);
                    int cy = BUTTON_MARGIN + i * (COLOR_SWATCH_SIZE + BUTTON_SPACING);
                    if (x >= cx && x < cx + COLOR_SWATCH_SIZE && y >= cy && y < cy + COLOR_SWATCH_SIZE) {
                        uint16_t color = colorOptions[i * 3 + j];
                        updateNeedleColor(color);
                        state = MAIN_MENU;
                        drawMainMenu();
                        return true;
                    }
                }
            }
            // Check back button
            int backX = DISPLAY_WIDTH - BUTTON_MARGIN - BUTTON_WIDTH;
            int backY = DISPLAY_HEIGHT - BUTTON_MARGIN - BUTTON_HEIGHT;
            if (x >= backX && x < backX + BUTTON_WIDTH && y >= backY && y < backY + BUTTON_HEIGHT) {
                state = MAIN_MENU;
                drawMainMenu();
                return true;
            }
        }
        return true;
    }

private:
    TFT_eSPI* display;
    Gauge** gauges;
    int numGauges;
    TFT_eSprite screenSprite;
    
    enum ScreenState { MAIN_MENU, COLOR_PICKER };
    ScreenState state;

    // Button dimensions
    static const int BUTTON_WIDTH = 100;
    static const int BUTTON_HEIGHT = 60;
    static const int BUTTON_SPACING = 20;
    static const int BUTTON_MARGIN = 20;
    static const int COLOR_SWATCH_SIZE = 60;

    // Color options
    uint16_t colorOptions[6] = {
        TFT_RED, TFT_GREEN, TFT_BLUE,
        TFT_YELLOW, TFT_CYAN, TFT_MAGENTA
    };

    void drawMainMenu() {
        screenSprite.fillSprite(TFT_BLACK);
        screenSprite.setTextFont(2);
        screenSprite.setTextSize(1);
        screenSprite.setTextColor(TFT_WHITE);

        // Draw 2x2 grid of buttons
        const char* labels[2][2] = {
            {"Needle Color", "Option 2"},
            {"Option 3", "Exit"}
        };
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                int x = BUTTON_MARGIN + j * (BUTTON_WIDTH + BUTTON_SPACING);
                int y = BUTTON_MARGIN + i * (BUTTON_HEIGHT + BUTTON_SPACING);
                screenSprite.fillRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, TFT_DARKGREY);
                screenSprite.drawRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, TFT_WHITE);
                int textWidth = screenSprite.textWidth(labels[i][j]);
                int textHeight = screenSprite.fontHeight();
                screenSprite.setCursor(x + (BUTTON_WIDTH - textWidth) / 2, y + (BUTTON_HEIGHT - textHeight) / 2);
                screenSprite.print(labels[i][j]);
            }
        }
        screenSprite.pushSprite(0, 0);
    }

    void drawColorPicker() {
        screenSprite.fillSprite(TFT_BLACK);
        screenSprite.setTextFont(2);
        screenSprite.setTextSize(1);
        screenSprite.setTextColor(TFT_WHITE);

        // Draw 2x3 grid of color swatches
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                int x = BUTTON_MARGIN + j * (COLOR_SWATCH_SIZE + BUTTON_SPACING);
                int y = BUTTON_MARGIN + i * (COLOR_SWATCH_SIZE + BUTTON_SPACING);
                screenSprite.fillRect(x, y, COLOR_SWATCH_SIZE, COLOR_SWATCH_SIZE, colorOptions[i * 3 + j]);
                screenSprite.drawRect(x, y, COLOR_SWATCH_SIZE, COLOR_SWATCH_SIZE, TFT_WHITE);
            }
        }

        // Draw back button
        int backX = DISPLAY_WIDTH - BUTTON_MARGIN - BUTTON_WIDTH;
        int backY = DISPLAY_HEIGHT - BUTTON_MARGIN - BUTTON_HEIGHT;
        screenSprite.fillRect(backX, backY, BUTTON_WIDTH, BUTTON_HEIGHT, TFT_DARKGREY);
        screenSprite.drawRect(backX, backY, BUTTON_WIDTH, BUTTON_HEIGHT, TFT_WHITE);
        int textWidth = screenSprite.textWidth("Back");
        int textHeight = screenSprite.fontHeight();
        screenSprite.setCursor(backX + (BUTTON_WIDTH - textWidth) / 2, backY + (BUTTON_HEIGHT - textHeight) / 2);
        screenSprite.print("Back");

        screenSprite.pushSprite(0, 0);
    }

    void updateNeedleColor(uint16_t color) {
        for (int i = 0; i < numGauges; i++) {
            if (gauges[i]->getType() == Gauge::NEEDLE_GAUGE) {
                static_cast<NeedleGauge*>(gauges[i])->setNeedleColor(color);
            }
        }
    }
};

#endif