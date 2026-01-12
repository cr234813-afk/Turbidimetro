// #include <U8g2lib.h>
// #include <Wire.h>

// #define SENSOR_ANALOG_PIN A0  // Pin analógico del sensor de turbidez
// #define SENSOR_DIGITAL_PIN 3  // Pin digital del sensor de turbidez
// #define RESET_BUTTON 2        // Pin digital para el botón de reset

// // Configurar pantalla OLED SSD1306, 128x32, I2C
// U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// void setup() {
//   // Configurar pines
//   pinMode(RESET_BUTTON, INPUT_PULLUP);      // Botón con pull-up interno
//   pinMode(SENSOR_DIGITAL_PIN, INPUT);       // Salida digital del sensor
  
//   // Inicializar comunicación serial para depuración
//   Serial.begin(9600);
  
//   // Inicializar pantalla OLED
//   u8g2.begin();
  
//   // Mostrar mensaje inicial
//   u8g2.clearBuffer();
//   u8g2.setFont(u8g2_font_ncenB08_tr); // Fuente pequeña para 128x32
//   u8g2.drawStr(0, 10, "Turbidimetro listo");
//   u8g2.sendBuffer();
//   delay(2000); // Mostrar mensaje inicial por 2 segundos
// }

// void loop() {
//   // Verificar si el botón de reset está presionado
//   if (digitalRead(RESET_BUTTON) == LOW) {
//     u8g2.clearBuffer();
//     u8g2.setFont(u8g2_font_ncenB08_tr);
//     u8g2.drawStr(0, 10, "Reseteando...");
//     u8g2.sendBuffer();
//     delay(1000);
    
//     // Volver al mensaje inicial
//     u8g2.clearBuffer();
//     u8g2.drawStr(0, 10, "Turbidimetro listo");
//     u8g2.sendBuffer();
//     delay(2000);
//   } else {
//     // Leer valor analógico
//     int sensorValue = analogRead(SENSOR_ANALOG_PIN);
//     float voltage = sensorValue * (5.0 / 1023.0);
//     float turbidity = 100 - (voltage * 20); // Fórmula aproximada
//     if (turbidity < 0) turbidity = 0;       // Evitar valores negativos
    
//     // Leer valor digital
//     int digitalValue = digitalRead(SENSOR_DIGITAL_PIN);
//     String digitalStatus = digitalValue == HIGH ? "Baja" : "Alta";
    
//     // Convertir turbidez a cadena para mostrar
//     char turbidityStr[10];
//     dtostrf(turbidity, 4, 1, turbidityStr); // Formato: 4 dígitos, 1 decimal
    
//     // Mostrar en pantalla OLED
//     u8g2.clearBuffer();
//     u8g2.setFont(u8g2_font_ncenB08_tr);
//     u8g2.drawStr(0, 10, "Turbidez:");
//     u8g2.drawStr(0, 20, turbidityStr);
//     u8g2.drawStr(50, 20, " NTU");
//     u8g2.drawStr(0, 30, "Estado: ");
//     u8g2.drawStr(50, 30, digitalStatus.c_str());
//     u8g2.sendBuffer();
    
//     // Depuración en monitor serial
//     Serial.print(F("Analogico: "));
//     Serial.print(sensorValue);
//     Serial.print(F(" | Voltaje: "));
//     Serial.print(voltage);
//     Serial.print(F("V | Turbidez: "));
//     Serial.print(turbidity);
//     Serial.print(F(" NTU | Digital: "));
//     Serial.println(digitalStatus);
    
//     delay(1000); // Actualizar cada segundo
//   }
// }
/*#include <U8g2lib.h>
#include <Wire.h>

// Configurar pantalla OLED SSD1306, 128x32, I2C
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ //U8X8_PIN_NONE);
/*
void setup() {
  // Inicializar comunicación serial para depuración
  Serial.begin(9600);
  
  // Inicializar pantalla OLED
  u8g2.begin();
  
  // Mostrar mensaje inicial
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Fuente pequeña para 128x32
  u8g2.drawStr(0, 10, "Pantalla OLED OK!");
  u8g2.sendBuffer();
  delay(2000); // Mostrar mensaje inicial por 2 segundos
}

void loop() {
  static int counter = 0; // Contador para probar actualización
  
  // Mostrar contador
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Pantalla OLED OK!");
  u8g2.drawStr(0, 20, "Contador:");
  char counterStr[10];
  sprintf(counterStr, "%d", counter);
  u8g2.drawStr(60, 20, counterStr);
  u8g2.sendBuffer();
  
  // Depuración en monitor serial
  Serial.print("Contador: ");
  Serial.println(counter);
  
  counter++; // Incrementar contador
  delay(1000); // Actualizar cada segundo
}*/
#include <U8g2lib.h>
#include <Wire.h>

#define SENSOR_ANALOG_PIN A0
#define SENSOR_DIGITAL_PIN 3
#define RESET_BUTTON 2

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(SENSOR_DIGITAL_PIN, INPUT);
  Serial.begin(9600);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Turbidimetro listo");
  u8g2.sendBuffer();
  delay(2000);
}

void loop() {
  if (digitalRead(RESET_BUTTON) == LOW) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Reseteando...");
    u8g2.sendBuffer();
    delay(1000);
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Turbidimetro listo");
    u8g2.sendBuffer();
    delay(2000);
  } else {
    int sensorValue = analogRead(SENSOR_ANALOG_PIN);
    float voltage = sensorValue * (5.0 / 1023.0);
    float turbidity = 100 - (voltage * 20);
    if (turbidity < 0) turbidity = 0;
    int digitalValue = digitalRead(SENSOR_DIGITAL_PIN);
    String digitalStatus = digitalValue == HIGH ? "Baja" : "Alta";
    char turbidityStr[10];
    dtostrf(turbidity, 4, 1, turbidityStr);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Turbidez:");
    u8g2.drawStr(0, 20, turbidityStr);
    u8g2.drawStr(50, 20, " NTU");
    u8g2.drawStr(0, 30, "Estado: ");
    u8g2.drawStr(50, 30, digitalStatus.c_str());
    u8g2.sendBuffer();
    Serial.print(F("Analogico: "));
    Serial.print(sensorValue);
    Serial.print(F(" | Voltaje: "));
    Serial.print(voltage);
    Serial.print(F("V | Turbidez: "));
    Serial.print(turbidity);
    Serial.print(F(" NTU | Digital: "));
    Serial.println(digitalStatus);
    delay(1000);
  }
}
