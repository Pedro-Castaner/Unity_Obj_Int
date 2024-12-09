#include <Wire.h>
#include <Adafruit_MPU6050.h>  // Librería de Adafruit para el MPU6050
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

// Pines de I2C
#define SDA_PIN 8
#define SCL_PIN 9

// Pines de NeoPixel
#define NEOPIXEL_PIN 21  // Pin donde está conectado el Neopixel
#define NUMPIXELS 1  // Número de LEDs en el Neopixel

// Crear objetos MPU6050 y Neopixel
Adafruit_MPU6050 myIMU;
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);  // Inicialización del Neopixel

// UUID para BLE
#define SERVICE_UUID "A9E90000-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_IMU "A9E90001-194C-4523-A473-5FDF36AA4D18"  // Nueva característica para datos IMU
BLECharacteristic *pNeoPixelCharacteristic;
BLECharacteristic *pIMUCharacteristic;

// Variables para almacenar datos del IMU
float accelX, accelY, accelZ, gyroX, gyroY, gyroZ;

bool deviceConnected = false;
unsigned long previousMillis = 0;  // Para manejar el tiempo del titileo
const long interval = 500;  // Intervalo de titileo (500 ms)

bool ledState = false;  // Estado actual del LED (encendido o apagado)

// Declaración de la función setNeopixelColor para valores RGB
void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b);


// Callback para la conexión BLE
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Conectado a dispositivo BLE");
    setNeopixelColor(0, 255, 0);  // Verde cuando está conectado
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Desconectado de dispositivo BLE");
    setNeopixelColor(255, 0, 0);  // Rojo cuando no está conectado
  }
};

// Callback para la escritura en la característica
class MyNeoPixelCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 4) {  // Espera cuatro bytes (RGB + estado del motor)
      uint8_t r = value[0];
      uint8_t g = value[1];
      uint8_t b = value[2];
      bool motorState = value[3];  // Estado del motor (0 o 1)

      setNeopixelColor(r, g, b);  // Cambiar color según los valores RGB recibidos
    } else {
      setNeopixelColor(125, 125, 0);  // Amarillo si hay algún problema con los datos
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
  setNeopixelColor(255, 0, 0);  // Inicialmente en rojo (no conectado)

  BLEDevice::init("ESP32 Neopixel Controller");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Crear característica para enviar datos IMU
  pIMUCharacteristic = pService->createCharacteristic(
      CHAR_UUID_IMU,
      BLECharacteristic::PROPERTY_NOTIFY
  );

  // Agregar BLE2902 a la característica pIMUCharacteristic
  pIMUCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  pServer->getAdvertising()->start();
}

void loop() {
  // Si no está conectado, hacer titilar la luz roja
  if (!deviceConnected) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      // Guardar el último tiempo en que cambió el estado
      previousMillis = currentMillis;

      // Alternar el estado del LED (titileo)
      if (ledState) {
        setNeopixelColor(0, 0, 0);  // Apagar el LED
      } else {
        setNeopixelColor(255, 0, 0);  // Encender en rojo
      }

      // Cambiar el estado
      ledState = !ledState;
    }
  } else {
    // Leer datos del IMU usando la librería de Adafruit
    sensors_event_t accel, gyro, temp;
    myIMU.getEvent(&accel, &gyro, &temp);

    accelX = accel.acceleration.x;
    accelY = accel.acceleration.y;
    accelZ = accel.acceleration.z;
    gyroX = gyro.gyro.x;
    gyroY = gyro.gyro.y;
    gyroZ = gyro.gyro.z;

    // Crear un array de bytes para los datos del IMU
    byte imuData[24]; // 6 floats * 4 bytes por float = 24 bytes

    // Convertir cada float a bytes y almacenarlos en el array
    memcpy(imuData, &accelX, sizeof(accelX));
    memcpy(imuData + 4, &accelY, sizeof(accelY));
    memcpy(imuData + 8, &accelZ, sizeof(accelZ));
    memcpy(imuData + 12, &gyroX, sizeof(gyroX));
    memcpy(imuData + 16, &gyroY, sizeof(gyroY));
    memcpy(imuData + 20, &gyroZ, sizeof(gyroZ));

    // Enviar los datos como un array de bytes a través de BLE
    pIMUCharacteristic->setValue(imuData, sizeof(imuData));
    pIMUCharacteristic->notify();

    // Imprimir los datos como un string (para depuración)
    Serial.print("AX: "); Serial.print(accelX, 2); Serial.print(" ");
    Serial.print("AY: "); Serial.print(accelY, 2); Serial.print(" ");
    Serial.print("AZ: "); Serial.print(accelZ, 2); Serial.print(" ");
    Serial.print("GX: "); Serial.print(gyroX, 2); Serial.print(" ");
    Serial.print("GY: "); Serial.print(gyroY, 2); Serial.print(" ");
    Serial.print("GZ: "); Serial.println(gyroZ, 2);

    delay(500);  // Ajustar para controlar la frecuencia de envío
  }
}

// Definición de la función setNeopixelColor
void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}
