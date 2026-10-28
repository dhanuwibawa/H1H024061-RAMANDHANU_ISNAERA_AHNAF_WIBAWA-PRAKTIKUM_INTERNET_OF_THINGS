#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <math.h>

const char* ssid = "Nxxy";
const char* password = "Nuxxccyy";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

const char* topic = "unsoed/tk245004/kelompok5/data";

WiFiClient espClient;
PubSubClient client(espClient);

float bacaSuhuMock() {
    static unsigned long jumlahBaca = 0;
    jumlahBaca++;

    float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);
    float noise = random(-5, 6) * 0.1;

    float suhu = suhuDasar + noise;

    if (jumlahBaca % 25 == 0) {
        suhu += 8.0;
    }

    return suhu;
}

void setupWiFi() {
    delay(10);
    Serial.println();
    Serial.print("Menghubungkan ke Wi-Fi: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi terhubung!");
    Serial.print("IP ESP8266: ");
    Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("Menghubungkan ke MQTT...");

        String clientId = "ESP8266-Kelompok2-";
        clientId += String(ESP.getChipId(), HEX);

        if (client.connect(clientId.c_str())) {
            Serial.println("berhasil!");
        } else {
            Serial.print("gagal, kode: ");
            Serial.println(client.state());
            delay(3000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(A0));

    setupWiFi();

    client.setServer(mqtt_server, mqtt_port);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        setupWiFi();
    }

    if (!client.connected()) {
        reconnectMQTT();
    }

    client.loop();

    float suhu = bacaSuhuMock();

    char payload[64];
    snprintf(payload, sizeof(payload), "{\"suhu\":%.2f}", suhu);

    bool berhasil = client.publish(topic, payload);

    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" C | MQTT: ");
    Serial.println(berhasil ? "Terkirim" : "Gagal");

    delay(2000);
}
