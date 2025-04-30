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

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  Serial.println("-- Received Response --");
  String chunk = "";
  for (int i = 0; i < length; i++) {
    chunk += (char)pData[i];
  }
  responseBuffer += chunk;
  Serial.println("responseBuffer: " + responseBuffer);
}

void intializeELM327() {
  // Send initialization commands to ELM327
  Serial.println("Sending command: AT D\\r");
  pWriteChar->writeValue("AT D\r");     // Reset to default

  Serial.println("Sending command: AT Z\\r");
  pWriteChar->writeValue("AT Z\r");     // Reset adapter

  Serial.println("Sending command: AT E0\\r");
  pWriteChar->writeValue("AT E0\r");    // Echo off

  Serial.println("Sending command: AT L0\\r");
  pWriteChar->writeValue("AT L0\r");    // Linefeeds off

  Serial.println("Sending command: AT S0\\r"); 
  pWriteChar->writeValue("AT S0\r");    // Spaces off

  Serial.println("Sending command: AT H0\\r"); 
  pWriteChar->writeValue("AT H0\r");    // Headers off

  Serial.println("Sending command: AT SP 0\\r"); 
  pWriteChar->writeValue("AT SP 0\r");  // Set auto protocol

  Serial.println("Sending command: AT AL0\\r"); 
  pWriteChar->writeValue("AT AL0\r");   // Turn adaptive timing off

  Serial.println("Sending command: AT ST 10\\r");
  pWriteChar->writeValue("AT ST 10\r"); // Set optimal timeout to 160ms
}

bool connectToOBD() {
  // Initialize
  Serial.println("Intializing BLEDevice");
  BLEDevice::init("ESP32_OBD");
  BLEDevice::setMTU(200);
  BLEDevice::setPower(ESP_PWR_LVL_P9);
  Serial.println("Done initializing BLEDevice");

  // Create BLE client
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  Serial.println("Attempting device connection...");
  if (pClient->connect(obdAddress)) {
    Serial.println("Device connected");
    pClient->setMTU(200);
    BLERemoteService* pRemoteService = pClient->getService(BLUETOOTH_SERVICE_UUID);
    if (pRemoteService != nullptr) {
      pWriteChar = pRemoteService->getCharacteristic(BLUETOOTH_WRITE_CHAR_UUID);
      pNotifyChar = pRemoteService->getCharacteristic(BLUETOOTH_NOTIFY_CHAR_UUID);
      if (pWriteChar != nullptr && pNotifyChar != nullptr && pNotifyChar->canNotify()) {
        pNotifyChar->registerForNotify(notifyCallback);
        pNotifyChar->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)"\x01\x00", 2);
        Serial.println("Initializing ELM327");
        intializeELM327();
        return true;
      }
    }
  }
  return false;
}