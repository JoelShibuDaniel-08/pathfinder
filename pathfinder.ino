/*
  PathFinder - Autonomous Obstacle-Avoidance Vehicle
  Board: Arduino Nano (or Uno)

  An HC-SR04 ultrasonic sensor is mounted on a servo. The vehicle drives
  forward until something is closer than OBSTACLE_CM, stops, sweeps the servo
  left and right to measure the free distance on each side, then turns toward
  the clearer side (or reverses and turns if both sides are blocked).

  Wiring (assumed - change the pins below to match your build):
    HC-SR04  : TRIG -> D7, ECHO -> D8, VCC -> 5V, GND -> GND
    Servo    : signal -> D9 (power from 5V / external supply, common GND)
    L298N    : ENA -> D5 (PWM), IN1 -> D2, IN2 -> D3
               ENB -> D6 (PWM), IN3 -> D4, IN4 -> D10
    Motors   : left motor on OUT1/OUT2, right motor on OUT3/OUT4
*/

#include <Servo.h>

// ---------- Pins ----------
const int TRIG_PIN = 7;
const int ECHO_PIN = 8;
const int SERVO_PIN = 9;

const int ENA = 5;   // left motor speed (PWM)
const int IN1 = 2;
const int IN2 = 3;
const int ENB = 6;   // right motor speed (PWM)
const int IN3 = 4;
const int IN4 = 10;

// ---------- Settings ----------
const int OBSTACLE_CM = 25;     // stop and avoid if closer than this
const int DRIVE_SPEED = 160;    // 0-255
const int TURN_SPEED = 170;     // 0-255
const int TURN_TIME_MS = 450;   // how long to turn (tune for your chassis)
const int BACKUP_TIME_MS = 400;
const int SERVO_LEFT = 160;     // servo angles for scanning
const int SERVO_CENTER = 90;
const int SERVO_RIGHT = 20;
const int SERVO_SETTLE_MS = 450;
const unsigned long ECHO_TIMEOUT_US = 25000; // ~4 m max range

Servo scanServo;

// ---------- Distance ----------
// Returns distance in cm, or 400 if nothing is detected / timeout.
int readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return 400;
  return (int)(duration * 0.0343 / 2.0);
}

// Median of 3 readings to ignore random spikes.
int readDistanceFiltered() {
  int a = readDistanceCm();
  delay(15);
  int b = readDistanceCm();
  delay(15);
  int c = readDistanceCm();
  if ((a >= b && a <= c) || (a <= b && a >= c)) return a;
  if ((b >= a && b <= c) || (b <= a && b >= c)) return b;
  return c;
}

// ---------- Motors ----------
void setMotors(bool leftFwd, bool rightFwd, int leftSpeed, int rightSpeed) {
  digitalWrite(IN1, leftFwd ? HIGH : LOW);
  digitalWrite(IN2, leftFwd ? LOW : HIGH);
  digitalWrite(IN3, rightFwd ? HIGH : LOW);
  digitalWrite(IN4, rightFwd ? LOW : HIGH);
  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

void driveForward() { setMotors(true, true, DRIVE_SPEED, DRIVE_SPEED); }
void driveBackward() { setMotors(false, false, DRIVE_SPEED, DRIVE_SPEED); }
void turnLeft() { setMotors(false, true, TURN_SPEED, TURN_SPEED); }
void turnRight() { setMotors(true, false, TURN_SPEED, TURN_SPEED); }

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ---------- Scanning ----------
int lookAt(int angle) {
  scanServo.write(angle);
  delay(SERVO_SETTLE_MS);
  return readDistanceFiltered();
}

void avoidObstacle() {
  stopMotors();
  delay(150);

  int leftDist = lookAt(SERVO_LEFT);
  int rightDist = lookAt(SERVO_RIGHT);
  scanServo.write(SERVO_CENTER);
  delay(SERVO_SETTLE_MS);

  Serial.print("Left: ");
  Serial.print(leftDist);
  Serial.print(" cm | Right: ");
  Serial.print(rightDist);
  Serial.println(" cm");

  if (leftDist < OBSTACLE_CM && rightDist < OBSTACLE_CM) {
    // Boxed in: back up and make a longer turn
    driveBackward();
    delay(BACKUP_TIME_MS);
    turnRight();
    delay(TURN_TIME_MS * 2);
  } else if (leftDist > rightDist) {
    turnLeft();
    delay(TURN_TIME_MS);
  } else {
    turnRight();
    delay(TURN_TIME_MS);
  }
  stopMotors();
  delay(100);
}

// ---------- Arduino ----------
void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  scanServo.attach(SERVO_PIN);
  scanServo.write(SERVO_CENTER);
  stopMotors();

  Serial.println("PathFinder ready");
  delay(1000);
}

void loop() {
  int distance = readDistanceFiltered();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < OBSTACLE_CM) {
    avoidObstacle();
  } else {
    driveForward();
  }
  delay(40);
}
