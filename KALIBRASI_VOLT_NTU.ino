#include <SoftwareSerial.h>
SoftwareSerial arduino(2, 3);  // RX, TX

// Array untuk menyimpan hasil pembacaan
unsigned int values[300];  // Array untuk menyimpan 300 nilai pembacaan
unsigned int i;

void setup() {
  Serial.begin(9600);
  arduino.begin(9600);  // Memulai komunikasi serial dengan baud rate 9600 ke esp32
}

void loop() {
  unsigned int z = 0;  // Reset nilai maksimum setiap kali loop dimulai

  // Menyimpan hasil pembacaan ke memori
  for (i = 0; i < 300; i++) {
    values[i] = analogRead(A0);  // Membaca nilai analog dari pin A0
    if (values[i] >= z) {
      z = values[i];  // Menyimpan nilai maksimum
    }
  }

  // Mengonversi nilai ADC menjadi tegangan
  float voltage = z * (5.0 / 1024.0);  // Mengonversi nilai ADC ke tegangan (5V untuk Arduino)

  // Menghitung NTU berdasarkan rentang tegangan
  float ntu;
  if (voltage >= 3.66) {
    // Menggunakan persamaan pertama (untuk rentang 0 NTU hingga 120 NTU)
    ntu = -1170.7 * voltage + 4407.3;
  } else {
    // Menggunakan persamaan kedua (untuk rentang 120 NTU hingga 400 NTU)
    ntu = -1061.8 * voltage + 4008.5;
  }

  // Menampilkan hasil pembacaan di Serial Monitor
  Serial.print("Tegangan = ");
  Serial.print(voltage, 4);
  Serial.print(" V | NTU = ");
  Serial.println(ntu);

  // Mengirim nilai NTU ke ESP32
  arduino.println(ntu);
  // arduino.println(voltage, 4);

  delay(60000);  // Menunggu selama 1 detik sebelum memulai pembacaan berikutnya
}
