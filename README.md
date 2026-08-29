# Autonomous Valet Parking & Multi-Sensor Obstacle Avoidance Vehicle

**Institution:** IIIT Hyderabad  
**Team:** Bijli_Ki_Tigdi (Daksh Bhardwaj & Pramodh)  
**Roll Number:** 2025102066 (Daksh)
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

As the automotive industry pivots toward fully autonomous environments, Automated Valet Parking (AVP) has become a critical research domain. This project scales down the complex architectural requirements of an AVP system into a localized, microcontroller-driven environment. 

We engineered a differential-drive robotic vehicle capable of navigating unpredictable indoor corridors, dynamically routing around obstacles without touching the walls, and identifying structurally enclosed spaces that match the geometric profile of a parking bay. By using a strict three-directional threshold logic, the vehicle assumes it has successfully parked the moment it is surrounded by obstacles (walls) on its left, right, and front flanks simultaneously. Once a valid parking slot is detected, the vehicle halts all kinetic movement and enters a secured parked state. It continuously monitors its egress path and executes an "unpark" routine when the exit route clears.

By utilizing an ESP32 microcontroller, the system benefits from a 32-bit dual-core architecture running at 240 MHz, ensuring that multi-directional sensor polling and Pulse Width Modulation (PWM) signal generation occur with near-zero latency.

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
| **Drive Wheels** | 2 | 65mm Rubber Grip | Traction | High-friction rubber tread ensures predictable acceleration and minimizes wheel slip on smooth indoor surfaces, which is critical for turning accuracy. Provided with the chassis in our case. |
| **Caster Wheel** | 1 | Small Metal Ball Type | Chassis Balancing | Acts as a frictionless third point of contact in the rear of the chassis, allowing for smooth zero-radius differential turning. Provided with the chassis in our case. |
| **Power Supply** | 1 | Lab Bench Voltage Regulators | Instead of an onboard battery, external lab regulators were used to supply the necessary voltages (12V and 5V) during prototyping and testing. |
| **Hardware Misc.** | 1 | Breadboard, Jumper Wires, Soldering kit | Circuit Routing | All the other components which are required for basic circuit design in any electronics project. |
---

## 3. Hardware Assembly & Wiring Manual

Due to the specific architecture of the ESP32, standard default pins often conflict with internal flash memory or boot strapping. We mapped the components to safe, dedicated GPIOs.

### A. Ultrasonic Distance Sensor Array
The array acts as the vehicle's spatial awareness. Three HC-SR04 sensors are positioned to create a 180-degree forward-facing field of view.
* **Left Sensor:** `Trig` -> `GPIO 25` | `Echo` -> `GPIO 33`
* **Right Sensor:** `Trig` -> `GPIO 32` | `Echo` -> `GPIO 35` *(Note: GPIO 35 is an input-only pin).*
* **Front Sensor:** `Trig` -> `GPIO 26` | `Echo` -> `GPIO 14`

### B. L298N Dual H-Bridge Motor Driver
Ensure the L298N ground is tied to the ESP32 ground. Remove the ENA/ENB jumpers to allow PWM speed control.
* **Left Motor Control:** `ENA` (Speed) -> `GPIO 15` | `IN1` (Fwd) -> `GPIO 5` | `IN2` (Rev) -> `GPIO 18`
* **Right Motor Control:** `ENB` (Speed) -> `GPIO 4` | `IN3` (Fwd) -> `GPIO 22` | `IN4` (Rev) -> `GPIO 23`

### C. MPU-6050 IMU Sensor
The default I2C pins (21/22) were remapped to avoid layout congestion.
* **Data Line (`SDA`):** `GPIO 17`
* **Clock Line (`SCL`):** `GPIO 19`
* **Power:** `VCC` -> `3.3V` / `5V` | `GND` -> `Common GND`

---

## 4. Power Distribution & Circuit Architecture

Because this prototype was tethered to lab bench power rather than an onboard battery pack, power routing required strict attention to voltage levels to protect the logic circuits.

### The Split-Rail Lab Setup
DC motors generate massive flyback voltage spikes when changing direction. If the ESP32 and the motors shared a single unregulated line, these spikes would trigger the ESP32's internal Brown-Out Detector (BOD), causing a system reboot loop.

To prevent this, the circuit utilizes a split-rail power topology sourced from the lab regulators:
1. **The 12V High-Current Rail:** The lab power supply's 12V output connects strictly to the 12V input terminal of the L298N driver. This rail handles the raw, noisy current required by the DC motors.
2. **The 5V Logic Rail:** A secondary regulated 5V line from the lab equipment is routed directly to the logic side. This clean 5V output powers the ESP32's `VIN` pin and provides the steady voltage required by the three HC-SR04 sensors.
3. **Star Grounding:** The ground lines from the 12V supply, the 5V supply, the L298N, the ESP32, and all sensors are tied together at a single centralized point. This creates a universal 0V reference without creating destructive ground loops.

---

## 5. Kinematic Model & Drive System Theory

The vehicle operates on a **Differential Drive Kinematic Model**. A differential drive system relies on two independently driven parallel wheels and a frictionless rear caster for balance. 

### Steering Dynamics via PWM
We translate this physical model into embedded C++ logic by manipulating the left and right wheels using the L298N driven by 8-bit PWM signals (0-255).
* **Straight Line Cruise:** `ENA = 200`, `ENB = 200`. Both motors receive identical average voltage, resulting in a theoretical straight path.
* **Sweeping Turn:** Abruptly stopping one wheel to turn causes skid steering. Skidding causes the chassis to shudder, introducing massive vibration noise into the ultrasonic sensor readings. Instead, our algorithm employs a sweeping arc differential. To turn left, `ENA` (Left Motor) is dropped to `120` while `ENB` (Right Motor) is maintained at `200`. This reduces the angular velocity on the inner radius, pivoting the chassis smoothly around the slower wheel.

---

## 6. Sensor Theory & Physics of Operation

### A. HC-SR04 Ultrasonic Time-of-Flight (ToF)
1. **Trigger Phase:** The ESP32 sends a 10-microsecond `HIGH` pulse to the sensor.
2. **Acoustic Burst:** The transmitter emits an 8-cycle sonic burst at 40 kHz.
3. **Echo Phase:** The sonic waves bounce off nearby obstacles and return. The `ECHO` pin outputs a `HIGH` signal for the exact duration that the sound wave took to travel out and back.
4. **Mathematical Conversion:** The speed of sound in dry air at 20°C is approximately 343 meters per second (0.0343 cm/us). 
   `Distance (cm) = (Time in microseconds * 0.0343) / 2`

### B. MPU-6050 Micro-Electro-Mechanical Systems (MEMS)
Inside the MPU-6050 chip, microscopic proof masses are suspended by silicon springs. When the vehicle accelerates or rotates, inertia causes these masses to deflect. This deflection changes the internal capacitance, which is converted into a voltage, digitized by a 16-bit ADC, and read by the ESP32 over I2C to maintain spatial awareness.

---

## 7. Software Architecture & State Machine

The firmware is designed with an O(1) time complexity approach per loop iteration. The vehicle evaluates its surroundings against strict distance thresholds: `FORWARD_T = 20`, `LEFT_T = 27`, `RIGHT_T = 20`. These thresholds ensure the vehicle adjusts its path well before it makes physical contact with the walls.

### State Machine Transition Matrix
1. **State 0 (Parked & Locked):** 
   * **Condition:** `parked == true`
   * **Action:** Disregard all navigational logic. Wait 5000ms. If `Front > 20`, the exit is clear. Transition to State 1 (Unpark).
2. **State 1 (Enclosure Detection / Parking):** 
   * **Condition:** `Front < 20 && Left < 27 && Right < 20`
   * **Action:** The vehicle has detected obstacles in all three directions right around the threshold distance. The logic assumes this geometric profile is a parking bay. Halt all motors, set `parked = true`. 
3. **State 2 (Dead-End Escape):** 
   * **Condition:** `Front < 20 && Left > 35 && Right > 35`
   * **Action:** The vehicle is facing a wall, but both sides are wide open. Execute a forced Right Turn to re-orient.
4. **State 3 (Corridor Tracking):** 
   * **Condition:** `Left < 27 && Right < 20 && Front > 20`
   * **Action:** The vehicle is perfectly centered between two walls with a clear path ahead. Maintain Forward Cruise.
5. **State 4 (Frontal Avoidance):** 
   * **Condition:** `Front < 20` (but walls are not uniformly close).
   * **Action:** Compare Left and Right distances. Steer towards the side with the larger integer value.
6. **State 5 (Lateral Correction):** 
   * **Condition:** `Left < 27` OR `Right < 20`
   * **Action:** If too close to the left, steer right. If too close to the right, steer left. This provides dynamic self-centering behavior without touching the boundaries.

---

## 8. Complete Firmware Implementation

```cpp
/**
 * Project: Autonomous Valet Parking System
 * Team: Bijli_Ki_Tigdi (Daksh Bhardwaj, Pramodh)
 * Roll: 2025102066
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
  // Assumes parked if covered by obstacles in all 3 directions right around the threshold
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

  // STATE: ASYMMETRIC WALL CORRECTION (Prevent Wall Touching)
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
