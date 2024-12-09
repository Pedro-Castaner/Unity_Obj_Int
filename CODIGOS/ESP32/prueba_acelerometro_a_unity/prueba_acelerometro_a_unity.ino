#include <Wire.h>
#include <SparkFunLSM6DS3.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 21  // Pin donde está conectado el Neopixel
#define NUMPIXELS 1      // Número de LEDs en el Neopixel
#define MOTOR_PIN 12     // Pin donde está conectado el motor

// Inicialización del Neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define SERVICE_UUID "A9E90000-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_NEOPIXEL "A9E90001-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_ACCELEROMETER "A9E90002-194C-4523-A473-5FDF36AA4D18" // Característica para datos del acelerómetro

BLECharacteristic *pCharacteristicNeoPixel;
BLECharacteristic *pCharacteristicAccelerometer;
bool deviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 500;

bool ledState = false;

// Configuración del acelerómetro
#define SDA_PIN 8
#define SCL_PIN 9
LSM6DS3 myIMU(I2C_MODE, 0x6B);  // Cambiar a 0x6B si es necesario

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

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 4) {
      uint8_t r = value[0];
      uint8_t g = value[1];
      uint8_t b = value[2];
      bool motorState = value[3];

      setNeopixelColor(r, g, b);
      controlMotor(motorState);
    } else {
      setNeopixelColor(125, 125, 0);
    }
  }
};

void setup() {
  Serial.begin(115200);
  pixels.begin();
  setNeopixelColor(255, 0, 0);

  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  BLEDevice::init("ESP32 Neopixel & Accelerometer");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Crear características
  pCharacteristicNeoPixel = pService->createCharacteristic(
                             CHAR_UUID_NEOPIXEL,
                             BLECharacteristic::PROPERTY_WRITE
                           );
  pCharacteristicNeoPixel->setCallbacks(new MyCallbacks());

  pCharacteristicAccelerometer = pService->createCharacteristic(
                                   CHAR_UUID_ACCELEROMETER,
                                   BLECharacteristic::PROPERTY_NOTIFY
                                 );

  pService->start();
  pServer->getAdvertising()->start();

  // Configuración del acelerómetro
  Wire.begin(SDA_PIN, SCL_PIN);
  if (myIMU.begin() != 0) {
    Serial.println("IMU no inicializado correctamente.");
    while (1);
  } else {
    Serial.println("IMU inicializado correctamente.");
  }
}

void loop() {
  if (!deviceConnected) {
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
    // Leer datos del acelerómetro y enviarlos por BLE
    float accelX = myIMU.readFloatAccelX();
    float accelY = myIMU.readFloatAccelY();
    float accelZ = myIMU.readFloatAccelZ();

    char accelData[12];
    sprintf(accelData, "%.2f,%.2f,%.2f", accelX, accelY, accelZ);
    pCharacteristicAccelerometer->setValue((uint8_t*)accelData, strlen(accelData));
    pCharacteristicAccelerometer->notify();

    delay(500); // Intervalo para enviar datos
  }
}

void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

void controlMotor(bool state) {
  if (state) {
    digitalWrite(MOTOR_PIN, HIGH);
    Serial.println("Motor Encendido");
  } else {
    digitalWrite(MOTOR_PIN, LOW);
    Serial.println("Motor Apagado");
  }
}
