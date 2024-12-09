#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 21  // Pin donde está conectado el Neopixel
#define NUMPIXELS 1  // Número de LEDs en el Neopixel


Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);  // Inicialización del Neopixel

// Definimos los pines de los motores
int motores[] = {3, 12,11,4,10,9, 5, 8, 7, 6};
int numMotores = 10;
int fadeAmount = 5;
int delayEntreMotores = 0; // Tiempo entre motores
int duracionVibracion = 200; // Duración de vibración en milisegundos

int secMot = 0; // Variable para seleccionar la secuencia de motores

// Configuración del PWM en el ESP32-S3
void configurarPWM(int pin, int canal) {
  ledcAttachPin(pin, canal);
  ledcSetup(canal, 5000, 8); // Frecuencia de 5 kHz y resolución de 8 bits
}

#define SERVICE_UUID "A9E90000-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_NEOPIXEL "A9E90001-194C-4523-A473-5FDF36AA4D18"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
unsigned long previousMillis = 0;  // Para manejar el tiempo del titileo
const long interval = 500;  // Intervalo de titileo (500 ms)

bool ledState = false;  // Estado actual del LED (encendido o apagado)

// Declaración de funciones
void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b);
void controlMotores(int motorState);


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
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 4) {  // Espera cuatro bytes (RGB + estado del motor)
      uint8_t r = value[0];
      uint8_t g = value[1];
      uint8_t b = value[2];
      int motorState = value[3];  // Estado del motor (0, 1 o 2)

      setNeopixelColor(r, g, b);  // Cambiar color según los valores RGB recibidos
      controlMotores(motorState);  // Controlar el motor según el estado recibido
    } else {
      setNeopixelColor(125, 125, 0);  // Amarillo si hay algún problema con los datos
    }
  }
};

void setup() {
  Serial.begin(9600);
  pixels.begin();
  setNeopixelColor(255, 0, 0);  // Inicialmente en rojo (no conectado)

  // Configuramos cada motor en su canal PWM
  for (int i = 0; i < numMotores; i++) {
    configurarPWM(motores[i], i); // Cada motor con un canal único
  }
  apagarMotores();

  BLEDevice::init("ESP32 Neopixel Controller");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Crear característica para el Neopixel
  pCharacteristic = pService->createCharacteristic(
                      CHAR_UUID_NEOPIXEL,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());

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
  }
}

// Definición de la función setNeopixelColor
void setNeopixelColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

// Definición de la función para controlar el motor
void controlMotores(int motorState) {
  if (motorState == 0) {
    apagarMotores();  // Llamar a la función para apagar los motores
    Serial.println("Apagando motores...");
  } else if (motorState == 1) {
    cicloMovimiento1();  // Llamar al ciclo de movimiento 1
    Serial.println("Ejecutando ciclo de movimiento 1...");
  } else if (motorState == 2) {
    cicloMovimiento2();  // Llamar al ciclo de movimiento 2
    Serial.println("Ejecutando ciclo de movimiento 2...");
  } else {
    Serial.println("Estado del motor no válido.");
  }
}

// Ciclo de movimiento 1
void cicloMovimiento1() {
  for (int i = 0; i < numMotores; i++) {
    ledcWrite(i, 155 + (10 * i)); // Vibración progresiva
    delay(duracionVibracion); // Mantener la vibración
    ledcWrite(i, 0); // Apagar el motor
    delay(delayEntreMotores); // Pausa entre motores
  }
}

// Ciclo de movimiento 2
void cicloMovimiento2() {
  for (int i = 0; i < numMotores; i++) {
    ledcWrite(i, 255); // Vibración máxima
    delay(duracionVibracion); // Mantener la vibración
  }
}

// Apagar todos los motores
void apagarMotores() {
  for (int i = 0; i < numMotores; i++) {
    ledcWrite(i, 0); // Apagar motor
  }
}
