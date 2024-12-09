#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 21  // Pin donde está conectado el Neopixel
#define NUMPIXELS 1  // Número de LEDs en el Neopixel

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);  // Inicialización del Neopixel

#define SERVICE_UUID "A9E90000-194C-4523-A473-5FDF36AA4D18"
#define CHAR_UUID_NEOPIXEL "A9E90001-194C-4523-A473-5FDF36AA4D18"


BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
unsigned long previousMillis = 0;  // Para manejar el tiempo del titileo
const long interval = 500;  // Intervalo de titileo (500 ms)

bool ledState = false;  // Estado actual del LED (encendido o apagado)

// Declaración de la función setNeopixelColor
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
    // El titileo en rojo se controlará en el loop principal cuando no esté conectado
  }
};

// Callback para la escritura en la característica
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      if (value[0] == 0x01) {
        setNeopixelColor(0, 255, 0);  // Verde para conectado
      } else {
        setNeopixelColor(255, 0, 0);  // Rojo para desconectado
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  pixels.begin();
  setNeopixelColor(255, 0, 0);  // Inicialmente en rojo (no conectado)

  BLEDevice::init("ESP32 Neopixel Controller");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
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
