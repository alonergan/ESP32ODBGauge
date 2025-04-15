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

#endif // CONFIG_H
