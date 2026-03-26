#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ===== WIFI =====
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";

// ===== API =====
// Troque pelo IP do computador onde o Flask está rodando
const char* SERVER_URL = "http://192.168.0.10:5000/receber-dados";

// ===== SENSOR =====
#define DHTPIN 4          // Pino de dados do DHT no ESP32
#define DHTTYPE DHT22     // Use DHT11 se for esse o sensor

DHT dht(DHTPIN, DHTTYPE);

// ===== IDENTIFICAÇÃO DO SENSOR =====
String sensorId = "esp32-sensor-01";

// ===== CONTROLE DE TEMPO =====
unsigned long ultimoEnvio = 0;
const unsigned long intervalo = 10000; // 10 segundos

void conectarWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  conectarWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi caiu. Reconectando...");
    conectarWiFi();
  }

  if (millis() - ultimoEnvio >= intervalo) {
    ultimoEnvio = millis();

    float umidade = dht.readHumidity();
    float temperatura = dht.readTemperature(); // Celsius

    if (isnan(umidade) || isnan(temperatura)) {
      Serial.println("Falha ao ler o sensor DHT.");
      return;
    }

    Serial.println("Leitura realizada:");
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(SERVER_URL);
      http.addHeader("Content-Type", "application/json");

      String json = "{";
      json += "\"sensorId\":\"" + sensorId + "\",";
      json += "\"temperatura\":" + String(temperatura, 2) + ",";
      json += "\"umidade\":" + String(umidade, 2);
      json += "}";

      Serial.println("Enviando JSON:");
      Serial.println(json);

      int httpResponseCode = http.POST(json);
      String response = http.getString();

      Serial.print("HTTP Code: ");
      Serial.println(httpResponseCode);

      Serial.print("Resposta servidor: ");
      Serial.println(response);

      http.end();
    } else {
      Serial.println("Sem conexão Wi-Fi no momento do envio.");
    }
  }
}