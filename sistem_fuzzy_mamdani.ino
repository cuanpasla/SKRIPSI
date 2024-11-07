#include "DFRobot_ESP_PH.h"
#include "EEPROM.h"
// #include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <AntaresESPHTTP.h>

#define ACCESSKEY "983825e56c487fe6:c4d80cf91d63ec37"  // Replace with your Antares account access key
#define WIFISSID "P SUNARDI SMARTFARM"                                // Replace with your Wi-Fi SSID
#define PASSWORD "12345678"                         // Replace with your Wi-Fi password

#define projectName "Monitoring_PHdanKekeruhan"  // Replace with the Antares application name that was created
#define deviceName "Monitor1"                    // Replace with the Antares device name that was created

AntaresESPHTTP antares(ACCESSKEY);

float minr[16];
float a1, a2;
float A1[16];
float A2[16];
float Rule[16];
const char *hasil;
float Z, pH, KEkeruhan;
String kualitasAir;

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

DFRobot_ESP_PH ph;
#define ESPADC 4096.0    //the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300  //the esp voltage supply value
#define PH_PIN 35        //the esp gpio data pin number
float kekeruhan, A, B, voltage, phValue, temperature = 25;

const int tb_pin = A0;

unsigned long previousMillis = 0;  // Untuk menyimpan waktu terakhir LCD diperbarui
const long interval = 1000;        // Interval waktu dalam milidetik (1 detik)

// Lebar OLED display dalam satuan pixels
#define SCREEN_WIDTH 128
// tinggi OLED display dalam satuan pixels
#define SCREEN_HEIGHT 64

// Deklarasi untuk display SSD1306 yang terhubung menggunakan I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/// Fungsi keanggotaan untuk input pH air
float fuPSasam(float ph) {
  if (ph >= 5.5) {
    return 0;
  } else if (5 < ph && ph < 5.5) {
    return (5.5 - ph) / (5.5 - 5);
  } else if (ph <= 5) {
    return 1;
  }
  return 0;
}

float fuPasam(float ph) {
  if (ph <= 5 || ph >= 6.5) {
    return 0;
  } else if (5 < ph && ph < 5.5) {
    return (ph - 5) / (5.5 - 5);
  } else if (5.5 <= ph && ph <= 6) {
    return 1;
  } else if (6 < ph && ph < 6.5) {
    return (6.5 - ph) / (6.5 - 6);
  }
  return 0;
}

float fuPnormal(float ph) {
  if (ph <= 6 || ph >= 8) {
    return 0;
  } else if (6 < ph && ph < 7) {
    return (ph - 6) / (7 - 6);
  } else if (ph == 7) {
    return 1;
  } else if (7 < ph && ph < 8) {
    return (8 - ph) / (8 - 7);
  }
  return 0;
}

float fuPbasa(float ph) {
  if (ph <= 7.5 || ph >= 9) {
    return 0;
  } else if (7.5 < ph && ph < 8) {
    return (ph - 7.5) / (8 - 7.5);
  } else if (8 <= ph && ph <= 8.5) {
    return 1;
  } else if (8.5 < ph && ph < 9) {
    return (9 - ph) / (9 - 8.5);
  }
  return 0;
}

float fuPSbasa(float ph) {
  if (ph <= 8.5) {
    return 0;
  } else if (8.5 < ph && ph < 9) {
    return (ph - 8.5) / (9 - 8.5);
  } else if (ph >= 9) {
    return 1;
  }
  return 0;
}

//======== Fungsi keanggotaan kekeruhan =========================================

float fuKJernih(float kekeruhan) {

  if (kekeruhan >= 5) {
    return 0;
  }

  else if (4 < kekeruhan && kekeruhan < 5) {
    return (5 - kekeruhan) / (5 - 4);
  }

  else if (kekeruhan <= 4) {
    return 1;
  }
  return 0;
}

float fuKKeruh(float kekeruhan) {

  if (kekeruhan <= 4 || kekeruhan >= 25) {
    return 0;
  }

  else if (4 < kekeruhan && kekeruhan < 5) {
    return (kekeruhan - 4) / (5 - 4);
  }

  else if (5 <= kekeruhan && kekeruhan <= 24) {
    return 1;
  }

  else if (24 < kekeruhan && kekeruhan < 25) {
    return (25 - kekeruhan) / (25 - 24);
  }
  return 0;
}


float fuKSangatKeruh(float kekeruhan) {

  if (kekeruhan <= 24) {
    return 0;
  }

  else if (24 < kekeruhan && kekeruhan < 25) {
    return (kekeruhan - 24) / (25 - 24);
  }

  else if (kekeruhan >= 25) {
    return 1;
  }
  return 0;
}

//=========OUTPUT NILAI A1 dan A2==============


float RUMUSOutputSangatBaik(float output) {
  a1 = output * (6 - 5) + 5;
  a2 = 7;
  return 0;
}

float RUMUSOutputBaik(float output) {
  a1 = output * (4 - 3) + 3;
  a2 = (-1) * ((output * (6 - 5)) - 6);
  return 0;
}


float RUMUSOutputBuruk(float output) {
  a1 = output * (2 - 1) + 1;
  a2 = (-1) * ((output * (4 - 3)) - 4);
  return 0;
}

float RUMUSOutputSangatBuruk(float output) {
  a1 = 0;
  a2 = (-1) * ((output * (2 - 1)) - 2);
  return 0;
}


//=============================================
float Min(float a, float b) {
  return (a < b) ? a : b;
}

void rule(float pH, float KEkeruhan, float *Rule) {
  // R1 If pH Normal and kekeruhan Jernih then kualitas Sangat_Baik
  Rule[1] = Min(fuPnormal(pH), fuKJernih(KEkeruhan));
  minr[1] = RUMUSOutputSangatBaik(Rule[1]);
  A1[1] = a1;
  A2[1] = a2;

  // R2 If pH Normal and kekeruhan Keruh then kualitas Baik
  Rule[2] = Min(fuPnormal(pH), fuKKeruh(KEkeruhan));
  minr[2] = RUMUSOutputBaik(Rule[2]);
  A1[2] = a1;
  A2[2] = a2;


  // R3 If pH Normal and kekeruhan Sangat Keruh then kualitas Sangat Buruk
  Rule[3] = Min(fuPnormal(pH), fuKSangatKeruh(KEkeruhan));
  minr[3] = RUMUSOutputSangatBuruk(Rule[3]);
  A1[3] = a1;
  A2[3] = a2;

  // R4 If pH Asam and kekeruhan Jernih then kualitas Baik
  Rule[4] = Min(fuPasam(pH), fuKJernih(KEkeruhan));
  minr[4] = RUMUSOutputBaik(Rule[4]);
  A1[4] = a1;
  A2[4] = a2;

  // R5 If pH Asam and kekeruhan Keruh then kualitas Buruk
  Rule[5] = Min(fuPasam(pH), fuKKeruh(KEkeruhan));
  minr[5] = RUMUSOutputBuruk(Rule[5]);
  A1[5] = a1;
  A2[5] = a2;

  // R6 If pH Asam and kekeruhan Sangat Keruh then kualitas Sangat Buruk
  Rule[6] = Min(fuPasam(pH), fuKSangatKeruh(KEkeruhan));
  minr[6] = RUMUSOutputSangatBuruk(Rule[6]);
  A1[6] = a1;
  A2[6] = a2;

  // R7 If pH Basa and kekeruhan Jernih then kualitas Baik
  Rule[7] = Min(fuPbasa(pH), fuKJernih(KEkeruhan));
  minr[7] = RUMUSOutputBaik(Rule[7]);
  A1[7] = a1;
  A2[7] = a2;

  // R8 If pH Basa and kekeruhan Keruh then kualitas Buruk
  Rule[8] = Min(fuPbasa(pH), fuKKeruh(KEkeruhan));
  minr[8] = RUMUSOutputBuruk(Rule[8]);
  A1[8] = a1;
  A2[8] = a2;

  // R9 If pH Basa and kekeruhan Sangat Keruh then kualitas Sangat Buruk
  Rule[9] = Min(fuPbasa(pH), fuKSangatKeruh(KEkeruhan));
  minr[9] = RUMUSOutputSangatBuruk(Rule[9]);
  A1[9] = a1;
  A2[9] = a2;

  // R10 If pH Sangat Asam and kekeruhan Jernih then kualitas Sangat Buruk
  Rule[10] = Min(fuPSasam(pH), fuKJernih(KEkeruhan));
  minr[10] = RUMUSOutputSangatBuruk(Rule[10]);
  A1[10] = a1;
  A2[10] = a2;

  // R11 If pH Sangat Asam and kekeruhan Keruh then kualitas Sangat Buruk
  Rule[11] = Min(fuPSasam(pH), fuKKeruh(KEkeruhan));
  minr[11] = RUMUSOutputSangatBuruk(Rule[1]);
  A1[11] = a1;
  A2[11] = a2;

  // R12 If pH Sangat Asam and kekeruhan Sangat Keruh then kualitas Sangat Buruk
  Rule[12] = Min(fuPSasam(pH), fuKSangatKeruh(KEkeruhan));
  minr[12] = RUMUSOutputSangatBuruk(Rule[12]);
  A1[12] = a1;
  A2[12] = a2;

  // R13 If pH Sangat Basa and kekeruhan Jernih then kualitas Sangat Buruk
  Rule[13] = Min(fuPSbasa(pH), fuKJernih(KEkeruhan));
  minr[13] = RUMUSOutputSangatBuruk(Rule[13]);
  A1[13] = a1;
  A2[13] = a2;

  // R14 If pH Sangat Basa and kekeruhan Keruh then kualitas Sangat Buruk
  Rule[14] = Min(fuPSbasa(pH), fuKKeruh(KEkeruhan));
  minr[14] = RUMUSOutputSangatBuruk(Rule[14]);
  A1[14] = a1;
  A2[14] = a2;

  // R15 If pH Sangat Basa and kekeruhan Sangat Keruh then kualitas Sangat Buruk
  Rule[15] = Min(fuPSbasa(pH), fuKSangatKeruh(KEkeruhan));
  minr[15] = RUMUSOutputSangatBuruk(Rule[15]);
  A1[15] = a1;
  A2[15] = a2;
}

float defuzzyfikasi(float *Rule) {
  float min = 100;
  float max = 0;
  float max_membership = 0;
  float sum = 0;
  float count = 0;

  // Cari nilai maksimum keanggotaan dari semua aturan
  for (int i = 1; i <= 15; i++) {

    if (Rule[i] > max_membership) {
      max_membership = Rule[i];
    }
  }

  for (int i = 1; i <= 25; i++) {
    if (Rule[i] == max_membership) {
      // Cek nilai minimum dan maksimum
      if (A1[i] < min) {
        min = A1[i];
      }
      if (A2[i] > max) {
        max = A2[i];
      }
    }
  }
  // Hitung nilai Z
  Z = (((max - min + 1) * (min + max)) / 2) / (max - min + 1);

  return Z;
}

void updateLCD() {
  float Rule[26];
  char k1[] = "Sangat Buruk";
  char k2[] = "Buruk";
  char k3[] = "Baik";
  char k4[] = "Sangat Baik";

  String ntuString = "";
  kekeruhan = 0;
  // String voltageString = "";
  // float voltage1 = 0;
  if (Serial2.available()) {
    ntuString = Serial2.readString();
    kekeruhan = ntuString.toFloat();
  }

  if (kekeruhan < 0) {
    kekeruhan = 0;
  }

  static unsigned long timepoint = millis();
  // ==============ph=============
  if (millis() - timepoint > 1000U)  //time interval: 1s
  {
    timepoint = millis();
    //voltage = rawPinValue / esp32ADC * esp32Vin
    voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE;  // read the voltage

    phValue = ph.readPH(voltage, temperature);  // convert voltage to pH with temperature compensation
    pH = phValue;
  }
  ph.calibration(voltage, temperature);  // calibration process by Serail CMD
                                         //===========Kekeruhan==================
  int tbValue = analogRead(tb_pin);
  // kekeruhan = map(tbValue, 0, 4095, 100, 0);
  // kekeruhan = 4.6;
  // phValue = 7.80;
  rule(phValue, kekeruhan, Rule);

  if (defuzzyfikasi(Rule) < 1.50) {
    kualitasAir = "Sangat Buruk ";
  } else if (defuzzyfikasi(Rule) < 3.50 && defuzzyfikasi(Rule) >= 1.50) {
    kualitasAir = "Buruk";
  } else if (defuzzyfikasi(Rule) < 5.50 && defuzzyfikasi(Rule) >= 3.50) {
    kualitasAir = "Baik";
  } else if (defuzzyfikasi(Rule) >= 5.50) {
    kualitasAir = "Sangat Baik";
  }


  Serial.println("-----------------------");
  Serial.print("PH : ");
  Serial.println(phValue);

  Serial.print("NTU : ");
  Serial.println(kekeruhan);
  Serial.println("-----------------------");

  Serial.print("Defuzzyfikasi : ");
  Serial.println(defuzzyfikasi(Rule));
  Serial.println("-----------------------");

  Serial.print("Defuzzyfikasi : ");
  Serial.println(kualitasAir);
  Serial.println("-----------------------");


  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("PH :");
  display.println(phValue);
  display.print("NTU:");
  display.println(kekeruhan);
  display.setTextSize(1);
  display.println("");
  display.print("Kwt Air : ");
  display.println(defuzzyfikasi(Rule));

  if (defuzzyfikasi(Rule) < 1.50) {
    display.print(k1);
  } else if (defuzzyfikasi(Rule) < 3.50 && defuzzyfikasi(Rule) >= 1.50) {
    display.print(k2);
  } else if (defuzzyfikasi(Rule) < 5.50 && defuzzyfikasi(Rule) >= 3.50) {
    display.print(k3);
  } else if (defuzzyfikasi(Rule) >= 5.50) {
    display.print(k4);
  }
  display.display();

  // Himpunan Fuzzy PH
  String HimpunanPH;

  if (phValue >= 0 && phValue <= 5.5) {
    HimpunanPH = "Sangat Asam";
  } else if (phValue > 5.5 && phValue <= 6.5) {
    HimpunanPH = "Asam";
  } else if (phValue > 6.5 && phValue <= 7.5) {
    HimpunanPH = "Normal";
  } else if (phValue > 7.5 && phValue <= 9) {
    HimpunanPH = "Basa";
  } else if (phValue > 9) {
    HimpunanPH = "Sangat Basa";
  }

  // // Himpunan Fuzzy Kekeruhan (KK)
  String HimpunanKK;

  if (kekeruhan >= 0 && kekeruhan <= 5) {
    HimpunanKK = "Jernih";
  } else if (kekeruhan > 5 && kekeruhan <= 25) {
    HimpunanKK = "Keruh";
  } else if (kekeruhan > 25) {
    HimpunanKK = "Sangat Keruh";
  }

  float deffuz = defuzzyfikasi(Rule);
  String phValueStr = String(phValue, 2);  // Batasi pH menjadi 2 angka desimal
  String kekeruhanStr = String(kekeruhan, 2);

  antares.add("pH", phValueStr);           // Masukkan pH dengan dua desimal
  antares.add("Kekeruhan", kekeruhanStr);  // Masukkan Kekeruhan dengan dua desimal
  antares.add("Kualitas Air", kualitasAir);
  antares.add("HimpunanPH", HimpunanPH);
  antares.add("HimpunanKK", HimpunanKK);
  antares.add("Defuzzifikasi", deffuz);
  antares.send(projectName, deviceName);
  delay(60000);    
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Menggunakan GPIO16 sebagai RX dan GPIO17 sebagai TX

  EEPROM.begin(32);  //needed to permit storage of calibration value in eeprom
  ph.begin();

  pinMode(tb_pin, INPUT);
  // Start Serial Monitor

  antares.setDebug(true);
  antares.wifiConnection(WIFISSID, PASSWORD);

  // SSD1306_SWITCHCAPVCC = menghasilkan tegangan tampilan dari 3.3V secara internal
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Alokasi SSD1306 gagal"));
    for (;;)
      ;
  }
  display.display();
  display.clearDisplay();
  display.setTextSize(1);               // Skala piksel normal 1:1
  display.setTextColor(SSD1306_WHITE);  // Tampilkan teks putih
  display.setCursor(0, 0);              // Mulai di sudut kiri atas
  display.display();
}

void loop() {
  unsigned long currentMillis = millis();

  // Update LCD jika interval waktu telah terpenuhi
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateLCD();
  }
}
