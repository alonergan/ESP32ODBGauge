#include "bluetooth.h"

BLEAddress obdAddress(BLUETOOTH_DEVICE_ADDRESS);
BLERemoteCharacteristic* pWriteChar = nullptr;
BLERemoteCharacteristic* pNotifyChar = nullptr;
BLEClient* pClient = nullptr;
boolean connected = false;
String responseBuffer = "";

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) { connected = true; }
  void onDisconnect(BLEClient* pclient) { connected = false; }
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                          uint8_t* pData, size_t length, bool isNotify) {
  String chunk = "";
  for (int i = 0; i < length; i++) {
    chunk += (char)pData[i];
  }
  responseBuffer += chunk;
}

bool connectToOBD() {
  BLEDevice::init("ESP32_OBD");
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  if (pClient->connect(obdAddress)) {
    BLERemoteService* pRemoteService = pClient->getService(BLUETOOTH_SERVICE_UUID);
    if (pRemoteService != nullptr) {
      pWriteChar = pRemoteService->getCharacteristic(BLUETOOTH_WRITE_CHAR_UUID);
      pNotifyChar = pRemoteService->getCharacteristic(BLUETOOTH_NOTIFY_CHAR_UUID);
      if (pWriteChar != nullptr && pNotifyChar != nullptr && pNotifyChar->canNotify()) {
        pNotifyChar->registerForNotify(notifyCallback);
        pNotifyChar->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)"\x01\x00", 2);
        return true;
      }
    }
  }
  return false;
}