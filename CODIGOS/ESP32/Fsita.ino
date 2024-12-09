// Definimos los pines de los motores
int motores[] = {7, 8, 9, 10, 11, 12, 13};
int numMotores = 7;
int fadeAmount = 5;
int delayEntreMotores = 0; // Tiempo entre motores
int duracionVibracion = 200; // Duración de vibración en milisegundos (2 segundos)

// Variable para almacenar el comando recibido
String comando = "";

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
  Serial.begin(9600); // Inicializamos la comunicación serial
}

void loop() {
  // Verificar si hay datos disponibles en el puerto serial
  if (Serial.available() > 0) {
    comando = Serial.readStringUntil('\n'); // Leer el comando hasta el salto de línea
    comando.trim(); // Elimina espacios en blanco y saltos de línea
  }

  if (comando == "ciclo1") {
    cicloMovimiento1();
    comando = ""; // Restablecer comando después de ejecutarlo
  } else if (comando == "ciclo2") {
    cicloMovimiento2();
    comando = ""; // Restablecer comando después de ejecutarlo
  } else {
    apagarMotores(); // Apaga todos los motores si no hay un comando válido
  }
}

// Ciclo de movimiento 1
void cicloMovimiento1() {
  for (int i = 0; i < numMotores; i++) {
    ledcWrite(i, 255); // Vibración máxima
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
}}

// Apagar todos los motores
void apagarMotores() {
  for (int i = 0; i < numMotores; i++) {
    ledcWrite(i, 0); // Apagar motor
  }
}