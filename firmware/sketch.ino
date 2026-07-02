#include <SPI.h>
#include <MFRC522.h>
#include <DS1302.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN 9
#define SS_PIN 10
#define CLK_PIN 7
#define DAT_PIN 6
#define RST_DS_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN);
DS1302 rtc(RST_DS_PIN, DAT_PIN, CLK_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Card {
  String uid;
  String name;
};

Card knownCards[] = {
  {"DE AD BE EF", "Aryan"},
};
int cardCount = 1;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Attendance Sys");
  lcd.setCursor(0, 1);
  lcd.print("Scan your card");

  rtc.setDOW(WEDNESDAY);
  rtc.setTime(18, 53, 0);
  rtc.setDate(1, 7, 2026);

  Serial.println("Name,UID,Date,Time");
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uid += " ";
  }
  uid.toUpperCase();
  return uid;
}

String lookupName(String uid) {
  for (int i = 0; i < cardCount; i++) {
    if (knownCards[i].uid == uid) return knownCards[i].name;
  }
  return "Unknown";
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = getUID();
  String name = lookupName(uid);

  Time t = rtc.getTime();

  String date = String(t.date) + "/" + String(t.mon) + "/" + String(t.yr);
  String timeStr = String(t.hour) + ":" +
                   (t.min < 10 ? "0" : "") + String(t.min) + ":" +
                   (t.sec < 10 ? "0" : "") + String(t.sec);

  Serial.println(name + "," + uid + "," + date + "," + timeStr);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(name.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(timeStr + " " + date);

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Attendance Sys");
  lcd.setCursor(0, 1);
  lcd.print("Scan your card");

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}