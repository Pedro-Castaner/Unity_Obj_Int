#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Definimos los pines de los motores
int motores[] = {7, 8, 9, 10, 11, 12, 13};
int numMotores = 7;
int fadeAmount = 5;
int delayEntreMotores = 0; // Tiempo entre motores
int duracionVibracion = 200; // Duración de vibración en milisegundos

// Pines físicos en la ESP32-S3 Mini para SDA y SCL
#define SDA_PIN 1  // Pin físico 8
#define SCL_PIN 2  // Pin físico 9

Adafruit_MPU6050 mpu;

// Configuración del PWM en el ESP32-S3
void configurarPWM(int pin, int canal) {
  ledcAttachPin(pin, canal);
  ledcSetup(canal, 5000, 8); // Frecuencia de 5 kHz y resolución de 8 bits
}

void setup() {
  // Configuramos cada motor en su canal PWM
  for (int i = 0; i < numMotores; i++) {
    configurarPWM(motores[i], i); // Cada motor con un canal único
  }

  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit MPU6050 test!");

  // Inicia I2C en los pines especificados
  Wire.begin(SDA_PIN, SCL_PIN);

  // Inicializa el MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  delay(100);
}

void loop() {
  // Obtiene nuevos eventos del sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Imprime los valores de aceleración
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.println(a.acceleration.z);

  // Activar ciclos de vibración según las lecturas
  if (a.acceleration.x > 5.0) {
    cicloMovimiento1();
  } else {
    apagarMotores();
  }

  delay(500); // Ajustar según la frecuencia deseada
}

// Ciclo de movimiento 1
void cicloMovimiento1() {
  for (int i = 0; i < numMotores; i++) {
    ledcWrite(i, 155+(10*i)); // Vibración máxima
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
