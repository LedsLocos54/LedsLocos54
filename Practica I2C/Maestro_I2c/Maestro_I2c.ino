#include <Wire.h>

void setup() {
  Wire.begin();        // Iniciar la comunicación I2C
}

void loop() {
  int valor = 42;      // Valor a enviar
  Wire.beginTransmission(8); // Dirección del dispositivo esclavo
  Wire.write(valor);   // Escribir el valor en el bus I2C
  Wire.endTransmission();   // Terminar la transmisión
  delay(500);          // Esperar un poco antes de enviar otro valor
}
