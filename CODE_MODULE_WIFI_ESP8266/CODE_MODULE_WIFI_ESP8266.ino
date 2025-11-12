#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

// --- Bluetooth (pines 6 y 7)
SoftwareSerial BT(6, 7); // RX, TX

// --- Pines de motores
const int ENA = 3;
const int IN1 = 4;
const int IN2 = 5;
const int ENB = 11;
const int IN3 = 12;
const int IN4 = 10;

// --- Sensor pH
const int PH_PIN = A0;

// --- Sensor DS18B20
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- Variables
int speedMotor = 255;
char cmd;
unsigned long lastReadTime = 0;
const unsigned long readInterval = 1000; // 1 segundo

void setup() {
  Serial.begin(9600);   // Serial → NodeMCU
  BT.begin(9600);       // Bluetooth

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(PH_PIN, INPUT);

  sensors.begin();      // Inicializa DS18B20

  Stop();
  Serial.println("Iniciando ATmega...");
}

void loop() {
  // --- Control por Bluetooth
  if (BT.available()) {
    cmd = BT.read();
    Stop();
    switch (cmd) {
      case 'U': case 'D': moveForward(); break;
      case 'L': case 'R': moveBackward(); break;
      default: Stop(); break;
    }
  }

  // --- Lectura no bloqueante del pH y temperatura
  unsigned long currentMillis = millis();
  if (currentMillis - lastReadTime >= readInterval) {
    lastReadTime = currentMillis;

    // Leer voltaje crudo del pH
    int sensorValue = analogRead(PH_PIN);
    float voltage = sensorValue * (5.0 / 1023.0); // Voltaje de 0 a 5V

    // Leer temperatura DS18B20
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    // Enviar datos al NodeMCU: voltaje del pH y temperatura
    Serial.print("PH_VOLT:");
    Serial.print(voltage, 3); // 3 decimales
    Serial.print(";TEMP:");
    Serial.println(tempC, 2);
  }
}

// --- Funciones de motor
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, speedMotor);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, speedMotor);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, speedMotor);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, speedMotor);
}

void Stop() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}