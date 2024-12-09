#include <Wire.h>
#include <SparkFunLSM6DS3.h>

// Definir los pines I2C
#define SDA_PIN 8
#define SCL_PIN 9

// Crear un objeto LSM6DS3 en modo I2C
LSM6DS3 myIMU(I2C_MODE, 0x6B); // Cambia a 0x6B si es necesario

void setup() {
  Serial.begin(9600);
  delay(1000); // Esperar para que se inicialice la conexión serial
  Serial.println("Configurando IMU...");

  // Inicializar el bus I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Inicializar el IMU
  if (myIMU.begin() != 0) {
    Serial.println("IMU no inicializado correctamente.");
    while (1); // Detener el programa si no se puede inicializar
  } else {
    Serial.println("IMU inicializado correctamente.");
  }
}

void loop() {
  // Leer y mostrar la aceleración
  Serial.print("Aceleración X: ");
  Serial.print(myIMU.readFloatAccelX(), 4);
  Serial.print("\tY: ");
  Serial.print(myIMU.readFloatAccelY(), 4);
  Serial.print("\tZ: ");
  Serial.println(myIMU.readFloatAccelZ(), 4);

  // Leer y mostrar los valores del giroscopio
  Serial.print("Giroscopio X: ");
  Serial.print(myIMU.readFloatGyroX(), 4);
  Serial.print("\tY: ");
  Serial.print(myIMU.readFloatGyroY(), 4);
  Serial.print("\tZ: ");
  Serial.println(myIMU.readFloatGyroZ(), 4);

  delay(500); // Esperar medio segundo entre lecturas
}
