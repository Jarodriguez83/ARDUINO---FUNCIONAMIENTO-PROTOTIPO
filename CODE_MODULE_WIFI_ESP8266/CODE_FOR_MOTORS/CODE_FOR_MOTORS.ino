#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <SoftwareSerial.h>

// --- Comunicación con el ATmega328P ---
// RX del NodeMCU (D7 = GPIO13) ← TX del ATmega
// TX del NodeMCU (D8 = GPIO15) → RX del ATmega (no lo usamos)
SoftwareSerial megaSerial(13, 15); // RX = GPIO13 (D7), TX = GPIO15 (D8)


// --- Configuración WiFi ---
const char* ssid = "edge";          // 🔹 Cambia por tu red WiFi
const char* password = "12345678";  // 🔹 Contraseña WiFi

// --- Configuración ThingSpeak ---
unsigned long myChannelNumber = 3151266;       // ✅ Tu Channel ID
const char* myWriteAPIKey = "FGAS03T0WCHSTST7"; // ✅ Tu Write API Key

WiFiClient client;
String inputString = "";

void setup() {
  Serial.begin(115200);   // Para ver mensajes por USB
  megaSerial.begin(9600); // Comunicación con el ATmega
  WiFi.begin(ssid, password);
  ThingSpeak.begin(client);

  Serial.println("Conectando al WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  while (megaSerial.available()) {
    char c = megaSerial.read();
    if (c == '\n' || c == '\r') {
      processData(inputString);
      inputString = "";
    } else {
      inputString += c;
    }
  }
}

// --- Procesar los datos recibidos del ATmega ---
void processData(String data) {
  data.trim();

  // Si los datos vienen como "pH:7.12"
  int sepIndex = data.indexOf(':');
  if (sepIndex != -1) {
    data = data.substring(sepIndex + 1);
  }

  float phValue = data.toFloat();

  if (phValue > 0 && phValue <= 14) {
    Serial.print("📡 pH recibido del ATmega: ");
    Serial.println(phValue, 2);

    int response = ThingSpeak.writeField(myChannelNumber, 1, phValue, myWriteAPIKey);

    if (response == 200) {
      Serial.println("✅ Dato enviado correctamente a ThingSpeak!");
    } else {
      Serial.print("❌ Error al enviar (");
      Serial.print(response);
      Serial.println(")");
    }

    delay(20000); // ThingSpeak acepta un envío cada 15–20 s
  } else {
    Serial.print("⚠️ Dato inválido recibido: ");
    Serial.println(data);
  }
}