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
        state(MAIN_MENU),
        bluetoothState(MAIN_BLUETOOTH_MENU),
        colorState(MAIN_COLOR_MENU) {}

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
                        if (i == 0 && j == 0) { // Gauge settings
                            state = GAUGES;
                            drawGaugesMenu();
                            return true;
                        } else if (i == 0 && j == 1) { // Bluetooth settings
                            state = BLUETOOTH;
                            drawBluetoothMenu();
                            return true;
                        } else if (i == 1 && j == 0) { // Color settings
                            state = COLOR;
                            drawColorMenu();
                            return true;
                        } else if (i == 1 && j == 1) { // Exit
                            return false;
                        }
                    }
                }
            }
        } else if (state == GAUGES) {
            return handleGaugesTouch(x, y);
        } else if (state == BLUETOOTH) {
            return handleBluetoothTouch(x, y);
        } else if (state == COLOR) {
            return handleColorTouch(x, y);
        }
        return true;
    }

private:
    TFT_eSPI* display;
    Gauge** gauges;
    int numGauges;
    TFT_eSprite screenSprite;
    
    enum ScreenState { MAIN_MENU, GAUGES, BLUETOOTH, COLOR };
    enum BluetoothState { MAIN_BLUETOOTH_MENU, PAIRING_MENU, STATS_MENU };
    enum ColorState { MAIN_COLOR_MENU, NEEDLE_COLOR, OUTLINE_COLOR, VALUE_COLOR };
    ScreenState state;
    BluetoothState bluetoothState;
    ColorState colorState;

    // Button dimensions
    static const int BUTTON_WIDTH = 140;
    static const int BUTTON_HEIGHT = 100;
    static const int BUTTON_SPACING = 20;
    static const int BUTTON_MARGIN = 10;
    static const int COLOR_SWATCH_SIZE = 60;

    // Color options
    uint16_t colorOptions[12] = {
        TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW,
        TFT_ORANGE, TFT_DARKGREEN, TFT_CYAN, TFT_GOLD, 
        TFT_VIOLET, TFT_PURPLE, TFT_SILVER, TFT_WHITE
    };

    bool handleGaugesTouch(uint16_t x, uint16_t y) {
        // Check main menu buttons
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                int bx = BUTTON_MARGIN + j * (BUTTON_WIDTH + BUTTON_SPACING);
                int by = BUTTON_MARGIN + i * (BUTTON_HEIGHT + BUTTON_SPACING);
                if (x >= bx && x < bx + BUTTON_WIDTH && y >= by && y < by + BUTTON_HEIGHT) {
                    if (i == 0 && j == 0) { // RPM
                        // Todo for now. Not sure how this should be handled
                    } else if (i == 0 && j == 1) { // Boost

                    } else if (i == 0 && j == 2) { // Torque
                        
                    } else if (i == 1 && j == 0) { // GMeter
                        
                    } else if (i == 1 && j == 1) { // Accel. Meter
                        
                    } else if (i == 1 && j == 2) { // Exit
                        state = MAIN_MENU;
                        drawMainMenu();
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool handleColorTouch(uint16_t x, uint16_t y) {
        if (colorState == MAIN_COLOR_MENU) {
            // Check main menu buttons
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    int bx = BUTTON_MARGIN + j * (BUTTON_WIDTH + BUTTON_SPACING);
                    int by = BUTTON_MARGIN + i * (BUTTON_HEIGHT + BUTTON_SPACING);
                    if (x >= bx && x < bx + BUTTON_WIDTH && y >= by && y < by + BUTTON_HEIGHT) {
                        if (i == 0 && j == 0) { // Needle color
                            colorState = NEEDLE_COLOR;
                            drawColorPicker();
                            return true;
                        } else if (i == 0 && j == 1) { // Outline color
                            colorState = OUTLINE_COLOR;
                            drawColorPicker();
                            return true;
                        } else if (i == 1 && j == 0) { // Value color
                            colorState = VALUE_COLOR;
                            drawColorPicker();
                            return true;
                        } else if (i == 1 && j == 1) { // Exit
                            colorState = MAIN_COLOR_MENU;
                            drawMainMenu();
                            return false;
                        }
                    }
                }
            }
        } else {
            // Not in main menu, only one sub menu to set color, so check color swatches
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 4; j++) {
                    int cx = BUTTON_MARGIN + j * (COLOR_SWATCH_SIZE + BUTTON_SPACING);
                    int cy = BUTTON_MARGIN + i * (COLOR_SWATCH_SIZE + BUTTON_SPACING);
                    if (x >= cx && x < cx + COLOR_SWATCH_SIZE && y >= cy && y < cy + COLOR_SWATCH_SIZE) {
                        uint16_t color = colorOptions[i * 4 + j];

                        // Handle color update based on screen state
                        switch (colorState) {
                            case NEEDLE_COLOR:
                                updateNeedleColor(color);
                                break;
                            case OUTLINE_COLOR:
                                updateOutlineColor(color);
                                break;
                            case VALUE_COLOR:
                                updateValueColor(color);
                                break;
                            default:
                                break;
                        }

                        // Return to the main color menu
                        colorState = MAIN_COLOR_MENU;
                        drawColorMenu();
                        return true;
                    }
                }
            }
        }
    }

    bool handleBluetoothTouch(uint16_t x, uint16_t y) {
        // Check main menu buttons
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                int bx = BUTTON_MARGIN + j * (BUTTON_WIDTH + BUTTON_SPACING);
                int by = BUTTON_MARGIN + i * (BUTTON_HEIGHT + BUTTON_SPACING);
                if (x >= bx && x < bx + BUTTON_WIDTH && y >= by && y < by + BUTTON_HEIGHT) {
                    if (i == 0 && j == 0) { // Pair device
                        // To be implemented
                    } else if (i == 0 && j == 1) { // Delete device
                        
                    } else if (i == 1 && j == 0) { // Stats
                        
                    } else if (i == 1 && j == 1) { // Exit
                        state = MAIN_MENU;
                        drawMainMenu();
                        return false;
                    }
                }
            }
        }
    }

    void drawMainMenu() {
        screenSprite.fillSprite(TFT_BLACK);
        screenSprite.setTextFont(2);
        screenSprite.setTextSize(1);
        screenSprite.setTextColor(TFT_WHITE);

        // Draw 2x2 grid of buttons
        const char* labels[2][2] = {
            {"Gauges", "Bluetooth"},
            {"Color", "Exit"}
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

    void drawGaugesMenu() {
        screenSprite.fillSprite(TFT_BLACK);
        screenSprite.setTextFont(2);
        screenSprite.setTextSize(1);
        screenSprite.setTextColor(TFT_WHITE);

        // Draw 2x2 grid of buttons
        const char* labels[2][3] = {
            {"RPM", "Boost", "Torque"},
            {"GMeter", "Acceleration Meter", "Exit"}
        };
        for (int i = 0; i < 3; i++) {
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

    void drawBluetoothMenu() {
        screenSprite.fillSprite(TFT_BLACK);
        screenSprite.setTextFont(2);
        screenSprite.setTextSize(1);
        screenSprite.setTextColor(TFT_WHITE);

        // Draw 2x2 grid of buttons
        const char* labels[2][2] = {
            {"Pair Device", "Stats"},
            {"Remove Device", "Exit"}
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

    void drawColorMenu() {
        screenSprite.fillSprite(TFT_BLACK);
        screenSprite.setTextFont(2);
        screenSprite.setTextSize(1);
        screenSprite.setTextColor(TFT_WHITE);

        // Draw 2x2 grid of buttons
        const char* labels[2][2] = {
            {"Needle Color", "Outline Color"},
            {"Value Color", "Exit"}
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

        // Draw 4x3 grid of color swatches
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 4; j++) {
                int x = BUTTON_MARGIN + j * (COLOR_SWATCH_SIZE + BUTTON_SPACING);
                int y = BUTTON_MARGIN + i * (COLOR_SWATCH_SIZE + BUTTON_SPACING);
                screenSprite.fillRect(x, y, COLOR_SWATCH_SIZE, COLOR_SWATCH_SIZE, colorOptions[i * 4 + j]);
                screenSprite.drawRect(x, y, COLOR_SWATCH_SIZE, COLOR_SWATCH_SIZE, TFT_WHITE);
            }
        }

        screenSprite.pushSprite(0, 0);
    }

    void updateNeedleColor(uint16_t color) {
        for (int i = 0; i < numGauges; i++) {
            if (gauges[i]->getType() == Gauge::NEEDLE_GAUGE) {
                static_cast<NeedleGauge*>(gauges[i])->setNeedleColor(color);
            }
        }
    }

    void updateOutlineColor(uint16_t color) {
        for (int i = 0; i < numGauges; i++) {
            if (gauges[i]->getType() == Gauge::NEEDLE_GAUGE) {
                static_cast<NeedleGauge*>(gauges[i])->setOutlineColor(color);
            }
        }
    }

    void updateValueColor(uint16_t color) {
        for (int i = 0; i < numGauges; i++) {
            if (gauges[i]->getType() == Gauge::NEEDLE_GAUGE) {
                static_cast<NeedleGauge*>(gauges[i])->setValueColor(color);
            }
        }
    }

    void updateGauge() {

    }
};

#endif