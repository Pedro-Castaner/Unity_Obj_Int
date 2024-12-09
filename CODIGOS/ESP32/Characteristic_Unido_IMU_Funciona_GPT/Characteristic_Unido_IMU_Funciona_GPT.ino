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

// Pines del motor y NeoPixel
#define MOTOR_PIN 22
#define NEOPIXEL_PIN 21
#define NUMPIXELS 1

// UUIDs para BLE
#define SERVICE_UUID "A9E90000-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_IMU "A9E90001-194C-4523-A473-5FDF36AA4D18"

Adafruit_MPU6050 myIMU;
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

BLECharacteristic *pIMUCharacteristic;

bool deviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 500;
bool ledState = false;

// Función para cambiar el color del NeoPixel
void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

// Función para controlar el motor
void controlMotor(bool state) {
  digitalWrite(MOTOR_PIN, state ? HIGH : LOW);
  Serial.println(state ? "Motor Encendido" : "Motor Apagado");
}

// Callbacks para la conexión BLE
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Conectado a dispositivo BLE");
    setNeopixelColor(0, 255, 0);  // Verde: conectado
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Desconectado de dispositivo BLE");
    setNeopixelColor(255, 0, 0);  // Rojo: desconectado
  }
};

// Callbacks para la característica BLE
class MyIMUCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 4) {
      uint8_t r = value[0];
      uint8_t g = value[1];
      uint8_t b = value[2];
      bool motorState = value[3];  // Estado del motor (0 o 1)

      setNeopixelColor(r, g, b);
      controlMotor(motorState);
    } else {
      Serial.println("Error: Datos recibidos con longitud incorrecta");
      setNeopixelColor(125, 125, 0);  // Amarillo: error
    }
  }
};

// Configuración inicial
void setup() {
  Serial.begin(9600);
  delay(1000);

  // Configuración de pines
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);  // Motor apagado inicialmente

  // Inicialización del MPU6050
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!myIMU.begin()) {
    Serial.println("No se pudo inicializar el MPU6050.");
    while (1);
  } else {
    Serial.println("MPU6050 inicializado correctamente.");
  }

  // Inicialización del NeoPixel
  pixels.begin();
  setNeopixelColor(255, 0, 0);  // Rojo: esperando conexión

  // Inicialización BLE
  BLEDevice::init("ESP32 IMU Controller");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pIMUCharacteristic = pService->createCharacteristic(
    CHAR_UUID_IMU,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
  );

  BLE2902 *descIMU = new BLE2902();
  descIMU->setNotifications(true);
  pIMUCharacteristic->addDescriptor(descIMU);
  pIMUCharacteristic->setCallbacks(new MyIMUCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Esperando conexión BLE...");
}

// Función para manejar la desconexión
void manejarDesconexion() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    setNeopixelColor(ledState ? 0 : 255, 0, 0);  // Alternar rojo apagado/encendido
    ledState = !ledState;
  }
}

// Función para enviar datos IMU
void enviarDatosIMU() {
  sensors_event_t accel, gyro, temp;
  myIMU.getEvent(&accel, &gyro, &temp);

  // Crear paquete de datos
  byte imuData[24];
  memcpy(imuData, &accel.acceleration.x, sizeof(accel.acceleration.x));
  memcpy(imuData + 4, &accel.acceleration.y, sizeof(accel.acceleration.y));
  memcpy(imuData + 8, &accel.acceleration.z, sizeof(accel.acceleration.z));
  memcpy(imuData + 12, &gyro.gyro.x, sizeof(gyro.gyro.x));
  memcpy(imuData + 16, &gyro.gyro.y, sizeof(gyro.gyro.y));
  memcpy(imuData + 20, &gyro.gyro.z, sizeof(gyro.gyro.z));

  pIMUCharacteristic->setValue(imuData, sizeof(imuData));
  pIMUCharacteristic->notify();

  Serial.print("AX: "); Serial.print(accel.acceleration.x, 2); Serial.print(" ");
  Serial.print("AY: "); Serial.print(accel.acceleration.y, 2); Serial.print(" ");
  Serial.print("AZ: "); Serial.print(accel.acceleration.z, 2); Serial.print(" ");
  Serial.print("GX: "); Serial.print(gyro.gyro.x, 2); Serial.print(" ");
  Serial.print("GY: "); Serial.print(gyro.gyro.y, 2); Serial.print(" ");
  Serial.print("GZ: "); Serial.println(gyro.gyro.z, 2);
}

// Bucle principal
void loop() {
  if (!deviceConnected) {
    manejarDesconexion();
  } else {
    enviarDatosIMU();
    delay(1000);  // Ajusta el intervalo según sea necesario
  }
}
