# Autonomous Valet Parking & Multi-Sensor Obstacle Avoidance Vehicle

**Institution:** IIIT Hyderabad  
**Team:** Bijli_Ki_Tigdi  
**Lead Developer:** Daksh (Student ID: 2025102066)  
**Platform:** ESP32 (C++ / Arduino Core)  

---

## Table of Contents
1. [Abstract & Project Motivation](#1-abstract--project-motivation)
2. [Kinematic Model & Drive System](#2-kinematic-model--drive-system)
3. [Comprehensive Bill of Materials (BOM)](#3-comprehensive-bill-of-materials-bom)
4. [Sensor Theory & Physics of Operation](#4-sensor-theory--physics-of-operation)
5. [Power Distribution & Circuit Architecture](#5-power-distribution--circuit-architecture)
6. [Software Architecture & State Machine](#6-software-architecture--state-machine)
7. [Complete Annotated Source Code](#7-complete-annotated-source-code)
8. [Calibration, Tuning, & Edge Cases](#8-calibration-tuning--edge-cases)
9. [Future Roadmap: ROS 2 & Embedded Linux Integration](#9-future-roadmap-ros-2--embedded-linux-integration)

---

## 1. Abstract & Project Motivation

As the automotive industry pivots toward fully autonomous environments, Advanced Driver Assistance Systems (ADAS) and Automated Valet Parking (AVP) have become critical research domains. This project scales down the complex architectural requirements of an AVP system into an embedded microcontroller environment. 

The primary objective is to engineer a localized, differential-drive robotic vehicle capable of navigating unpredictable indoor corridors, dynamically routing around obstacles, and identifying structurally enclosed spaces that match the geometric profile of a parking bay. Once a valid parking slot is detected, the vehicle must autonomously maneuver into the space, halt all kinetic movement, and enter a secured parked state. Furthermore, the system is designed to monitor its egress path continuously and execute an "unpark" routine the moment the exit route is cleared.

By utilizing an ESP32 microcontroller over a traditional 8-bit Arduino, the system benefits from a 32-bit dual-core architecture running at 240 MHz. This allows for rapid, non-blocking execution of the state-machine matrix, ensuring that sensor polling, floating-point math for inertial offset calculations, and Pulse Width Modulation (PWM) signal generation occur with near-zero latency. 

---

## 2. Kinematic Model & Drive System

The vehicle operates on a **Differential Drive Kinematic Model**. Unlike standard Ackermann steering found in passenger cars (where front wheels pivot on an axle), a differential drive system relies on two independently driven parallel wheels and a frictionless rear caster for balance. 

### Kinematic Equations
The motion of the robot is entirely dictated by the rotational velocities of the left wheel ($V_L$) and the right wheel ($V_R$).
* **Linear Velocity ($V$):** $V = (V_R + V_L) / 2$
* **Angular Velocity ($\omega$):** $\omega = (V_R - V_L) / L$ *(where $L$ is the track width between the two wheels)*

### Steering Dynamics via PWM
To translate this into embedded C++ logic, we manipulate $V_R$ and $V_L$ using an L298N Dual H-Bridge motor driver controlled via PWM signals. 
* **Straight Line Cruise:** `ENA = 200`, `ENB = 200`. Both motors receive identical average voltage, resulting in a theoretical straight path (assuming identical motor gearbox friction).
* **Sweeping Turn:** Instead of abruptly stopping one wheel to turn, the algorithm employs a sweeping arc differential. To turn left, `ENA` (Left Motor) is dropped to `120` while `ENB` (Right Motor) is maintained at `200`. This reduces the angular velocity on the inner radius, pivoting the chassis smoothly around the slower wheel without causing the rubber tires to skid. Skidding is actively avoided as it introduces massive vibration noise into the ultrasonic sensor readings.

---

## 3. Comprehensive Bill of Materials (BOM)

The hardware for this prototype was carefully selected to balance computational overhead with electrical reliability. The following components are integrated into the final build:

### Core Processing & Actuation
* **Main Microcontroller:** ESP32 Development Board (Upgraded from the initially planned Arduino UNO R3). Acts as the main brain for all control and logic.
* **Motor Driver:** L298N Dual H-Bridge Module (2A peak per channel). Controls the 2 DC motors, allowing independent direction and speed control via PWM.
* **Actuators:** 2x Geared DC Motors (12V, 200-300 RPM). Drives the wheels; provides high torque to overcome the chassis weight.

### Chassis & Mobility
* **Base Frame:** Small 2WD Iron Robot Car Chassis Kit. Acts as the mechanical base for mounting motors, the microcontroller, and sensors.
* **Traction:** 2x 65mm Rubber Wheels. Provides traction and mobility across various indoor floor surfaces.
* **Stability:** 1x Caster Wheel (Small Metal Ball Type). Serves as a balancing wheel for stability and smooth differential turns.

### Sensing & Telemetry
* **Proximity Sensing:** 3x HC-SR04 Ultrasonic Sensors. Detects obstacles and measures distance for the front, left, and right flanks.
* **Inertial Measurement:** 1x MPU-6050 IMU Sensor. Measures orientation, tilt, and yaw to assist with path correction and stability.
* **Location Tracking:** 1x NEO-6M GPS Module (Optional integration for outdoor tracking).

### Power & Connectivity
* **Primary Power Source:** 12V Li-ion / Lead-Acid Battery Pack. The main high-current source for the DC motors and the L298N driver.
* **Logic Voltage Regulation:** 7805 Voltage Regulator Module (LM2596 Buck Converter). Safely steps down and converts the 12V battery rail to a clean 5V regulated output for the sensors and ESP32.
* **Power Hardware:** 1x DC Power Jack (2.1mm), 1x Mini Toggle Switch (On/Off), and 1x USB Cable (Type A to B / Micro for ESP32).
* **Circuit Prototyping:** 1x 400/830-Point Breadboard, 1 Set of Male-to-Male/Male-to-Female Jumper Wires, and 0.8mm Solder Wire Roll.
* **Status Indicators:** 2x LEDs (Red & Green) paired with a Resistor Pack (220 Ohm, 10k Ohm) to visually indicate system ON/OFF status and parking success.

---

## 4. Sensor Theory & Physics of Operation

### A. HC-SR04 Ultrasonic Time-of-Flight (ToF)
The vehicle relies on an array of three HC-SR04 ultrasonic transducers. These operate on the Time-of-Flight principle.
1. **Trigger Phase:** The ESP32 sends a 10-microsecond `HIGH` pulse to the `TRIG` pin.
2. **Acoustic Burst:** The sensor's transmitter emits an 8-cycle sonic burst at 40 kHz (well above human hearing).
3. **Echo Phase:** The sonic waves bounce off nearby obstacles and return to the receiver. The `ECHO` pin outputs a `HIGH` signal for the exact duration that the sound wave took to travel out and back.
4. **Mathematical Conversion:** The speed of sound in dry air at $20^\circ\text{C}$ is approximately 343 meters per second, or 0.0343 centimeters per microsecond. 
   $$\text{Distance (cm)} = \frac{\text{Time} (\mu\text{s}) \times 0.0343}{2}$$
   The division by 2 is crucial because the measured time represents the round-trip distance (to the wall and back).

### B. MPU-6050 Micro-Electro-Mechanical Systems (MEMS)
The MPU-6050 combines a 3-axis gyroscope and a 3-axis accelerometer on a single silicon die. It communicates with the ESP32 via the I2C protocol. Inside the chip, microscopic proof masses are suspended by silicon springs. When the vehicle accelerates or rotates, inertia causes these masses to deflect. This deflection changes the internal capacitance, which is converted into a voltage, digitized by a 16-bit ADC, and read by the ESP32 to maintain spatial awareness.

---

## 5. Power Distribution & Circuit Architecture

Power management in robotics is arguably more critical than logic, as motor stalls can instantly crash a microcontroller. 

### The Ground Loop Problem
DC motors are electrically noisy. When the L298N driver reverses polarity to the motors (changing direction), massive flyback voltage spikes are generated. If the ESP32 and the motors share an unregulated power rail, these voltage spikes will trigger the ESP32's internal Brown-Out Detector (BOD), causing a system reboot loop.

### The Solution: Star Grounding & Regulated Rails
1. **The 12V High-Current Rail:** The 12V battery connects directly to the 12V input of the L298N driver. This rail handles raw, noisy current up to 2-3 Amps.
2. **The 5V Logic Rail:** The 12V line is split and fed into the 7805 Linear Regulator / Buck Converter. This isolates the logic components. The clean 5V output powers the ESP32's `VIN` pin and provides the $V_{CC}$ for all three HC-SR04 sensors.
3. **Common Ground:** The ground pins of the battery, L298N, 7805 regulator, ESP32, and all sensors MUST be tied together at a single point (Star Ground) to ensure a universal 0V reference.

---

## 6. Software Architecture & State Machine

The firmware is designed with an O(1) time complexity approach per loop iteration. Rather than using blocking functions (like `delay()` which halts the CPU), the system uses time-deltas (`millis()`) and sequential threshold logic to guarantee a high refresh rate.

### State Machine Transition Matrix
The vehicle evaluates its surroundings against strict distance thresholds: `FORWARD_T = 20`, `LEFT_T = 27`, `RIGHT_T = 20`.

1. **State 0 (Parked & Locked):** 
   * **Condition:** `parked == true`
   * **Action:** Disregard all navigational logic. Check `millis()` against `parkedTime`. If 5000ms have passed, poll the Front sensor. If `Front > 20`, transition to State 1.
2. **State 1 (Enclosure Detection / Parking):** 
   * **Condition:** `Front < 20 && Left < 27 && Right < 20`
   * **Action:** The vehicle has driven into a 3-sided box. Halt all motors. Set `parked = true`. Transition to State 0.
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
   * **Action:** If too close to the left, steer right. If too close to the right, steer left. This provides self-centering behavior.

---

## 7. Complete Annotated Source Code

The following C++ firmware is engineered for the Arduino Core on ESP32. Every logical block is extensively documented to clarify the underlying data flow.

```cpp
/**
 * Project: Autonomous Valet Parking System
 * Author: Daksh (Bijli_Ki_Tigdi)
 * Institution: IIIT Hyderabad
 * Description: Real-time spatial navigation, obstacle avoidance, and parking slot detection.
 */

#include <Wire.h>
#include <MPU6050_light.h>

// Instantiate the MPU6050 object using the standard Two-Wire Interface (I2C)
MPU6050 mpu(Wire);

// ==========================================
// PIN MAPPING DEFINITIONS
// ==========================================

// Ultrasonic Sensor Pinout (Using ESP32 GPIOs)
#define LEFT_TRIG   25  // Trigger pin for Left HC-SR04
#define LEFT_ECHO   33  // Echo pin for Left HC-SR04
#define RIGHT_TRIG  32  // Trigger pin for Right HC-SR04
#define RIGHT_ECHO  35  // Echo pin for Right HC-SR04 (Input only pin on ESP32)
#define FRONT_TRIG  26  // Trigger pin for Front HC-SR04
#define FRONT_ECHO  14  // Echo pin for Front HC-SR04

// L298N Motor Driver Pinout
#define IN1         5   // Logic Pin 1 (Left Motor Forward)
#define IN2         18  // Logic Pin 2 (Left Motor Reverse)
#define IN3         22  // Logic Pin 3 (Right Motor Forward)
#define IN4         23  // Logic Pin 4 (Right Motor Reverse)
#define ENA         15  // PWM Pin for Left Motor Speed Control
#define ENB         4   // PWM Pin for Right Motor Speed Control

// ==========================================
// CALIBRATED THRESHOLDS & CONSTANTS
// ==========================================
#define LEFT_T      27  // Minimum allowable distance to left wall (cm)
#define RIGHT_T     20  // Minimum allowable distance to right wall (cm)
#define FORWARD_T   20  // Minimum stopping distance for frontal obstacles (cm)

// Global State Trackers
bool parked = false;            // Boolean flag to lock the navigation loop
unsigned long parkedTime = 0;   // Stores the system uptime (in ms) when parking occurred

// ==========================================
// ULTRASONIC ACQUISITION FUNCTIONS
// ==========================================

/**
 * @brief Sends an acoustic burst and measures the time-of-flight echo.
 * @param trig The designated trigger GPIO pin.
 * @param echo The designated echo GPIO pin.
 * @return The calculated distance in centimeters.
 */
long readDistance(int trig, int echo) {
  // 1. Ensure the trigger pin is completely LOW before firing
  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  // 2. Fire the 10-microsecond acoustic trigger pulse
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // 3. Measure the duration of the returning HIGH pulse.
  // The 25000 microsecond timeout prevents the MCU from hanging infinitely
  // if the sound wave is absorbed by a soft surface and never returns.
  long duration = pulseIn(echo, HIGH, 25000);
  
  // 4. Calculate Distance = (Time / 2) * Speed of Sound (0.034 cm/us)
  long distance = duration * 0.034 / 2;

  // 5. Data sanitization: Filter out 0 (timeout) or extreme noise values
  if (distance == 0 || distance > 400) {
    return 400; // Cap at 4 meters (maximum reliable range of HC-SR04)
  }
  return distance;
}

// Wrapper functions for cleaner code in the main loop
long readLeftSensor()    { return readDistance(LEFT_TRIG, LEFT_ECHO); }
long readRightSensor()   { return readDistance(RIGHT_TRIG, RIGHT_ECHO); }
long readForwardSensor() { return readDistance(FRONT_TRIG, FRONT_ECHO); }

// ==========================================
// MOTOR KINEMATICS & ACTUATION
// ==========================================

/**
 * @brief Engages both motors in a forward trajectory at nominal speed.
 */
void moveForward() {
  analogWrite(ENA, 200);   // ~78% Duty Cycle for Left Motor
  analogWrite(ENB, 200);   // ~78% Duty Cycle for Right Motor
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

/**
 * @brief Executes a sweeping left turn by reducing the inner wheel's velocity.
 */
void turnLeft() {
  analogWrite(ENA, 120);   // Decelerate Left Motor
  analogWrite(ENB, 200);   // Maintain Right Motor Speed
  digitalWrite(IN1, LOW);  // Disengage left forward drive
  digitalWrite(IN2, HIGH); // Engage left reverse (assists in tighter radius)
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

/**
 * @brief Executes a sweeping right turn by reducing the inner wheel's velocity.
 */
void turnRight() {
  analogWrite(ENA, 200);   // Maintain Left Motor Speed
  analogWrite(ENB, 120);   // Decelerate Right Motor
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  // Disengage right forward drive
  digitalWrite(IN4, HIGH); // Engage right reverse (assists in tighter radius)
}

/**
 * @brief Halts all motor driver channels.
 */
void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ==========================================
// PARKING STATE HANDLER
// ==========================================

/**
 * @brief Transitions the system into a dormant parking state.
 */
void performParking() {
  Serial.println("\n==================================");
  Serial.println("[SUCCESS] TARGET PARKING SLOT DETECTED.");
  Serial.println("[SYSTEM] Initiating Motor Halt.");
  Serial.println("==================================\n");
  stopCar();
  parked = true;          // Lock the state machine
  parkedTime = millis();  // Record the timestamp of parking
}

// ==========================================
// SYSTEM BOOTSTRAP (SETUP)
// ==========================================

void setup() {
  // Initialize serial telemetry at 115200 baud for fast data output
  Serial.begin(115200);

  // Initialize I2C with custom ESP32 Pins (SDA=17, SCL=19)
  Wire.begin(17, 19);
  
  // Initialize and calibrate the MPU6050
  mpu.begin();
  Serial.println("[BOOT] Calibrating IMU Offsets. Do not move vehicle.");
  mpu.calcOffsets(); // Calculates zero-point bias for gyro/accel
  Serial.println("[BOOT] IMU Calibration complete.");

  // Define Sensor Pin Modes
  pinMode(LEFT_TRIG, OUTPUT);
  pinMode(LEFT_ECHO, INPUT);
  pinMode(RIGHT_TRIG, OUTPUT);
  pinMode(RIGHT_ECHO, INPUT);
  pinMode(FRONT_TRIG, OUTPUT);
  pinMode(FRONT_ECHO, INPUT);

  // Define L298N Motor Driver Pin Modes
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Seed the random number generator using analog noise (useful for random turn biases)
  randomSeed(analogRead(0));

  // Ensure safe boot state
  stopCar();
  Serial.println("[BOOT] Initializing Autonomous Navigation Loop...");
}

// ==========================================
// MAIN LOGIC LOOP (O(1) execution per cycle)
// ==========================================

void loop() {
  // 1. Telemetry Acquisition Phase
  long L = readLeftSensor();
  long R = readRightSensor();
  long F = readForwardSensor();

  // Print localized telemetry for serial plotting/debugging
  Serial.print("Telemetry -> L:"); Serial.print(L);
  Serial.print(" | R:"); Serial.print(R);
  Serial.print(" | F:"); Serial.println(F);

  // -----------------------------------------------------
  // STATE 1: UN-PARKING ROUTINE
  // -----------------------------------------------------
  if (parked) {
    // Non-blocking timer check: Wait for 5 seconds of absolute dwell time
    if (millis() - parkedTime > 5000) {
      // Check if the physical obstacle blocking the parking exit has moved
      if (F > FORWARD_T) {
        Serial.println("[STATE] Egress Path Clear. Executing Un-Park Maneuver.");
        parked = false; // Release the state machine lock
      }
    }
    // Return early to prevent any motor movement while actively parked
    return;
  }

  // -----------------------------------------------------
  // STATE 2: PARKING BAY DETECTION (3-SIDED ENCLOSURE)
  // -----------------------------------------------------
  // If the vehicle registers walls on all three sides simultaneously, 
  // it has successfully entered a parking slot.
  if (F < 20 && L < LEFT_T && R < RIGHT_T) {
    performParking();
    return;
  }

  // -----------------------------------------------------
  // STATE 3: DEAD END OVERRIDE
  // -----------------------------------------------------
  // Forward path is blocked, but both lateral flanks are wide open.
  // Force a hard right turn to navigate out of the T-Junction.
  if (F < FORWARD_T && L > 35 && R > 35) {
    Serial.println("[STATE] Dead-End detected. Both flanks clear. FORCING RIGHT TURN.");
    turnRight();
    return;
  }

  // -----------------------------------------------------
  // STATE 4: CORRIDOR CENTERING (NARROW PASSAGE)
  // -----------------------------------------------------
  // Flanks are tight, but forward path is clear. Maintain forward momentum.
  if (L < LEFT_T && R < RIGHT_T && F > FORWARD_T) {
    Serial.println("[STATE] Narrow Corridor. Walls detected both sides. PUSHING FORWARD.");
    moveForward();
    return;
  }

  // -----------------------------------------------------
  // STATE 5: DYNAMIC OBSTACLE AVOIDANCE
  // -----------------------------------------------------
  // Frontal collision imminent. Compare lateral distances and turn toward open space.
  if (F < FORWARD_T) {
    if (L > R) {
      Serial.println("[STATE] Front Blocked. Left space optimal. TURNING LEFT.");
      turnLeft();
    } else {
      Serial.println("[STATE] Front Blocked. Right space optimal. TURNING RIGHT.");
      turnRight();
    }
    return;
  }

  // -----------------------------------------------------
  // STATE 6: ASYMMETRIC WALL CORRECTION (LANE KEEPING)
  // -----------------------------------------------------
  // Drifted too close to left wall -> Bias Right
  if (L < LEFT_T) {
    Serial.println("[STATE] Proximity Alert: Left Wall. CORRECTING RIGHT.");
    turnRight();
    return;
  }

  // Drifted too close to right wall -> Bias Left
  if (R < RIGHT_T) {
    Serial.println("[STATE] Proximity Alert: Right Wall. CORRECTING LEFT.");
    turnLeft();
    return;
  }

  // -----------------------------------------------------
  // STATE 7: DEFAULT CRUISE
  // -----------------------------------------------------
  // No immediate threats detected. Proceed forward.
  Serial.println("[STATE] Perimeter Clear. CRUISING.");
  moveForward();
}
