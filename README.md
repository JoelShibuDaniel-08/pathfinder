# PathFinder: Autonomous Obstacle-Avoidance Vehicle

A small robot car that drives on its own and steers around obstacles. An
ultrasonic sensor mounted on a servo measures the distance ahead. When
something gets too close, the car stops, looks left and right, and turns
toward the side with more free space.

## Hardware
- Arduino Nano (or Uno)
- HC-SR04 ultrasonic sensor
- SG90 servo motor (carries the ultrasonic sensor)
- L298N motor driver
- 2 DC gear motors and a chassis
- Battery pack

## How it works
1. The car drives forward while reading distance from the ultrasonic sensor.
   Each reading is the median of 3 samples, which filters out random spikes.
2. If an obstacle is closer than 25 cm, the car stops.
3. The servo turns the sensor left, then right, measuring the distance on each side.
4. The car turns toward the clearer side. If both sides are blocked, it backs
   up and turns around.
5. The loop repeats.

## Wiring
| Part | Arduino pin |
|---|---|
| HC-SR04 TRIG / ECHO | D7 / D8 |
| Servo signal | D9 |
| L298N ENA / IN1 / IN2 (left motor) | D5 / D2 / D3 |
| L298N ENB / IN3 / IN4 (right motor) | D6 / D4 / D10 |

Connect all grounds together. Change the pin constants at the top of
`pathfinder.ino` if your wiring differs.

## Setup
1. Open `pathfinder.ino` in the Arduino IDE.
2. Select your board and port, then upload.
3. Open the Serial Monitor at 9600 baud to see distance readings.
4. Tune `OBSTACLE_CM`, `DRIVE_SPEED`, and `TURN_TIME_MS` for your chassis.

## Author
Joel Shibu Daniel
