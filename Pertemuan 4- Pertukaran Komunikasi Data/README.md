# Modul 4: Komunikasi dan Pertukaran Data pada IoT
## Pustaka dan Ketergantungan
Untuk menjalankan program ini, diperlukan beberapa perpustakaan berikut yang harus diinstal pada Arduino IDE:
   1. ESP8266WiFi.h
   2. PubSubClient.h
   3. ArduinoJson.h
   4. DHT.h(Untuk tinjauan 4B)

## Percobaan 4A: Berlangganan dan Deserialisasi Data JSON
### Detailnya
Pada percobaan ini, ESP8266 diatur untuk menerima (berlangganan) perintah dari broker MQTT ( broker.hivemq.com). ESP8266 akan menunggu pesan masuk 
berformat JSON pada topik tertentu, memecah data tersebut (deserialisasi), lalu mengontrol aktuator (LED) sesuai isi dari pesan JSON tersebut.
## Penjelasan Fungsi Utama
- `hubungkanWiFi()`: menghubungkan ESP8266 ke router WiFi menggunakan SSID dan Password yang telah ditentukan. Fungsinya akan tertahan dalam loop
  `while` hingga koneksi berhasil.
- `hubungkanMQTT()`: menerjemahkan perangkat ke MQTT Broker. Fungsi ini juga menghasilkan clientIdsecara acak. Setelah koneksi berhasil, fungsi
  `client.subscribe(topicPerintah)` dipanggil di dalam sini agar alat langsung mendengarkan topik perintah.
- `callback(char* topic, byte* payload, unsigned int length)`: Fungsi yang akan dipanggil secara otomatis setiap kali ada pesan baru yang masuk
  ke topik yang di-subscribe. Fungsi ini membaca payload (pesan), lalu menggunakan `deserializeJson()` untuk mengekstrak datanya.
- `setup()`: Menginisialisasi Serial Monitor, pin LED sebagai output, menghubungkan ke WiFi, mengatur server MQTT, dan memutar fungsi `callback`.
- `loop()`: Menjaga koneksi MQTT tetap hidup (dengan `client.connected()` dan memanggil ulang `hubungkanMQTT()`). Fungsi `client.loop()` wajib dipanggil
  di sini untuk memproses paket masuk.

## Penjelasan Percabangan (Kondisi)
- `if (error)`: Jika fungsi deserialisasi JSON gagal (misal format salah), program akan masuk blok ini untuk mencetak kesalahan dan menghentikan sisa
  eksekusi fungsi dengan `return`.
- `if (String(perintah) == "ON")/ else if`: Memeriksa nilai dari JSON "perintah". Jika nilainya "ON", akan memanggil `digitalWrite(ledPin, HIGH)`.
  Jika "OFF", panggil `LOW`.

## Jawaban Pertanyaan Praktikum 4A (Modifikasi PWM)
Untuk menambahkan fitur mengatur intensitas kecerahan LED melalui PWM berdasarkan pesan JSON seperti `{"perintah": "ON", "intensitas": 200}`, berikut 
adalah modifikasi kode pada fungsi `callback`:
```cpp
// ... kode bagian atas fungsi callback tetap sama ...

// Deserialisasi data JSON yang diterima
JsonDocument doc;
DeserializationError error = deserializeJson(doc, pesan);
if (error) {
  Serial.print("Gagal parsing JSON: ");
  Serial.println(error.c_str());
  return;
}

const char* perintah = doc["perintah"];
int intensitas = doc["intensitas"]; // TAMBAHAN: Mengambil nilai "intensitas" berformat integer dari objek JSON

if (String(perintah) == "ON") {
  analogWrite(ledPin, intensitas); // TAMBAHAN: Menggunakan analogWrite (PWM) untuk mengatur kecerahan LED (0-1023 untuk ESP8266)
  Serial.print("Aktuator: ON, Intensitas: ");
  Serial.println(intensitas); // TAMBAHAN: Mencetak nilai intensitas ke Serial Monitor
} else if (String(perintah) == "OFF") {
  analogWrite(ledPin, 0); // TAMBAHAN: Mematikan LED dengan mengatur duty cycle PWM ke 0
  Serial.println("Aktuator: OFF");
}
```

## Percobaan 4B: Pertukaran Data Dua Arah (Full Duplex)
### Detailnya
Percobaan ini menggabungkan fitur terbitkan dan berlangganan agar berjalan beriringan. ESP8266 mengirimkan (publish) data pembacaan sensor suhu DHT11 
setiap 5 detik ke satu topik, sementara di saat yang sama ia terus mendengarkan (berlangganan) perintah kendali LED dari topik yang berbeda. Penggunaan 
`delay()` dihindari dan diganti dengan `millis()` agar proses mendengarkan tidak terhambat.

### Penjelasan Percabangan Khusus (Non-Blocking & Ternary)
- `if (millis() - waktuTerakhirPublish > intervalPublish)`: Merupakan logika penundaan non-blocking . Program menghitung selisih waktu saat ini dengan waktu
  terakhir data dikirim. Jika selisihnya lebih dari 5000 ms, maka sensor dibaca dan data di-publish. Ini memastikan `client.loop()` tidak pernah berhenti.
- `if (!isnan(suhu))`: Percabangan untuk validasi. membaca angka suhu yang dibaca bukanlah Not a Number (NaN) sebelum dikirimkan.
- `String(perintah) == "ON" ? HIGH : LOW`: Ini adalah Operator Ternary (versi singkat dari IF-ELSE). Artinya: "Jika perintahnya ON, kembalikan
  nilai HIGH, selain itu kembalikan LOW". Hasilnya langsung dimasukkan ke parameter `digitalWrite()`.

### Jawaban Pertanyaan Praktikum 4B (Modifikasi Tambah Topik Aktuator Kedua)
Untuk menambahkan kontrol aktuator kedua (misal: Buzzer) dengan topik MQTT terpisah, kita harus memodifikasi variabel deklarasi, pendaftaran 
berlangganan, dan memeriksa variabel `topic` pada fungsi `callback`.
Modifikasi Kode:
```cpp
// 1. TAMBAHAN DEKLARASI GLOBAL (di bagian atas program)
const char* topicBuzzer = "unsoed/tk245004/kelompok5/buzzer"; // Definisi topik khusus untuk buzzer
const int buzzerPin = 12; // (misal GPIO12/D6) Deklarasi pin untuk aktuator kedua

// 2. TAMBAHAN DI DALAM FUNGSI setup()
pinMode(buzzerPin, OUTPUT); // Mengatur pin buzzer sebagai output

// 3. TAMBAHAN DI DALAM FUNGSI hubungkanMQTT()
if (client.connect(clientId.c_str())) {
  client.subscribe(topicPerintah); // (Sudah ada) Subscribe LED
  client.subscribe(topicBuzzer);   // TAMBAHAN: Subscribe ke topik Buzzer setelah koneksi MQTT terjalin
  Serial.println("Terhubung dan subscribe topik LED dan Buzzer");
}

// 4. MODIFIKASI FUNGSI callback()
void callback(char* topic, byte* payload, unsigned int length) {
  String pesan;
  for (unsigned int i = 0; i < length; i++) pesan += (char) payload[i];
  
  JsonDocument doc;
  if (deserializeJson(doc, pesan)) return;

  const char* perintah = doc["perintah"];

  // TAMBAHAN: Membedakan aksi berdasarkan 'topic' dari pesan yang masuk
  if (String(topic) == String(topicPerintah)) {
    // Blok dieksekusi HANYA jika pesan berasal dari topik LED
    digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
    Serial.print("Perintah LED diterima: ");
    Serial.println(perintah);
  } 
  else if (String(topic) == String(topicBuzzer)) {
    // TAMBAHAN: Blok dieksekusi HANYA jika pesan berasal dari topik Buzzer
    digitalWrite(buzzerPin, String(perintah) == "ON" ? HIGH : LOW);
    Serial.print("Perintah Buzzer diterima: ");
    Serial.println(perintah);
  }
}
```
Penjelasan Modifikasi Kode: Fungsi `callback` di atas membandingkan argumen topicyang diterima MQTT dengan konstanta topik ( `topicPerintah` dan `topicBuzzer`) yang 
sudah dideklarasikan. Dengan percabangan `if-else if` tersebut, mikrokontroler tidak akan salah menyalakan perangkat (misal: pesan masuk ke topik buzzer 
tidak akan mengubah status LED).

Rangkaian Percobaan 4A:
<img width="960" height="1280" alt="WhatsApp Image 2026-10-08 at 16 14 22" src="https://github.com/user-attachments/assets/9813703b-d030-4417-8de3-685c7d611f6d" />

Rangkaian Percobaan 4B:
<img width="960" height="1280" alt="WhatsApp Image 2026-10-08 at 16 14 22 (1)" src="https://github.com/user-attachments/assets/289030e4-804a-4c75-a4b9-83647aced3e0" />
