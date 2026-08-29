# Autonomous Valet Parking & Multi-Sensor Obstacle Avoidance Vehicle

**Institution:** IIIT Hyderabad  
**Team:** Bijli_Ki_Tigdi (Daksh Bhardwaj & Pramodh)  
**Platform:** ESP32 (C++ / Arduino Core)  
**Domain:** Embedded Systems, Autonomous Navigation, Spatial Computing  

---

## Table of Contents
1. [Project Abstract & Overview](#1-project-abstract--overview)
2. [Detailed Component Specifications (Bill of Materials)](#2-detailed-component-specifications-bill-of-materials)
3. [Hardware Assembly & Wiring Manual](#3-hardware-assembly--wiring-manual)
4. [Power Distribution & Circuit Architecture](#4-power-distribution--circuit-architecture)
5. [Kinematic Model & Drive System Theory](#5-kinematic-model--drive-system-theory)
6. [Sensor Theory & Physics of Operation](#6-sensor-theory--physics-of-operation)
7. [Software Architecture & State Machine](#7-software-architecture--state-machine)
8. [Complete Firmware Implementation](#8-complete-firmware-implementation)
9. [Calibration, Tuning, & Troubleshooting Guide](#9-calibration-tuning--troubleshooting-guide)

---

## 1. Project Abstract & Overview

As the automotive industry pivots toward fully autonomous environments, Advanced Driver Assistance Systems (ADAS) and Automated Valet Parking (AVP) have become critical research domains. This project scales down the complex architectural requirements of an AVP system into an embedded microcontroller environment. 

We engineered a localized, differential-drive robotic vehicle capable of navigating unpredictable indoor corridors, dynamically routing around obstacles, and identifying structurally enclosed spaces that match the geometric profile of a parking bay. Once a valid parking slot is detected, the vehicle autonomously maneuvers into the space, halts all kinetic movement, and enters a secured parked state. Furthermore, the system continuously monitors its egress path and executes an "unpark" routine the moment the exit route is cleared.

By utilizing an ESP32 microcontroller over a traditional 8-bit Arduino UNO R3, the system benefits from a 32-bit dual-core architecture running at 240 MHz. This allows for rapid, non-blocking execution of the state-machine matrix, ensuring that sensor polling, floating-point math for inertial offset calculations, and Pulse Width Modulation (PWM) signal generation occur with near-zero latency. 

---

## 2. Detailed Component Specifications (Bill of Materials)

The hardware for this prototype was selected to balance computational overhead with electrical reliability. Below is the exhaustive list of components used, their specifications, and their strict engineering role within this system.

| Component | Qty | Specification / Model | Primary Function | Detailed Engineering Role & Justification |
| :--- | :--- | :--- | :--- | :--- |
| **Microcontroller** | 1 | ESP32 Development Board (32-bit, 240MHz) | Central Processing Unit | Replaces the Arduino UNO R3. Handles all sensor polling, threshold decision trees, non-blocking timers, and high-frequency PWM generation. Operates at 3.3V logic. |
| **Motor Driver** | 1 | L298N Dual H-Bridge Module | High-Current Motor Actuation | Receives low-voltage logic signals (IN1-IN4) and PWM signals (ENA, ENB) from the ESP32 to dictate the direction and speed of the 12V motors. Rated for 2A peak per channel. |
| **Ultrasonic Sensor** | 3 | HC-SR04 (40 kHz, 2cm - 400cm range) | Spatial Proximity Sensing | Positioned at Front (0 degrees), Left (90 degrees), and Right (90 degrees). Emits acoustic bursts to measure time-of-flight, providing real-time boundary data for the navigation matrix. |
| **IMU Sensor** | 1 | MPU-6050 (6-DOF MEMS) | Inertial Measurement | Communicates via I2C. Provides real-time 3-axis accelerometer and 3-axis gyroscope data to monitor chassis orientation, tilt, and yaw drift during maneuvers. |
| **Geared DC Motor** | 2 | 12V, 200-300 RPM High Torque | Primary Propulsion | Drives the main wheels. The gear reduction provides sufficient torque to move the heavy chassis from a dead stop without stalling. |
| **Robot Chassis** | 1 | 2WD Iron Stamped Frame | Structural Integrity | Provides a rigid, non-flexing mechanical base to mount all electronic components, preventing sensor misalignment during physical impact. |
| **Drive Wheels** | 2 | 65mm Rubber Grip | Traction | High-friction rubber tread ensures predictable acceleration and minimizes wheel slip on smooth indoor surfaces, which is critical for turning accuracy. |
| **Caster Wheel** | 1 | Small Metal Ball Type | Chassis Balancing | Acts as a frictionless third point of contact in the rear of the chassis, allowing for smooth zero-radius differential turning. |
| **Primary Battery** | 1 | 12V Pack (Li-ion / Lead-Acid) | Main Power Source | Supplies the raw, unregulated high-current surges demanded by the DC motors during acceleration and reversing phases. |
| **Voltage Regulator** | 1 | LM2596 Buck Converter / 7805 | Logic Power Isolation | Steps down the noisy 12V motor rail to a clean, highly regulated 5V output to safely power the ESP32 and the HC-SR04 sensor array without triggering brownouts. |
| **Hardware Misc.** | 1 | Toggle Switch, Breadboard, Jumper Wires | Circuit Routing | Enables hard power cut-offs, safe proto-typing connections, and structured wiring harnesses between the logic board and actuators. |

---

## 3. Hardware Assembly & Wiring Manual

Due to the specific architecture of the ESP32, standard default pins often conflict with internal flash memory or boot strapping. We mapped the components to safe, dedicated GPIOs.

### A. Ultrasonic Distance Sensor Array
The HC-SR04 sensors require a 5V VCC but the ESP32 is a 3.3V device. The ESP32 GPIOs are generally tolerant to short 5V echo pulses, but a voltage divider on the ECHO pins is recommended for long-term safety.
* **Left Sensor:** `Trig` -> `GPIO 25` | `Echo` -> `GPIO 33`
* **Right Sensor:** `Trig` -> `GPIO 32` | `Echo` -> `GPIO 35` *(Note: GPIO 35 is an input-only pin, perfect for reading echoes).*
* **Front Sensor:** `Trig` -> `GPIO 26` | `Echo` -> `GPIO 14`

### B. L298N Dual H-Bridge Motor Driver
Ensure the L298N ground is tied to the ESP32 ground. Remove the ENA/ENB jumpers to allow PWM speed control.
* **Left Motor Control:** `ENA` (Speed) -> `GPIO 15` | `IN1` (Fwd) -> `GPIO 5` | `IN2` (Rev) -> `GPIO 18`
* **Right Motor Control:** `ENB` (Speed) -> `GPIO 4` | `IN3` (Fwd) -> `GPIO 22` | `IN4` (Rev) -> `GPIO 23`

### C. MPU-6050 IMU Sensor
The default I2C pins (21/22) were remapped to avoid layout congestion.
* **Data Line (`SDA`):** `GPIO 17`
* **Clock Line (`SCL`):** `GPIO 19`
* **Power:** `VCC` -> `3.3V` or `5V` (depending on module regulator) | `GND` -> `Common GND`

---

## 4. Power Distribution & Circuit Architecture

Power management in robotics is arguably more critical than logic, as motor stalls can instantly crash a microcontroller. 

### The Ground Loop & Noise Problem
DC motors are electrically noisy. When the L298N driver reverses polarity to the motors (changing direction), massive flyback voltage spikes are generated. If the ESP32 and the motors share an unregulated power rail, these voltage spikes will trigger the ESP32's internal Brown-Out Detector (BOD), causing a system reboot loop.

### The Engineered Solution: Star Grounding & Regulated Rails
To solve this, we implemented a split-rail power topology:
1. **The 12V High-Current Rail:** The 12V battery connects strictly to the 12V input terminal of the L298N driver. This rail handles raw, noisy current surges up to 2-3 Amps.
2. **The 5V Logic Rail:** The 12V line is spliced and fed into the LM2596 Buck Converter (or 7805 regulator). This component magnetically isolates the logic side. The clean 5V output powers the ESP32's `VIN` pin and provides the steady voltage required by all three HC-SR04 sensors.
3. **Star Grounding:** The ground pins of the battery, L298N, Buck Converter, ESP32, and all sensors are tied together at a single centralized point to ensure a universal 0V reference without ground loops.

---

## 5. Kinematic Model & Drive System Theory

The vehicle operates on a **Differential Drive Kinematic Model**. Unlike standard Ackermann steering found in passenger cars (where front wheels pivot on an axle), a differential drive system relies on two independently driven parallel wheels and a frictionless rear caster for balance. 

### Kinematic Equations
The motion of the robot is entirely dictated by the rotational velocities of the left wheel (V_L) and the right wheel (V_R).
* **Linear Velocity (V):** V = (V_R + V_L) / 2
* **Angular Velocity (W):** W = (V_R - V_L) / L *(where L is the track width between the two wheels)*

### Steering Dynamics via PWM
We translate this physical model into embedded C++ logic by manipulating V_R and V_L using the L298N driven by 8-bit PWM signals (0-255).
* **Straight Line Cruise:** `ENA = 200`, `ENB = 200`. Both motors receive identical average voltage, resulting in a theoretical straight path.
* **Sweeping Turn:** Abruptly stopping one wheel to turn causes skid steering. Skidding causes the chassis to shudder, introducing massive vibration noise into the ultrasonic sensor readings. Instead, our algorithm employs a sweeping arc differential. To turn left, `ENA` (Left Motor) is dropped to `120` while `ENB` (Right Motor) is maintained at `200`. This reduces the angular velocity on the inner radius, pivoting the chassis smoothly around the slower wheel.

---

## 6. Sensor Theory & Physics of Operation

### A. HC-SR04 Ultrasonic Time-of-Flight (ToF)
The navigation relies heavily on the Time-of-Flight principle applied to acoustic waves.
1. **Trigger Phase:** The ESP32 sends a 10-microsecond `HIGH` pulse to the sensor.
2. **Acoustic Burst:** The transmitter emits an 8-cycle sonic burst at 40 kHz.
3. **Echo Phase:** The sonic waves bounce off nearby obstacles and return. The `ECHO` pin outputs a `HIGH` signal for the exact duration that the sound wave took to travel out and back.
4. **Mathematical Conversion:** The speed of sound in dry air at 20 degrees Celsius is approximately 343 meters per second (0.0343 cm/us). 
   `Distance (cm) = (Time in microseconds * 0.0343) / 2`
   The division by 2 isolates the one-way distance to the obstacle.

### B. MPU-6050 Micro-Electro-Mechanical Systems (MEMS)
The MPU-6050 combines a 3-axis gyroscope and a 3-axis accelerometer on a single silicon die. Inside the chip, microscopic proof masses are suspended by silicon springs. When the vehicle accelerates or rotates, inertia causes these masses to deflect. This deflection changes the internal capacitance, which is converted into a voltage, digitized by a 16-bit ADC, and read by the ESP32 over I2C to maintain spatial awareness.

---

## 7. Software Architecture & State Machine

The firmware is designed with an O(1) time complexity approach per loop iteration. Rather than using blocking functions (like `delay()` which halts the CPU), the system uses time-deltas (`millis()`) and sequential threshold logic to guarantee a high refresh rate.

The vehicle evaluates its surroundings against strict distance thresholds: `FORWARD_T = 20`, `LEFT_T = 27`, `RIGHT_T = 20`.

### State Machine Transition Matrix
1. **State 0 (Parked & Locked):** 
   * **Condition:** `parked == true`
   * **Action:** Disregard all navigational logic. Check `millis()` against `parkedTime`. If 5000ms have passed, poll the Front sensor. If `Front > 20`, the exit is clear. Transition to State 1.
2. **State 1 (Enclosure Detection / Parking):** 
   * **Condition:** `Front < 20 && Left < 27 && Right < 20`
   * **Action:** The vehicle has driven into a 3-sided box. Halt all motors. Set `parked = true`. Record time. Transition to State 0.
3. **State 2 (Dead-End Escape):** 
   * **Condition:** `Front < 20 && Left > 35 && Right > 35`
   * **Action:** The vehicle is facing a wall, but both sides are wide open. Execute a forced Right Turn to re-orient into an open path.
4. **State 3 (Corridor Tracking):** 
   * **Condition:** `Left < 27 && Right < 20 && Front > 20`
   * **Action:** The vehicle is perfectly centered between two walls with a clear path ahead. Maintain Forward Cruise.
5. **State 4 (Frontal Avoidance):** 
   * **Condition:** `Front < 20` (but walls are not uniformly close).
   * **Action:** Compare Left and Right distances. Steer towards the side with the larger integer value.
6. **State 5 (Lateral Correction):** 
   * **Condition:** `Left < 27` OR `Right < 20`
   * **Action:** If too close to the left, steer right. If too close to the right, steer left. This provides dynamic self-centering behavior.

---

## 8. Complete Firmware Implementation

This is the finalized C++ source code to be compiled and uploaded via the Arduino IDE to the ESP32.

```cpp
/**
 * Project: Autonomous Valet Parking System
 * Team: Bijli_Ki_Tigdi
 * Description: Real-time spatial navigation, obstacle avoidance, and parking slot detection.
 */

#include <Wire.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

// ==========================================
// PIN MAPPING DEFINITIONS
// ==========================================
#define LEFT_TRIG   25
#define LEFT_ECHO   33
#define RIGHT_TRIG  32
#define RIGHT_ECHO  35
#define FRONT_TRIG  26
#define FRONT_ECHO  14

#define IN1         5
#define IN2         18
#define IN3         22
#define IN4         23
#define ENA         15
#define ENB         4

// ==========================================
// CALIBRATED THRESHOLDS & CONSTANTS
// ==========================================
#define LEFT_T      27  // Minimum allowable distance to left wall (cm)
#define RIGHT_T     20  // Minimum allowable distance to right wall (cm)
#define FORWARD_T   20  // Minimum stopping distance for frontal obstacles (cm)

bool parked = false;
unsigned long parkedTime = 0;

// ==========================================
// SENSOR ACQUISITION
// ==========================================
long readDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // 25000us timeout prevents the MCU from hanging if sound wave is lost
  long duration = pulseIn(echo, HIGH, 25000);
  long distance = duration * 0.034 / 2;

  // Filter out 0 (timeout) or extreme noise values
  if (distance == 0 || distance > 400) {
    return 400;
  }
  return distance;
}

long readLeftSensor()    { return readDistance(LEFT_TRIG, LEFT_ECHO); }
long readRightSensor()   { return readDistance(RIGHT_TRIG, RIGHT_ECHO); }
long readForwardSensor() { return readDistance(FRONT_TRIG, FRONT_ECHO); }

// ==========================================
// MOTOR KINEMATICS
// ==========================================
void moveForward() {
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnLeft() {
  analogWrite(ENA, 120);   // Decelerate Left Motor for sweeping arc
  analogWrite(ENB, 200);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH); // Reverse inner wheel for tighter pivot
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  analogWrite(ENA, 200);
  analogWrite(ENB, 120);   // Decelerate Right Motor for sweeping arc
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH); // Reverse inner wheel for tighter pivot
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ==========================================
// PARKING HANDLER
// ==========================================
void performParking() {
  Serial.println("\n[SUCCESS] TARGET PARKING SLOT DETECTED.");
  Serial.println("[SYSTEM] Initiating Motor Halt.\n");
  stopCar();
  parked = true;
  parkedTime = millis();
}

// ==========================================
// SYSTEM BOOTSTRAP
// ==========================================
void setup() {
  Serial.begin(115200);

  // Initialize I2C with custom ESP32 Pins
  Wire.begin(17, 19);
  
  mpu.begin();
  Serial.println("[BOOT] Calibrating IMU Offsets. Do not move vehicle.");
  mpu.calcOffsets();
  Serial.println("[BOOT] IMU Calibration complete.");

  pinMode(LEFT_TRIG, OUTPUT);
  pinMode(LEFT_ECHO, INPUT);
  pinMode(RIGHT_TRIG, OUTPUT);
  pinMode(RIGHT_ECHO, INPUT);
  pinMode(FRONT_TRIG, OUTPUT);
  pinMode(FRONT_ECHO, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  randomSeed(analogRead(0));
  stopCar();
  Serial.println("[BOOT] Initializing Autonomous Navigation Loop...");
}

// ==========================================
// MAIN LOGIC LOOP
// ==========================================
void loop() {
  long L = readLeftSensor();
  long R = readRightSensor();
  long F = readForwardSensor();

  Serial.print("Telemetry -> L:"); Serial.print(L);
  Serial.print(" | R:"); Serial.print(R);
  Serial.print(" | F:"); Serial.println(F);

  // STATE: UN-PARKING ROUTINE
  if (parked) {
    if (millis() - parkedTime > 5000) {
      if (F > FORWARD_T) {
        Serial.println("[STATE] Egress Path Clear. Executing Un-Park Maneuver.");
        parked = false; 
      }
    }
    return;
  }

  // STATE: PARKING BAY DETECTION (3-SIDED ENCLOSURE)
  if (F < 20 && L < LEFT_T && R < RIGHT_T) {
    performParking();
    return;
  }

  // STATE: DEAD END OVERRIDE (Forcing a right turn if stuck)
  if (F < FORWARD_T && L > 35 && R > 35) {
    Serial.println("[STATE] Dead-End detected. FORCING RIGHT TURN.");
    turnRight();
    return;
  }

  // STATE: CORRIDOR CENTERING (NARROW PASSAGE)
  if (L < LEFT_T && R < RIGHT_T && F > FORWARD_T) {
    Serial.println("[STATE] Narrow Corridor centered. PUSHING FORWARD.");
    moveForward();
    return;
  }

  // STATE: DYNAMIC OBSTACLE AVOIDANCE
  if (F < FORWARD_T) {
    if (L > R) {
      Serial.println("[STATE] Front Blocked. TURNING LEFT.");
      turnLeft();
    } else {
      Serial.println("[STATE] Front Blocked. TURNING RIGHT.");
      turnRight();
    }
    return;
  }

  // STATE: ASYMMETRIC WALL CORRECTION
  if (L < LEFT_T) {
    Serial.println("[STATE] Proximity Alert: Left Wall. CORRECTING RIGHT.");
    turnRight();
    return;
  }

  if (R < RIGHT_T) {
    Serial.println("[STATE] Proximity Alert: Right Wall. CORRECTING LEFT.");
    turnLeft();
    return;
  }

  // STATE: DEFAULT CRUISE
  Serial.println("[STATE] Perimeter Clear. CRUISING.");
  moveForward();
}
