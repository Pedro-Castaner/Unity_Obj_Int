#include <Wire.h>
#include <SparkFunLSM6DS3.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

// Pines de I2C
#define SDA_PIN 1
#define SCL_PIN 2

// Pines de NeoPixel
#define NEOPIXEL_PIN 21
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
LSM6DS3 myIMU(I2C_MODE, 0x6B); // Dirección I2C estándar del LSM6DS3

// UUIDs para BLE
#define SERVICE_UUID "A9E90000-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_IMU "A9E90001-194C-4523-A473-5FDF36AA4D18"
BLECharacteristic *pIMUCharacteristic;

bool deviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 500;
bool ledState = false;

// Función para cambiar el color del Neopixel
void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

// Callbacks para conexión BLE
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

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inicializar I2C y el sensor LSM6DS3
  Wire.begin(SDA_PIN, SCL_PIN);
  if (myIMU.begin() != 0) {
    Serial.println("No se pudo inicializar el LSM6DS3.");
    while (1);
  } else {
    Serial.println("LSM6DS3 inicializado correctamente.");
  }

  // Inicializar NeoPixel
  pixels.begin();
  setNeopixelColor(255, 0, 0);

  // Configurar BLE
  BLEDevice::init("ESP32 LSM6DS3");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pIMUCharacteristic = pService->createCharacteristic(
      CHAR_UUID_IMU,
      BLECharacteristic::PROPERTY_NOTIFY
  );

  BLE2902 *desc = new BLE2902();
  desc->setNotifications(true);
  pIMUCharacteristic->addDescriptor(desc);

  pService->start();
  pServer->getAdvertising()->start();
}

void loop() {
  if (!deviceConnected) {
    // Parpadeo del NeoPixel cuando no hay conexión
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if (ledState) {
        setNeopixelColor(0, 0, 0);
      } else {
        setNeopixelColor(255, 0, 0);
      }
      ledState = !ledState;
    }
  } else {
    // Leer datos del LSM6DS3
    float accelX = myIMU.readFloatAccelX();
    float accelY = myIMU.readFloatAccelY();
    float accelZ = myIMU.readFloatAccelZ();
    float gyroX = myIMU.readFloatGyroX();
    float gyroY = myIMU.readFloatGyroY();
    float gyroZ = myIMU.readFloatGyroZ();

    // Enviar datos por BLE
    byte imuData[24];
    memcpy(imuData, &accelX, sizeof(accelX));
    memcpy(imuData + 4, &accelY, sizeof(accelY));
    memcpy(imuData + 8, &accelZ, sizeof(accelZ));
    memcpy(imuData + 12, &gyroX, sizeof(gyroX));
    memcpy(imuData + 16, &gyroY, sizeof(gyroY));
    memcpy(imuData + 20, &gyroZ, sizeof(gyroZ));

    pIMUCharacteristic->setValue(imuData, sizeof(imuData));
    pIMUCharacteristic->notify();

    // Imprimir valores en el monitor serie
    Serial.print("AX: "); Serial.print(accelX, 2); Serial.print(" ");
    Serial.print("AY: "); Serial.print(accelY, 2); Serial.print(" ");
    Serial.print("AZ: "); Serial.print(accelZ, 2); Serial.print(" ");
    Serial.print("GX: "); Serial.print(gyroX, 2); Serial.print(" ");
    Serial.print("GY: "); Serial.print(gyroY, 2); Serial.print(" ");
    Serial.print("GZ: "); Serial.println(gyroZ, 2);

    delay(1000);  // Reducir frecuencia para evitar saturación
  }
}
