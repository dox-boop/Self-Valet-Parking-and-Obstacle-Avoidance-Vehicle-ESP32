# Autonomous Valet Parking & Obstacle Avoidance Vehicle (ESP32)

An embedded robotics system engineered for autonomous indoor navigation, dynamic obstacle avoidance, corridor tracking, and automated parking bay docking. Developed by team **Bijli_Ki_Tigdi** (Daksh Bhardwaj & Pramodh) for academic research and application at IIIT Hyderabad.

## 1. Project Background & System Architecture

As autonomous vehicle technology scales down to embedded microcontrollers, implementing reliable localized navigation without expensive LiDAR or high-overhead compute platforms remains a critical engineering challenge. This project implements a differential-drive robot car designed to autonomously cruise corridors, avoid dynamic and static obstacles, detect available three-sided parking slots, park itself, and unpark when the slot entrance clears.

The core architecture balances real-time low-level motor actuation, multi-sensor data fusion, and deterministic state-machine navigation. The control algorithm is written in C++ and optimized for the ESP32 platform, leveraging non-blocking timers and hardware-specific I2C mapping.

## 2. Hardware Bill of Materials (BOM)

To build a structurally rigid and electrically isolated platform, we utilized a custom 2WD iron chassis powered by high-torque geared DC motors. 

| Component | Specification | Qty | Role & Engineering Justification |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | ESP32 Development Board | 1 | Executes sensor polling, threshold decision trees, and non-blocking timers. |
| **Motor Driver** | L298N Dual H-Bridge | 1 | Handles DC motor bidirectional drive and receives asymmetric PWM for steering. |
| **DC Motors** | 12V Geared (200-300 RPM) | 2 | Primary wheel actuators mounted on a differential-drive axle. |
| **Chassis** | Small 2WD Iron Chassis | 1 | Provides mechanical rigidity alongside a metal caster ball for zero-radius turning. |
| **Sensing** | HC-SR04 Ultrasonic Sensors | 3 | Positioned at Front, Left, and Right for perimeter awareness. |
| **Sensing** | MPU-6050 6-DOF IMU | 1 | Interfaced via custom I2C pins for heading stabilization and offset calibration. |
| **Power** | 12V Battery Pack | 1 | Supplies unregulated high-current surges demanded by the drive motors. |
| **Regulator** | 7805 Linear / LM2596 Buck | 1 | Supplies clean, noise-isolated 5V logic power to the ESP32 and sensor rails. |
| **Miscellaneous** | 65mm Wheels, Toggle Switch | - | High-friction traction, power switching, and LED status indicators. |

## 3. Pin Mapping & Hardware Wiring

Because standard ESP32 I2C pins conflict with certain digital assignments on custom layouts, the hardware I2C bus is remapped via software to custom GPIOs.

**A. Ultrasonic Distance Sensor Array**
* Left HC-SR04: `Trig` -> `GPIO 25` | `Echo` -> `GPIO 33`
* Right HC-SR04: `Trig` -> `GPIO 32` | `Echo` -> `GPIO 35`
* Front HC-SR04: `Trig` -> `GPIO 26` | `Echo` -> `GPIO 14`

**B. L298N Dual H-Bridge Motor Driver**
* Left Motor: `ENA` (PWM) -> `GPIO 15` | `IN1` -> `GPIO 5` | `IN2` -> `GPIO 18`
* Right Motor: `ENB` (PWM) -> `GPIO 4` | `IN3` -> `GPIO 22` | `IN4` -> `GPIO 23`

**C. MPU-6050 IMU Sensor**
* `SDA` -> `GPIO 17` | `SCL` -> `GPIO 19`

## 4. State Machine & Navigation Algorithm

The control loop operates as a priority-encoded decision tree evaluated on every cycle. 

### Execution Priority Rules:
1. **Unparking Dwell Filter:** When parked, the vehicle locks all motor actuation for at least 5000 ms. Once expired, it monitors the front sensor. If clearance (>20 cm) is detected, it unparks automatically.
2. **Parking Bay Detection:** If the vehicle enters a slot where Front < 20 cm, Left < 27 cm, and Right < 20 cm, the car immediately kills motor power and flags a successful park.
3. **Dead-End Handling:** If Front < 20 cm while both sides are wide open (>35 cm), the system enforces a forced right-angle sweep to continue mapping corridors.
4. **Corridor Centering:** When both sides detect walls within threshold limits but the forward path is open, the car maintains forward cruise.
5. **Dynamic Obstacle Avoidance:** When blocked directly ahead, the car compares left and right clearances and pivots toward the wider space.
6. **Asymmetric Wall Correction:** If drifted too close to the left wall (<27 cm), it steers right. If too close to the right wall (<20 cm), it steers left.

## 5. Parameter & Constant Analysis

To achieve steady trajectories without hunting or wall collisions, every constant in the firmware was experimentally tuned.

* **FORWARD_T (20 cm):** Minimum stopping distance. At cruising speeds, this 20 cm window gives the L298N driver and DC motor gearboxes sufficient deceleration margin.
* **LEFT_T (27 cm) & RIGHT_T (20 cm):** Asymmetrical side thresholds compensate for the sensor mount offsets on the chassis and provide a directional bias when tracking walls, preventing high-frequency oscillation.
* **Cruising Duty Cycle (200/255):** Translates to ~78.4% PWM duty cycle, delivering enough torque to overcome static friction while keeping motor RPM manageable.
* **Turn Ratios (120 vs 200):** Instead of stopping one motor entirely (causing harsh skid turns), dropping the inner motor to PWM 120 while keeping the outer motor at PWM 200 produces a clean, sweeping arc.
* **Pulse In Timeout (25000 µs):** Bounding the timeout to 25 ms limits ultrasonic range searching to ~4.25 m, keeping main loop iteration latency under 30 ms.

## 6. Hardware Calibration & Troubleshooting Guide

* **IMU Offset Drift:** During boot, the firmware calls `mpu.calcOffsets()`. The vehicle must remain completely stationary on a flat surface for the first 3 seconds after boot to prevent zero-point drift.
* **Ultrasonic Sensor Crosstalk:** HC-SR04 sensors can catch acoustic reflections from nearby smooth walls. If false triggers occur, slightly angle the side sensors backward relative to the lateral axis.
* **Brownout Resets under Motor Stall:** The ESP32 internal brownout detector will trip if high-current motor surges drop the 5V logic line. Ensure the 7805 regulator is properly sinking heat and the logic line is isolated from the 12V motor supply.

## 7. Future Scope

* Implement a real-time PID loop using MPU-6050 yaw rate data for strict straight-line correction.
* Migrate ultrasonic polling to ESP32 Core 0 and run state logic on Core 1 using FreeRTOS.
* Integrate micro-ROS publisher nodes for Wi-Fi telemetry broadcasting to an external Ubuntu ROS 2 environment.
