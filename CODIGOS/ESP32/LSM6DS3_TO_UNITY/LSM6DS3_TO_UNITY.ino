#include <Wire.h>
#include <SparkFunLSM6DS3.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_NeoPixel.h>

#define SDA_PIN 8
#define SCL_PIN 9

#define NEOPIXEL_PIN 21  
#define NUMPIXELS 1  
#define MOTOR_PIN 12  

LSM6DS3 myIMU(I2C_MODE, 0x6B);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);  

#define SERVICE_UUID "A9E90000-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_NEOPIXEL "A9E90001-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_IMU "A9E90002-194C-4523-A473-5FDF36AA4D18"  

BLECharacteristic *pNeoPixelCharacteristic;
BLECharacteristic *pIMUCharacteristic;

float accelX, accelY, accelZ, gyroX, gyroY, gyroZ;

bool deviceConnected = false;
unsigned long previousMillis = 0;  
const long interval = 500;  

bool ledState = false;

void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b);
void controlMotor(bool state);

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Conectado a dispositivo BLE");
    setNeopixelColor(0, 255, 0);  
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Desconectado de dispositivo BLE");
    setNeopixelColor(255, 0, 0);  
  }
};

class MyNeoPixelCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 4) {
      uint8_t r = value[0];
      uint8_t g = value[1];
      uint8_t b = value[2];
      bool motorState = value[3];

      setNeopixelColor(r, g, b);
      controlMotor(motorState);
    }
  }
};

class MyIMUCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    if (myIMU.available()) {
      accelX = myIMU.readFloatAccelX();
      accelY = myIMU.readFloatAccelY();
      accelZ = myIMU.readFloatAccelZ();
      gyroX = myIMU.readFloatGyroX();
      gyroY = myIMU.readFloatGyroY();
      gyroZ = myIMU.readFloatGyroZ();

      String imuData = String(accelX) + "," + String(accelY) + "," + String(accelZ) + "," +
                       String(gyroX) + "," + String(gyroY) + "," + String(gyroZ);

      pIMUCharacteristic->setValue(imuData.c_str());
      pIMUCharacteristic->notify();
    }
  }
};

void setup() {
  Serial.begin(9600);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!myIMU.begin()) {
    Serial.println("No se pudo inicializar el IMU");
    while (1);
  }
  pixels.begin();

  BLEDevice::init("ESP32 Neopixel Controller");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pNeoPixelCharacteristic = pService->createCharacteristic(
    CHAR_UUID_NEOPIXEL,
    BLECharacteristic::PROPERTY_WRITE
  );
  pNeoPixelCharacteristic->setCallbacks(new MyNeoPixelCallbacks());

  pIMUCharacteristic = pService->createCharacteristic(
    CHAR_UUID_IMU,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pIMUCharacteristic->setCallbacks(new MyIMUCallbacks());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("Esperando conexiones BLE...");
}

void loop() {
  if (deviceConnected) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      pIMUCharacteristic->notify();
    }
  }
}

void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

void controlMotor(bool state) {
  if (state) {
    digitalWrite(MOTOR_PIN, HIGH);
  } else {
    digitalWrite(MOTOR_PIN, LOW);
  }
}
