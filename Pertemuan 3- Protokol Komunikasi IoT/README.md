# Modul 3: Protokol Komunikasi IoT
## 1. Penjelasan Singkat Percobaan
- Percobaan 3A (Komunikasi HTTP): Mengimplementasikan pengiriman data sensor (dummy data suhu dan kelembaban) dari
  mikrokontroler ESP8266 ke sebuah server endpoint ( httpbin.org/post) menggunakan protokol HTTP dengan metode POST. Data
  dikirimkan dalam format JSON.
- Percobaan 3B (Komunikasi MQTT): Mengimplementasikan pertukaran data menggunakan pola publish-subscribe denganprotokol
  MQTT. ESP8266 mempublikasikan data sensor dalam format JSON ke sebuah broker publik ( broker.hivemq.com) pada topik
  tertentu secara berkala.
  
## 2. Pustaka / Ketergantungan
Library yang dibutuhkan untuk menjalankan program ini antara lain:
- `ESP8266WiFi.h`
- `ESP8266HTTPClient.h`
- `WiFiClientSecure.h`
- `ArduinoJson.h`
- `PubSubClient.h`

## 3. Penjelasan Kode & Fungsi
### Pada Percobaan 3A (HTTP)
- `setup()`: Fungsi yang dijalankan sekali saat mikrokontroler menyala. Berfungsi untuk mengatur kecepatan komunikasi serial (115200) dan
  menginisialisasi proses koneksi ke jaringan WiFi.
- `loop()`: Fungsi utama yang berjalan terus-menerus. Di dalamnya, program menyiapkan koneksi HTTP/HTTPS, membentuk struktur JSON berisi
  suhu dan kelembaban, mengubahnya menjadi string ( serializeJson), dan mengirimkannya menggunakan http.POST().
- `client.setInsecure()`: Digunakan pada objek WiFiClientSecureuntuk melewati proses verifikasi sertifikat SSL saat mengakses endpoint HTTPS.
### Pada percobaan 3B (MQTT)
- `hubungkanWiFi()`: Fungsi kustom untuk menghubungkan ESP8266 ke jaringan WiFi yang telah ditentukan SSID dan password-nya.
- `hubungkanMQTT()`: Fungsi kustom yang menangani proses koneksi ke broker MQTT. Fungsi ini akan mencoba menyambung ulang terus-menerus
  dengan membuat Client ID acak ( random ) jika belum terhubung.
- `setup()`: Melakukan inisialisasi koneksi Serial, memanggil hubungkanWiFi(), dan mengkonfigurasi alamat serta port server broker
  MQTT ( client.setServer).
- `loop()`: mengedit perangkat selalu terhubung ke broker, memanggil client.loop()untuk menjaga koneksi tetap hidup ( keep-alive ), membuat
  objek JSON, dan mempublikasikannya menggunakan client.publish()setiap 5 detik.

## 4. Penjelasan Percabangan (Kondisi)
- `while (WiFi.status() != WL_CONNECTED)`: Loop yang akan terus menahan program (memberikan delay dan mencetak titik) SELAMA status WiFi belum terhubung.
- `if (WiFi.status() == WL_CONNECTED)`: (Pada 3A) menolak bahwa HTTP request hanya akan dibuat dan dikirim JIKA perangkat benar-benar sudah terkoneksi
  ke jaringan internet.
- `if (httpResponseCode > 0)`: (Pada 3A) Memeriksa apakah kode respon dari server bernilai lebih dari 0 (misalnya 200), yang menandakan permintaan
  berhasil terkirim dan diterima server, bukan error di sisi jaringan klien.
- `while (!client.connected())`: (Pada 3B) Loop yang akan terus dieksekusi SELAMA perangkat belum berhasil melakukan handshake dengan broker MQTT.
- `if (client.connect(clientId.c_str()))`: (Pada 3B) Percabangan yang mencoba menghubungkan ke broker MQTT. Jika koneksi berhasil (true), akan
  mencetak pesan sukses. Jika gagal, blok else akan dijalankan untuk mencetak kode error dan menunggu sebelum mencoba lagi.

## 5. Jawaban Pertanyaan Praktikum (Percobaan 3A)
Tugas: Modifikasi program agar dapat mengirimkan data tambahan berupa waktu (dalam milidetik sejak dinyalakan menggunakan `millis())` ke dalam 
JSON yang dikirim, dan memberikan penjelasan di setiap baris kode yang ditambahkan.

Modifikasi Kode: Untuk menambahkan data `millis()`, modifikasi dilakukan pada blok pembuatan objek JSON dalam fungsi `loop()`:
```
// Membuat objek data sensor dalam format JSON
JsonDocument doc;
doc["suhu"] = 28.5;
doc["kelembaban"] = 65.0;

// -- BARIS YANG DITAMBAHKAN --
doc["waktu"] = millis(); 
// ---------------------------

String requestBody;
serializeJson(doc, requestBody);
```
Penjelasan Baris Kode yang Ditambahkan:
- `doc["waktu"] = millis();`
  - `doc["waktu"]`: Perintah ini ramuan JsonDocumentuntuk membuat pasangan key baru dengan nama "waktu".
  - `=`: Penugasan operator untuk mengisi nilai ke dalam kunci tersebut.
  - `millis()`: Memanggil fungsi internal Arduino yang berfungsi menghitung dan mengembalikan jumlah waktu dalam satuan milidetik
    (ms) semenjak board mikrokontroler pertama kali dialiri listrik dan mulai menjalankan program.
  - Kesimpulan: Secara keseluruhan, baris ini menambahkan timestamp relatif ke dalam paket data JSON, sehingga saat diterima oleh
    server, formatnya menjadi `{"suhu":28.5, "kelembaban":65.0, "waktu":15000}` (contoh jika data dikirim pada detik ke-15).

Percobaan 3A:
<img width="1600" height="900" alt="WhatsApp Image 2026-09-23 at 17 06 38" src="https://github.com/user-attachments/assets/ef67aaf8-8db4-478b-b95c-da1f4de61032" />

Percobaan 3B:
<img width="1601" height="892" alt="Screenshot 2026-09-23 170851" src="https://github.com/user-attachments/assets/baaa59aa-3579-4395-bcd6-264102fcaf2b" />
