#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BLEDevice.h>
#include "config.h"

extern BLEAddress obdAddress;
extern BLERemoteCharacteristic* pWriteChar;
extern BLERemoteCharacteristic* pNotifyChar;
extern BLEClient* pClient;
extern boolean connected;
extern String responseBuffer;

bool connectToOBD();
bool reconnectToOBD();

#endif