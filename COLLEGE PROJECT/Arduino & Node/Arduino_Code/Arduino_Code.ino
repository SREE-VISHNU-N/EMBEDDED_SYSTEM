#include <Servo.h>
#include <AFMotor.h>
#include <SoftwareSerial.h>

AF_DCMotor motor1(1);   // Fan
Servo myServo;

const int ledPin = 8;
SoftwareSerial nodeMCU(2, 4); // RX, TX

char cmd=0;

void setup() {
  Serial.begin(9600);      // OpenCV (USB)
  nodeMCU.begin(9600);     // NodeMCU

  pinMode(ledPin, OUTPUT);

  motor1.setSpeed(255);
  motor1.run(RELEASE);

  myServo.attach(9);
  myServo.write(90);

  Serial.println("System Ready");
}

void loop() {

  // ----- NodeMCU -----
  if (nodeMCU.available()) {
    cmd = nodeMCU.read();
    Serial.print("NodeMCU: ");
    Serial.println(cmd);
    action(cmd);
  }

  // ----- OpenCV -----
  if (Serial.available()) {
    cmd = Serial.read();
    Serial.print("OpenCV: ");
    Serial.println(cmd);
    action(cmd);
  }
}

void action(char c) {
  if (c == '1') digitalWrite(ledPin, HIGH);   // LED ON
  if (c == '2') digitalWrite(ledPin, LOW);    // LED OFF
  if (c == '3') motor1.run(FORWARD);          // FAN ON
  if (c == '4') motor1.run(RELEASE);          // FAN OFF
  if (c == '5') myServo.write(0);             // SERVO 0°
  if (c == '0') myServo.write(90);            // SERVO 90°
}