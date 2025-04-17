#ifndef CONFIG_H
#define CONFIG_H

#include <TFT_eSPI.h>

/*
  Bluetooth config
*/
#define BLUETOOTH_DEVICE_ADDRESS "8c:de:52:dc:5d:2a"         // OBDII BLE device MAC address
#define BLUETOOTH_SERVICE_UUID "FFF0"                        // OBDII BLE main service UUID
#define BLUETOOTH_WRITE_CHAR_UUID "FFF2"                     // OBDII BLE write/read characteristic UUID
#define BLUETOOTH_NOTIFY_CHAR_UUID "FFF1"                    // OBDII BLE write/notify characteristic UUID

/*
  Display config
*/
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define DISPLAY_CENTER_X DISPLAY_WIDTH / 2
#define DISPLAY_CENTER_Y DISPLAY_HEIGHT / 2
#define DISPLAY_BG_COLOR TFT_BLACK

/*
  Gauge params
*/
#define GAUGE_WIDTH DISPLAY_HEIGHT
#define GAUGE_HEIGHT DISPLAY_HEIGHT
#define GAUGE_RADIUS GAUGE_HEIGHT / 2
#define GAUGE_ARC_WIDTH 50
#define GAUGE_LINE_WIDTH 3
#define GAUGE_START_ANGLE 60
#define GAUGE_END_ANGLE 300
#define GAUGE_FG_COLOR TFT_WHITE
#define GAUGE_BG_COLOR DISPLAY_BG_COLOR
#define GAUGE_LABEL_SIZE 3
#define GAUGE_UNITS_SIZE 2

/* 
  Needle Params 
*/
#define NEEDLE_WIDTH 10
#define NEEDLE_LENGTH 45
#define NEEDLE_RADIUS 115
#define NEEDLE_COLOR_PRIMARY TFT_RED
#define NEEDLE_UPDATE_THRESHOLD 1

/* 
  Value Params 
*/
#define VALUE_WIDTH 120
#define VALUE_HEIGHT 45
#define VALUE_X (DISPLAY_WIDTH - VALUE_WIDTH) / 2
#define VALUE_Y 180
#define VALUE_BG_COLOR DISPLAY_BG_COLOR
#define VALUE_TEXT_COLOR TFT_WHITE
#define VALUE_FONT_SIZE 4

#endif // CONFIG_H
