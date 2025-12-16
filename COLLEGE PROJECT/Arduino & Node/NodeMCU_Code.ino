#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

// ===== Arduino communication pins =====
SoftwareSerial arduinoSerial(D1, D2);  // D1 = TX → Arduino RX(6), D2 = RX ← Arduino TX(7)

// ===== WiFi details =====
const char* ssid = "sree";
const char* password = "12345678910";

// ===== Web Server =====
ESP8266WebServer server(80);

void handleRoot() {
  String page = "<h1>SV Smart Home Control</h1>"
                "<button onclick=\"location.href='/led_on'\">LED ON</button>"
                "<button onclick=\"location.href='/led_off'\">LED OFF</button><br><br>"
                "<button onclick=\"location.href='/fan_on'\">FAN ON</button>"
                "<button onclick=\"location.href='/fan_off'\">FAN OFF</button><br><br>"
                "<button onclick=\"location.href='/door_open'\">DOOR OPEN</button>"
                "<button onclick=\"location.href='/door_close'\">DOOR CLOSE</button>";

  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(9600);
  arduinoSerial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.println("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("Your control page: ");
  Serial.println(WiFi.localIP());

  // Web routes
  server.on("/", handleRoot);

  server.on("/led_on",    [](){ arduinoSerial.write('1'); server.send(200, "text/plain", "LED ON"); });
  server.on("/led_off",   [](){ arduinoSerial.write('2'); server.send(200, "text/plain", "LED OFF"); });

  server.on("/fan_on",    [](){ arduinoSerial.write('3'); server.send(200, "text/plain", "FAN ON"); });
  server.on("/fan_off",   [](){ arduinoSerial.write('4'); server.send(200, "text/plain", "FAN OFF"); });

  server.on("/door_open", [](){ arduinoSerial.write('0'); server.send(200, "text/plain", "DOOR OPEN"); });
  server.on("/door_close",[](){ arduinoSerial.write('5'); server.send(200, "text/plain", "DOOR CLOSE"); });

  server.begin();
}

void loop() {
  server.handleClient();
}
