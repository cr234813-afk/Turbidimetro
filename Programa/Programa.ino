#include <U8g2lib.h> //Libreria para la pantalla oled
#include <avr/wdt.h> //Libreria para reinicio del arduino

// Definir pines
#define SENSOR_ANALOG_PIN A1    // Pin analógico del sensor de turbidez
#define ENCODER_A_PIN 2         // Pin A del encoder rotativo
#define ENCODER_B_PIN 3         // Pin B del encoder rotativo
#define ENCODER_SW_PIN 4        // Pin del switch del encoder

// Inicializar la pantalla OLED (128x32)
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE); // Giro 180°

// Variable para la opción del menú (0: Medir NTU, 1: NTU Real Time, 2: Medir Directo, 3: Reiniciar)
int menuOption = 0;

// Variables para EMA
float smoothedValue = 0.0;
const float ALPHA = 0.1; // Factor de suavizado (0.05-0.2: más bajo = más suave)

// Para filtro mediano (array temporal)
const int MEDIAN_SAMPLES = 5; // Número impar para mediano
int readings[MEDIAN_SAMPLES];

// Para estabilización en Medir NTU
const int STAB_SAMPLES = 5; // Número de muestras para chequear estabilidad
const int STAB_THRESHOLD = 5; //10 // Máxima diferencia permitida para considerar estable
int stabBuffer[STAB_SAMPLES]; // Buffer para últimas lecturas
const unsigned long MIN_STAB_TIME = 30000;  // 30 segundos mínimo
const unsigned long MAX_STAB_TIME = 45000;  // 45 segundos máximo
const int READ_INTERVAL = 500;              // Intervalo entre lecturas (ms)

// Función para leer el sensor con filtro mediano + EMA
int readSmoothedSensor() {
  // Paso 1: Tomar múltiples lecturas rápidas
  for (int i = 0; i < MEDIAN_SAMPLES; i++) {
    readings[i] = analogRead(SENSOR_ANALOG_PIN);
    delayMicroseconds(100);
  }

  // Paso 2: Filtro mediano para eliminar outliers
  // Ordenar el array (burbuja simple, ya que es pequeño)
  for (int i = 0; i < MEDIAN_SAMPLES - 1; i++) {
    for (int j = 0; j < MEDIAN_SAMPLES - i - 1; j++) {
      if (readings[j] > readings[j + 1]) {
        int temp = readings[j];
        readings[j] = readings[j + 1];
        readings[j + 1] = temp;
      }
    }
  }
  int median = readings[MEDIAN_SAMPLES / 2];
  
  // Paso 3: Aplicar EMA sobre el mediano
  smoothedValue = (ALPHA * median) + ((1 - ALPHA) * smoothedValue);
  return (int)smoothedValue;
}

// Nueva función: selecciona la ecuación correcta según el valor raw
float calculateNTU(int sensorRaw) {
  float voltage = (sensorRaw / 1023.0) * 5.0;
  float NTU;

  if (sensorRaw > 930) {
    // Ecuación para valores altos (sensor en zona de baja transmisión)
    NTU = -2985.0 * pow(voltage, 2) + 26928.0 * voltage - 60699.0;
  } else {
    // Ecuación para rango normal
    NTU = 912.5 * pow(voltage, 3) - 11157.0 * pow(voltage, 2) + 44246.0 * voltage - 56263.0; //Sensor DF Robot
    //NTU = -21.10 * pow(voltage, 3) + 517.1 * pow(voltage, 2) - 3479 * voltage + 7142; //Sensor MJKDZ JD23-2
  }

  return (NTU < 0) ? 0.0 : NTU;
}

// Función para mostrar "cargando..." en pantalla
void showLoadingScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 20, "Cargando...");
  u8g2.sendBuffer();
}

// Función para esperar estabilización
int waitForStableReading() {
  showLoadingScreen();
  unsigned long startTime = millis();
  int index = 0;

  // Inicializar buffer
  for (int i = 0; i < STAB_SAMPLES; i++) {
    stabBuffer[i] = 0;
  }

  while (millis() - startTime < MAX_STAB_TIME) {
    int value = readSmoothedSensor();
    // Agregar al buffer (shift)
    for (int i = 0; i < STAB_SAMPLES - 1; i++) {
      stabBuffer[i] = stabBuffer[i + 1];
    }
    stabBuffer[STAB_SAMPLES - 1] = value;

    delay(READ_INTERVAL);

    // Solo chequear después de MIN_STAB_TIME y buffer lleno
    if (millis() - startTime >= MIN_STAB_TIME && index >= STAB_SAMPLES) {
      // Calcular max - min
      int minVal = stabBuffer[0];
      int maxVal = stabBuffer[0];
      for (int i = 1; i < STAB_SAMPLES; i++) {
        if (stabBuffer[i] < minVal) minVal = stabBuffer[i];
        if (stabBuffer[i] > maxVal) maxVal = stabBuffer[i];
      }
      if (maxVal - minVal <= STAB_THRESHOLD) {
        // Estable: retornar promedio
        long sum = 0;
        for (int i = 0; i < STAB_SAMPLES; i++) sum += stabBuffer[i];
        return sum / STAB_SAMPLES;
      }
    }
    index++;
  }
  // Si no estabiliza en 20s, retornar última lectura
  return stabBuffer[STAB_SAMPLES - 1];
}

// Función para mostrar la pantalla de bienvenida
void showWelcomeScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr); // Fuente grande
  u8g2.drawStr(0, 15, "TURBIDIMETRO");
  u8g2.drawStr(10, 32, "BIENVENIDO");
  u8g2.sendBuffer();
  delay(3000); // Mostrar durante 3 segundos
}

// Función para dibujar el menú con páginas
void drawMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);// Fuente pequeña para mostrar más opciones
  if (menuOption < 3) {
    // Primera pantalla: Medir NTU, NTU Real Time, Medir Directo
    u8g2.drawStr(10, 10, menuOption == 0 ? "> Medir NTU" : " Medir NTU");
    u8g2.drawStr(10, 20, menuOption == 1 ? "> NTU Real Time" : " NTU Real Time");
    u8g2.drawStr(10, 30, menuOption == 2 ? "> Medir Directo" : " Medir Directo");
  } else {
    // Segunda pantalla: Reiniciar
    u8g2.drawStr(10, 10, menuOption == 3 ? "> Reiniciar" : " Reiniciar");
  }
  u8g2.sendBuffer();
}

// Función para leer el encoder rotativo
int readEncoder() {
  static int lastA = digitalRead(ENCODER_A_PIN);
  int currentA = digitalRead(ENCODER_A_PIN);
  if (currentA != lastA) {
    if (digitalRead(ENCODER_B_PIN) != currentA) {
      lastA = currentA;
      return 1; // Giro en sentido horario
    } else {
      lastA = currentA;
      return -1; // Giro en sentido antihorario
    }
  }
  return 0;
}

// Función para medir en tiempo real (valor raw)
void realTimeMeasurement() {
  while (digitalRead(ENCODER_SW_PIN) == HIGH) { // Salir con presión del switch
    int sensorValue = readSmoothedSensor();

    // Mostrar valor en el Serial
    Serial.print("Sensor Raw Value: ");
    Serial.println(sensorValue);

    // Mostrar valor en la pantalla OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr); // Fuente pequeña
    u8g2.drawStr(0, 10, "Valor Sensor:");
    u8g2.setFont(u8g2_font_ncenB14_tr); // Fuente grande para valor
    u8g2.setCursor(0, 25);
    u8g2.print(sensorValue);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(60, 25, "(0-1023)");
    u8g2.sendBuffer();

    delay(500); // Actualizar cada 500ms para suavidad
  }
  delay(50);
  while (digitalRead(ENCODER_SW_PIN) == LOW); // Esperar liberación
}

// Medir NTU en tiempo real (con las dos ecuaciones)
void realTimeNTUMeasurement() {
  while (digitalRead(ENCODER_SW_PIN) == HIGH) { // Salir con presión del switch
    int sensorValue = readSmoothedSensor();
    float NTU = calculateNTU(sensorValue);

    Serial.print("NTU Real Time: ");
    Serial.println(NTU, 2);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "NTU Real Time:");
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(0, 25);
    u8g2.print(NTU, 1);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(60, 25, "NTU");
    u8g2.sendBuffer();

    delay(500); // Actualizar cada 500ms para suavidad
  }
  delay(50);
  while (digitalRead(ENCODER_SW_PIN) == LOW); // Esperar liberación
}

// Función para confirmar reinicio
bool confirmReset() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Confirmar Reinicio?");
  u8g2.drawStr(0, 25, "Presione para OK");
  u8g2.sendBuffer();

  unsigned long startTime = millis();
  while (millis() - startTime < 5000) { // 5 segundos para confirmar
    if (digitalRead(ENCODER_SW_PIN) == LOW) {
      delay(50);
      while (digitalRead(ENCODER_SW_PIN) == LOW); // Esperar liberación
      return true; // Confirmado
    }
  }
  return false; // No confirmado
}

void setup() {
  Serial.begin(9600);
  // Inicializar la pantalla OLED
  u8g2.begin();

  // Configurar pines del encoder como entradas
  pinMode(ENCODER_A_PIN, INPUT);
  pinMode(ENCODER_B_PIN, INPUT);
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP); // Switch con pull-up interno

  // Deshabilitar el watchdog al inicio
  wdt_disable();

  // Inicializar smoothedValue con una lectura inicial
  smoothedValue = analogRead(SENSOR_ANALOG_PIN);
  
  // Mostrar la pantalla de bienvenida
  showWelcomeScreen();
  
  // Mostrar el menú inicial
  drawMenu();
}

void loop() {
  // Leer el encoder para navegar el menú
  int enc = readEncoder();
  if (enc != 0) {
    menuOption += enc;
    if (menuOption < 0) menuOption = 3; // Limitar a 0-3 y ciclar
    if (menuOption > 3) menuOption = 0;
    drawMenu();
  }

  // Leer el switch del encoder para seleccionar la opción
  if (digitalRead(ENCODER_SW_PIN) == LOW) { // Switch activo en bajo
    delay(50);
    while (digitalRead(ENCODER_SW_PIN) == LOW); // Esperar a que se suelte

    if (menuOption == 0) {
      // Opción "Medir NTU": Esperar estabilización y mostrar NTU
      int sensorValue = waitForStableReading();
      float NTU = calculateNTU(sensorValue);

      // Valores en el serial
      Serial.print("Sensor Raw Value: "); Serial.println(sensorValue);
      Serial.print("Voltage: "); Serial.print((sensorValue / 1023.0) * 5.0, 3); Serial.println(" V"); // Muestra 3 decimales
      Serial.print("NTU: "); Serial.println(NTU, 2);// Muestra 2 decimales

      // Determinar el mensaje según el valor de NTU
      String message;
      if (NTU < 5)        message = "Excelente";
      else if (NTU < 15)  message = "Aceptable";
      else if (NTU < 100) message = "Precaucion";
      else                message = "No aceptable";

      // Pantalla 1: Mostrar valor de turbidez
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);// Fuente pequeña
      u8g2.drawStr(0, 10, "Turbidez:");
      u8g2.setFont(u8g2_font_ncenB14_tr);// Fuente grande para NTU
      u8g2.setCursor(0, 25);
      u8g2.print(NTU, 1);// Mostrar NTU con 1 decimal
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(60, 25, "NTU");
      u8g2.sendBuffer();

      // Esperar presión del switch para mostrar categoría
      while (digitalRead(ENCODER_SW_PIN) == HIGH); // Esperar presión
      delay(50);
      while (digitalRead(ENCODER_SW_PIN) == LOW); // Esperar liberación

      // Pantalla 2: Mostrar categoría
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(0, 16, "Categoria:");
      u8g2.drawStr(0, 32, message.c_str());
      u8g2.sendBuffer();

      // Esperar presión del switch para volver al menú
      while (digitalRead(ENCODER_SW_PIN) == HIGH);  // Esperar presión
      delay(50);
      while (digitalRead(ENCODER_SW_PIN) == LOW); // Esperar liberación

      drawMenu(); // Volver al menú
    }
    else if (menuOption == 1) {
      // Opción "NTU Real Time": Medir NTU en tiempo real
      realTimeNTUMeasurement();
      drawMenu(); // Volver al menú
    }
    else if (menuOption == 2) {
      // Opción "NTU Real Time": Medir en tiempo real (raw)
      realTimeMeasurement();
      drawMenu(); // Volver al menú
    }
    else if (menuOption == 3) {
      // Opción "Reiniciar": Confirmar y reiniciar el Arduino
      if (confirmReset()) {
        wdt_enable(WDTO_15MS); // Habilitar watchdog con timeout de 15ms
        while (1); // Esperar el reinicio
      }
      drawMenu(); // Volver al menú si no se confirma
    }
  }
}

