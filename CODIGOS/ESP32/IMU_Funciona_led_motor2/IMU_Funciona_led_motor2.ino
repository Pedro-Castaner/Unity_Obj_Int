#include <Wire.h>
#include <Adafruit_MPU6050.h>
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
#define MOTOR_PIN 11  // Pin donde está conectado el motor

Adafruit_MPU6050 myIMU;
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// UUIDs para BLE
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

// Función para cambiar el color del Neopixel
void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}
void controlMotor(bool state);  // Declaración de función para controlar el motor

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

// Callbacks para escritura en la característica
class MyNeoPixelCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pNeoPixelCharacteristic) {
    std::string value = pNeoPixelCharacteristic->getValue();
    if (value.length() == 4) {
      uint8_t r = value[0];
      uint8_t g = value[1];
      uint8_t b = value[2];
      bool motorState = value[3];  // Estado del motor (0 o 1)
      
      setNeopixelColor(r, g, b);
      controlMotor(motorState);  // Controlar el motor según el estado recibido
    } else {
      setNeopixelColor(125, 125, 0);
    }
  }
};

void setup() {
  Serial.begin(9600);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!myIMU.begin()) {
    Serial.println("No se pudo inicializar el MPU6050.");
    while (1);
  } else {
    Serial.println("MPU6050 inicializado correctamente.");
  }

  pixels.begin();
  setNeopixelColor(255, 0, 0);

  pinMode(MOTOR_PIN, OUTPUT);  // Configura el pin del motor como salida
  digitalWrite(MOTOR_PIN, LOW);  // Asegúrate de que el motor esté apagado inicialmente

  BLEDevice::init("ESP32 Neopixel Controller");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pIMUCharacteristic = pService->createCharacteristic(
      CHAR_UUID_IMU,
      BLECharacteristic::PROPERTY_NOTIFY
  );

    // Crear característica para el Neopixel
  pNeoPixelCharacteristic = pService->createCharacteristic(
                      CHAR_UUID_NEOPIXEL,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  // Modificación: Añadimos y configuramos el descriptor BLE2902
  BLE2902 *desc = new BLE2902();
  desc->setNotifications(true);  // Asegura que las notificaciones están habilitadas
  pIMUCharacteristic->addDescriptor(desc);

  pService->start();
  pServer->getAdvertising()->start();
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
    sensors_event_t accel, gyro, temp;
    myIMU.getEvent(&accel, &gyro, &temp);

    accelX = accel.acceleration.x;
    accelY = accel.acceleration.y;
    accelZ = accel.acceleration.z;
    gyroX = gyro.gyro.x;
    gyroY = gyro.gyro.y;
    gyroZ = gyro.gyro.z;

    byte imuData[24];
    memcpy(imuData, &accelX, sizeof(accelX));
    memcpy(imuData + 4, &accelY, sizeof(accelY));
    memcpy(imuData + 8, &accelZ, sizeof(accelZ));
    memcpy(imuData + 12, &gyroX, sizeof(gyroX));
    memcpy(imuData + 16, &gyroY, sizeof(gyroY));
    memcpy(imuData + 20, &gyroZ, sizeof(gyroZ));

    pIMUCharacteristic->setValue(imuData, sizeof(imuData));
    pIMUCharacteristic->notify();

    Serial.print("AX: "); Serial.print(accelX, 2); Serial.print(" ");
    Serial.print("AY: "); Serial.print(accelY, 2); Serial.print(" ");
    Serial.print("AZ: "); Serial.print(accelZ, 2); Serial.print(" ");
    Serial.print("GX: "); Serial.print(gyroX, 2); Serial.print(" ");
    Serial.print("GY: "); Serial.print(gyroY, 2); Serial.print(" ");
    Serial.print("GZ: "); Serial.println(gyroZ, 2);

    delay(1000);  // Incrementar intervalo para evitar saturación
  }
}
// Definición de la función para controlar el motor
void controlMotor(bool state) {
  if (state) {
    digitalWrite(MOTOR_PIN, HIGH);  // Enciende el motor
    Serial.println("Motor Encendido");
  } else {
    digitalWrite(MOTOR_PIN, LOW);   // Apaga el motor
    Serial.println("Motor Apagado");
  }
}
