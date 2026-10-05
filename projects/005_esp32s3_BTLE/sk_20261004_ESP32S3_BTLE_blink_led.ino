#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* ===== Onboard LED Pin ===== */
// Change this pin if your ESP32-S3 board uses a different GPIO for the onboard LED
const int LED_PIN = 4; 

/* ===== Nordic UART Service UUIDs ===== */
// Standard Nordic UART Service UUIDs recognized automatically by Serial Bluetooth Terminal
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
bool deviceConnected = false;

/* ===== BLE Server Connection Callbacks ===== */
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Mobile phone connected over BLE!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Mobile phone disconnected.");
      // Restart advertising so you can reconnect anytime
      BLEDevice::startAdvertising();
    }
};

/* ===== BLE Data Receive Callbacks ===== */
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue().c_str();
      
      // Trim carriage return / line feed characters sent by terminal apps
      rxValue.trim();

      if (rxValue.length() > 0) {
        Serial.print("Received Command: ");
        Serial.println(rxValue);

        // Turn On LED
        if (rxValue.equalsIgnoreCase("ON")) {
          digitalWrite(LED_PIN, HIGH);
          Serial.println("LED turned ON");
        } 
        // Turn Off LED
        else if (rxValue.equalsIgnoreCase("OFF")) {
          digitalWrite(LED_PIN, LOW);
          Serial.println("LED turned OFF");
        }
      }
    }
};

void setup() {
  Serial.begin(115200);

  // Configure LED Pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Start with LED OFF

  // 1. Initialize BLE
  BLEDevice::init("ESP32S3_LED_Ctrl");

  // 2. Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 3. Create Nordic UART Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 4. Create RX Characteristic (For receiving commands from mobile)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE |
                                           BLECharacteristic::PROPERTY_WRITE_NR
                                         );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // 5. Create TX Characteristic (For sending status back to mobile)
  pTxCharacteristic = pService->createCharacteristic(
                                 CHARACTERISTIC_UUID_TX,
                                 BLECharacteristic::PROPERTY_NOTIFY
                               );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // 6. Start Service
  pService->start();

  // 7. Start Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE Ready! Open 'Serial Bluetooth Terminal' -> BLUETOOTH LE tab -> Connect to 'ESP32S3_LED_Ctrl'.");
}

void loop() {
  // Everything runs asynchronously in the BLE callbacks
  delay(1000);
}