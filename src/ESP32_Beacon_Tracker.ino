#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define MAX_BEACONS 50

struct BeaconInfo {
  String macAddress;
  String carReg;
  int rssi;
  bool registered;
};

BeaconInfo beacons[MAX_BEACONS];
int beaconCount = 0;

unsigned long scanInterval = 5000; // Scan every 5 seconds
unsigned long lastScanTime = 0;

BLEScan* pBLEScan;

int findBeacon(String mac) {
  for (int i = 0; i < beaconCount; i++) {
    if (beacons[i].macAddress == mac) {
      return i;
    }
  }
  return -1;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String mac = String(advertisedDevice.getAddress().toString().c_str());
    int index = findBeacon(mac);

    if (index != -1) {
      beacons[index].rssi = advertisedDevice.getRSSI();
    } else if (beaconCount < MAX_BEACONS) {
      beacons[beaconCount].macAddress = mac;
      beacons[beaconCount].rssi = advertisedDevice.getRSSI();
      beacons[beaconCount].registered = false;
      beacons[beaconCount].carReg = "";
      beaconCount++;
    }
  }
};

void displayRegisteredBeacons() {
  Serial.println("========== Registered Beacons ==========");
  bool found = false;

  for (int i = 0; i < beaconCount; i++) {
    if (beacons[i].registered) {
      Serial.print("Car Reg: ");
      Serial.print(beacons[i].carReg);
      Serial.print(" | MAC: ");
      Serial.print(beacons[i].macAddress);
      Serial.print(" | RSSI: ");
      Serial.print(beacons[i].rssi);
      Serial.println(" dBm");
      found = true;
    }
  }

  if (!found) {
    Serial.println("No registered beacons detected.");
  }
  Serial.println("=========================================");
}

void displayUnregisteredBeacons() {
  Serial.println("------ Unregistered Beacons Nearby ------");
  for (int i = 0; i < beaconCount; i++) {
    if (!beacons[i].registered) {
      Serial.println(beacons[i].macAddress);
    }
  }
  Serial.println("-----------------------------------------");
}

void registerBeacon() {
  Serial.println("Enter MAC address to register:");
  while (Serial.available() == 0) {}
  String macInput = Serial.readStringUntil('\n');
  macInput.trim();

  int index = findBeacon(macInput);
  if (index == -1) {
    Serial.println("MAC address not found.");
    return;
  }

  Serial.println("Enter Car Registration:");
  while (Serial.available() == 0) {}
  String carRegInput = Serial.readStringUntil('\n');
  carRegInput.trim();

  beacons[index].carReg = carRegInput;
  beacons[index].registered = true;
  Serial.println("Beacon registered successfully!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Beacon Tracker...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void loop() {
  if (millis() - lastScanTime > scanInterval) {
    Serial.println("Scanning...");
    pBLEScan->clearResults();
    pBLEScan->start(3, false);
    lastScanTime = millis();

    displayRegisteredBeacons();
    displayUnregisteredBeacons();

    Serial.println("Type 'register' to register a new beacon, or press Enter to continue.");
    unsigned long waitStart = millis();

    while (millis() - waitStart < 5000) {
      if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "register") {
          registerBeacon();
        }
        break;
      }
    }
  }
}

