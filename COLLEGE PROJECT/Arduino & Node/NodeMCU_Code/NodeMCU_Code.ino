#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>

/* ================= USER CONFIG ================= */
#define WIFI_SSID "sree"
#define WIFI_PASSWORD "12345678910"

#define API_KEY "AIzaSyCk-Ip8o7d6KIW_c36qS1BWK1LDKcOcE-o"
#define DATABASE_URL "https://smarthomesv-f2367-default-rtdb.firebaseio.com/"

#define USER_EMAIL "sreevishnu.n28@gmail.com"
#define USER_PASSWORD "Vi$hnu28"
/* =============================================== */

// Arduino Serial (NodeMCU → Arduino)
SoftwareSerial arduinoSerial(D1, D2);  // TX, RX

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Last values
int lastLed = -1;
int lastFan = -1;
int lastServo = -1;

void setup() {
  Serial.begin(9600);
  arduinoSerial.begin(9600);

  // ===== WiFi =====
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  // ===== Firebase Config =====
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase started");

  // Initial status
  Firebase.RTDB.setString(&fbdo, "/home/deviceStatus", "online");
}

void loop() {

  // ===== LED =====
  if (Firebase.RTDB.getInt(&fbdo, "/home/led")) {
    int led = fbdo.intData();
    if (led != lastLed) {
      arduinoSerial.write(led == 1 ? '1' : '2');
      lastLed = led;
      Serial.println("LED updated");
    }
  }

  // ===== FAN =====
  if (Firebase.RTDB.getInt(&fbdo, "/home/fan")) {
    int fan = fbdo.intData();
    if (fan != lastFan) {
      arduinoSerial.write(fan == 1 ? '3' : '4');
      lastFan = fan;
      Serial.println("FAN updated");
    }
  }

  // ===== SERVO =====
  if (Firebase.RTDB.getInt(&fbdo, "/home/servo")) {
    int servo = fbdo.intData();
    if (servo != lastServo) {
      arduinoSerial.write(servo == 90 ? '0' : '5');
      lastServo = servo;
      Serial.println("SERVO updated");
    }
  }

  delay(300);
}
