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

// Pines del motor
#define MOTOR_PIN 5  // Modifica según tu conexión

Adafruit_MPU6050 myIMU;
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// UUIDs para BLE
#define SERVICE_UUID "A9E90000-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_IMU "A9E90001-194C-4523-A473-5FDF36AA4D18"

BLECharacteristic *pIMUCharacteristic;

float accelX, accelY, accelZ, gyroX, gyroY, gyroZ;

bool deviceConnected = false;
unsigned long previousMillis = 0;
const unsigned long imuInterval = 1000; // Intervalo de notificaciones por defecto (ms)
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

// Callback para manejar comandos recibidos por BLE
class CommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string command = pCharacteristic->getValue();
    
    if (command == "ON") {
      digitalWrite(MOTOR_PIN, HIGH);  // Motor ON
      setNeopixelColor(0, 255, 0);    // LED verde
    } else if (command == "OFF") {
      digitalWrite(MOTOR_PIN, LOW);   // Motor OFF
      setNeopixelColor(255, 0, 0);    // LED rojo
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
  setNeopixelColor(255, 0, 0);  // LED rojo por defecto

  pinMode(MOTOR_PIN, OUTPUT);  // Configuramos el pin del motor como salida
  digitalWrite(MOTOR_PIN, LOW);  // Motor apagado por defecto

  BLEDevice::init("ESP32 Neopixel Controller");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pIMUCharacteristic = pService->createCharacteristic(
      CHAR_UUID_IMU,
      BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
  );

  
  pIMUCharacteristic->setCallbacks(new CommandCallbacks());

  // Se añade el descriptor BLE2902 para permitir las notificaciones.
  BLE2902 *imuDescriptor = new BLE2902();
  pIMUCharacteristic->addDescriptor(imuDescriptor);

  pService->start();
  pServer->getAdvertising()->start();
}

void loop() {
  if (!deviceConnected) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;
      if (ledState) {
        setNeopixelColor(0, 0, 0);
      } else {
        setNeopixelColor(255, 0, 0);
      }
      ledState = !ledState;
    }
  } else {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= imuInterval) {
      previousMillis = currentMillis;

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
    }
  }
}
