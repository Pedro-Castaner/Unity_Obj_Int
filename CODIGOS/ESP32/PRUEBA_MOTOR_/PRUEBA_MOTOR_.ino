// Definir el pin del motor
#define MOTOR_PIN 11

void setup() {
  // Configurar el pin como salida
  pinMode(MOTOR_PIN, OUTPUT);
}

void loop() {
  // Encender el motor
  digitalWrite(MOTOR_PIN, HIGH);
  delay(3000);  // Esperar 3 segundos

  // Apagar el motor
  digitalWrite(MOTOR_PIN, LOW);
  delay(3000);  // Esperar 3 segundos
}
