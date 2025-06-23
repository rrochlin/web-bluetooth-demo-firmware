#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

struct DataPack {
  uint32_t timesstamp;
  float x_axis;
  float y_axis;
  float z_axis;
  bool is_calibrated;
};

float value = 0;
void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32_CoreSync_Test");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now advertising...");
}

void loop() {
  if (deviceConnected) {
    for (int i=0; i<10; i++) {
      DataPack my_data;
      my_data.timesstamp = millis();
      my_data.x_axis = sin(value/10.0) * 100.0;
      my_data.y_axis = cos(value/10.0) * 100.0;
      my_data.z_axis = sin(value/5.0) * 50.0;
      my_data.is_calibrated = true;

      pCharacteristic->setValue((uint8_t *)&my_data, sizeof(my_data));
      pCharacteristic->notify();
      value += 0.1;
    }
    delay(100);
  } else {
    Serial.println("Device is not connected");
    delay(1000);
  }
}