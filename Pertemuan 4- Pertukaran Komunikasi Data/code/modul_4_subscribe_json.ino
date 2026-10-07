#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ==========================
// KONFIGURASI WIFI
// ==========================
const char* ssid = "Nxxy";
const char* password = "Nuxxccyy";

// ==========================
// KONFIGURASI MQTT
// ==========================
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

const char* topicPerintah =
  "unsoed/tk245004/kelompok5/perintah";

// ==========================
// PIN LED
// ==========================
const int ledPin = 5;

// ==========================
// OBJECT WIFI & MQTT
// ==========================
WiFiClient espClient;
PubSubClient client(espClient);


// ==========================
// CALLBACK MQTT
// ==========================
// Fungsi ini dipanggil otomatis
// ketika ada pesan masuk
void callback(char* topic, byte* payload, unsigned int length) {

  String pesan;

  // Mengubah payload menjadi String
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  // Menampilkan pesan yang diterima
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(pesan);

  // ==========================
  // DESERIALISASI JSON
  // ==========================
  JsonDocument doc;

  DeserializationError error =
    deserializeJson(doc, pesan);

  // Jika JSON tidak valid
  if (error) {
    Serial.print("Gagal parsing JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Mengambil nilai "perintah"
  const char* perintah = doc["perintah"];

  // ==========================
  // KENDALI LED
  // ==========================
  if (String(perintah) == "ON") {

    digitalWrite(ledPin, HIGH);
    Serial.println("Aktuator: ON");

  } 
  else if (String(perintah) == "OFF") {

    digitalWrite(ledPin, LOW);
    Serial.println("Aktuator: OFF");
  }
}


// ==========================
// HUBUNGKAN WIFI
// ==========================
void hubungkanWiFi() {

  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi berhasil terhubung!");
}


// ==========================
// HUBUNGKAN MQTT
// ==========================
void hubungkanMQTT() {

  while (!client.connected()) {

    Serial.print("Menghubungkan ke broker MQTT...");

    String clientId =
      "ESP32Client-" +
      String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {

      Serial.println("berhasil terhubung!");

      // Subscribe ke topic perintah
      client.subscribe(topicPerintah);

      Serial.print("Subscribe ke topic: ");
      Serial.println(topicPerintah);

    } 
    else {

      Serial.print("gagal, rc=");
      Serial.print(client.state());

      Serial.println(
        " coba lagi dalam 2 detik"
      );

      delay(2000);
    }
  }
}


// ==========================
// SETUP
// ==========================
void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);

  // Hubungkan WiFi
  hubungkanWiFi();

  // Tentukan server MQTT
  client.setServer(
    mqttServer,
    mqttPort
  );

  // Daftarkan callback
  client.setCallback(callback);
}


// ==========================
// LOOP
// ==========================
void loop() {

  // Jika MQTT terputus
  if (!client.connected()) {
    hubungkanMQTT();
  }

  // Memproses pesan MQTT
  client.loop();
}
