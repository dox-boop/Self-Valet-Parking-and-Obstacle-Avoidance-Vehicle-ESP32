// #include <Wire.h>
// #include <MPU6050_light.h>

// MPU6050 mpu(Wire);

// // Ultrasonic pins
// #define LEFT_TRIG 25
// #define LEFT_ECHO 33
// #define RIGHT_TRIG 32
// #define RIGHT_ECHO 35
// #define FRONT_TRIG 26
// #define FRONT_ECHO 14

// // Motor pins
// #define IN1 5
// #define IN2 18
// #define IN3 22
// #define IN4 23
// #define ENA 15
// #define ENB 4

// // Thresholds
// #define LEFT_T 27
// #define RIGHT_T 20
// #define FORWARD_T 28   // UPDATED HERE

// bool parked = false;
// unsigned long parkedTime = 0;


// // --------------------- SENSOR READ ---------------------
// long readDistance(int trig, int echo) {
//   digitalWrite(trig, LOW);
//   delayMicroseconds(2);
//   digitalWrite(trig, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(trig, LOW);

//   long duration = pulseIn(echo, HIGH, 25000);
//   long distance = duration * 0.034 / 2;

//   if (distance == 0 || distance > 400) return 400;
//   return distance;
// }

// long readLeftSensor() { return readDistance(LEFT_TRIG, LEFT_ECHO); }
// long readRightSensor() { return readDistance(RIGHT_TRIG, RIGHT_ECHO); }
// long readForwardSensor() { return readDistance(FRONT_TRIG, FRONT_ECHO); }


// // ---------------- MOTOR CONTROL ----------------
// void moveForward() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, HIGH);
//   digitalWrite(IN4, LOW);
// }

// void turnLeft() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, HIGH);
//   digitalWrite(IN3, HIGH);
//   digitalWrite(IN4, LOW);
// }

// void turnRight() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, HIGH);
// }

// void stopCar() {
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, LOW);
// }


// // ---------------- PARKING ----------------
// void performParking() {
//   Serial.println("PARKING SUCCESSFUL");
//   stopCar();
//   parked = true;
//   parkedTime = millis();
// }


// // ---------------- SETUP ----------------
// void setup() {
//   Serial.begin(115200);

//   Wire.begin(17, 19);
//   mpu.begin();
//   mpu.calcOffsets();

//   pinMode(LEFT_TRIG, OUTPUT);
//   pinMode(LEFT_ECHO, INPUT);
//   pinMode(RIGHT_TRIG, OUTPUT);
//   pinMode(RIGHT_ECHO, INPUT);
//   pinMode(FRONT_TRIG, OUTPUT);
//   pinMode(FRONT_ECHO, INPUT);

//   pinMode(IN1, OUTPUT);
//   pinMode(IN2, OUTPUT);
//   pinMode(IN3, OUTPUT);
//   pinMode(IN4, OUTPUT);
//   pinMode(ENA, OUTPUT);
//   pinMode(ENB, OUTPUT);

//   stopCar();
// }


// // ---------------- MAIN LOOP ----------------
// void loop() {

//   long L = readLeftSensor();
//   long R = readRightSensor();
//   long F = readForwardSensor();

//   Serial.print("L=");
//   Serial.print(L);
//   Serial.print("  R=");
//   Serial.print(R);
//   Serial.print("  F=");
//   Serial.println(F);


//   // ---------- UN-PARK ----------
//   if (parked) {
//     if (millis() - parkedTime > 5000) {
//       if (F > FORWARD_T) {
//         Serial.println("UNPARK → Moving again");
//         parked = false;
//       }
//     }
//     return;
//   }


//   // ---------- PARK ----------
//   if (F < 20 && L < LEFT_T && R < RIGHT_T) {
//     performParking();
//     return;
//   }


//   // ---------- MOVEMENT LOGIC ----------

//   // FORWARD BLOCKED
//   if (F < FORWARD_T) {
//     if (L > R) {
//       Serial.println("Forward blocked → LEFT");
//       turnLeft();
//     } else {
//       Serial.println("Forward blocked → RIGHT");
//       turnRight();
//     }
//     return;
//   }

//   // SIDE BLOCKED BUT FORWARD CLEAR → KEEP MOVING
//   if (L < LEFT_T && R < RIGHT_T) {
//     Serial.println("Sides blocked but forward clear → FORWARD");
//     moveForward();
//     return;
//   }

//   // LEFT BLOCKED
//   if (L < LEFT_T) {
//     Serial.println("Left blocked → RIGHT");
//     turnRight();
//     return;
//   }

//   // RIGHT BLOCKED
//   if (R < RIGHT_T) {
//     Serial.println("Right blocked → LEFT");
//     turnLeft();
//     return;
//   }

//   // ALL GOOD
//   Serial.println("All clear → FORWARD");
//   moveForward();
// }

// #include <Wire.h>
// #include <MPU6050_light.h>

// MPU6050 mpu(Wire);

// // Ultrasonic pins
// #define LEFT_TRIG 25
// #define LEFT_ECHO 33
// #define RIGHT_TRIG 32
// #define RIGHT_ECHO 35
// #define FRONT_TRIG 26
// #define FRONT_ECHO 14

// // Motor pins
// #define IN1 5
// #define IN2 18
// #define IN3 22
// #define IN4 23
// #define ENA 15
// #define ENB 4

// // Thresholds
// #define LEFT_T 27
// #define RIGHT_T 20
// #define FORWARD_T 28

// bool parked = false;
// unsigned long parkedTime = 0;


// // --------------------- SENSOR READ ---------------------
// long readDistance(int trig, int echo) {
//   digitalWrite(trig, LOW);
//   delayMicroseconds(2);
//   digitalWrite(trig, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(trig, LOW);

//   long duration = pulseIn(echo, HIGH, 25000);
//   long distance = duration * 0.034 / 2;

//   if (distance == 0 || distance > 400) return 400;
//   return distance;
// }

// long readLeftSensor() { return readDistance(LEFT_TRIG, LEFT_ECHO); }
// long readRightSensor() { return readDistance(RIGHT_TRIG, RIGHT_ECHO); }
// long readForwardSensor() { return readDistance(FRONT_TRIG, FRONT_ECHO); }


// // ---------------- MOTOR CONTROL ----------------
// void moveForward() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, HIGH);
//   digitalWrite(IN4, LOW);
// }

// void turnLeft() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, HIGH);
//   digitalWrite(IN3, HIGH);
//   digitalWrite(IN4, LOW);
// }

// void turnRight() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, HIGH);
// }

// void stopCar() {
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, LOW);
// }


// // ---------------- PARKING ----------------
// void performParking() {
//   Serial.println("PARKING SUCCESSFUL");
//   stopCar();
//   parked = true;
//   parkedTime = millis();
// }


// // ---------------- SETUP ----------------
// void setup() {
//   Serial.begin(115200);

//   Wire.begin(17, 19);
//   mpu.begin();
//   mpu.calcOffsets();

//   pinMode(LEFT_TRIG, OUTPUT);
//   pinMode(LEFT_ECHO, INPUT);
//   pinMode(RIGHT_TRIG, OUTPUT);
//   pinMode(RIGHT_ECHO, INPUT);
//   pinMode(FRONT_TRIG, OUTPUT);
//   pinMode(FRONT_ECHO, INPUT);

//   pinMode(IN1, OUTPUT);
//   pinMode(IN2, OUTPUT);
//   pinMode(IN3, OUTPUT);
//   pinMode(IN4, OUTPUT);
//   pinMode(ENA, OUTPUT);
//   pinMode(ENB, OUTPUT);

//   randomSeed(analogRead(0));  // For random turning

//   stopCar();
// }


// // ---------------- MAIN LOOP ----------------
// void loop() {

//   long L = readLeftSensor();
//   long R = readRightSensor();
//   long F = readForwardSensor();

//   Serial.print("L=");
//   Serial.print(L);
//   Serial.print("  R=");
//   Serial.print(R);
//   Serial.print("  F=");
//   Serial.println(F);


//   // ---------- UN-PARK ----------
//   if (parked) {
//     if (millis() - parkedTime > 5000) {
//       if (F > FORWARD_T) {
//         Serial.println("UNPARK → Moving again");
//         parked = false;
//       }
//     }
//     return;
//   }


//   // ---------- PARK ----------
//   if (F < 20 && L < LEFT_T && R < RIGHT_T) {
//     performParking();
//     return;
//   }


//   // ---------- MOVEMENT LOGIC ----------

//   // FORWARD BLOCKED
//   if (F < FORWARD_T) {

//     // NEW RULE: both sides very open → RANDOM TURN (70% RIGHT, 30% LEFT)
//     if (L > 35 && R > 35) {
//       int choice = random(0, 10); // 0–9
//       if (choice < 3) {
//         Serial.println("Forward blocked & sides open → RANDOM LEFT");
//         turnLeft();
//       } else {
//         Serial.println("Forward blocked & sides open → RANDOM RIGHT");
//         turnRight();
//       }
//       return;
//     }

//     // Normal forward-block logic
//     if (L > R) {
//       Serial.println("Forward blocked → LEFT");
//       turnLeft();
//     } else {
//       Serial.println("Forward blocked → RIGHT");
//       turnRight();
//     }
//     return;
//   }

//   // SIDE BLOCKED BUT FORWARD CLEAR → KEEP MOVING
//   if (L < LEFT_T && R < RIGHT_T) {
//     Serial.println("Sides blocked but forward clear → FORWARD");
//     moveForward();
//     return;
//   }

//   // LEFT BLOCKED
//   if (L < LEFT_T) {
//     Serial.println("Left blocked → RIGHT");
//     turnRight();
//     return;
//   }

//   // RIGHT BLOCKED
//   if (R < RIGHT_T) {
//     Serial.println("Right blocked → LEFT");
//     turnLeft();
//     return;
//   }

//   // ALL GOOD
//   Serial.println("All clear → FORWARD");
//   moveForward();
// }


//THIS FOLLOWS CANTOR'S X AND Y AXIS TYPE COORDINATES.

// #include <Wire.h>
// #include <MPU6050_light.h>

// MPU6050 mpu(Wire);

// // Ultrasonic pins
// #define LEFT_TRIG 25
// #define LEFT_ECHO 33
// #define RIGHT_TRIG 32
// #define RIGHT_ECHO 35
// #define FRONT_TRIG 26
// #define FRONT_ECHO 14

// // Motor pins
// #define IN1 5
// #define IN2 18
// #define IN3 22
// #define IN4 23
// #define ENA 15
// #define ENB 4

// // Thresholds
// #define LEFT_T 27
// #define RIGHT_T 20
// #define FORWARD_T 20    // <<< UPDATED HERE

// bool parked = false;
// unsigned long parkedTime = 0;


// // --------------------- SENSOR READ ---------------------
// long readDistance(int trig, int echo) {
//   digitalWrite(trig, LOW);
//   delayMicroseconds(2);
//   digitalWrite(trig, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(trig, LOW);

//   long duration = pulseIn(echo, HIGH, 25000);
//   long distance = duration * 0.034 / 2;

//   if (distance == 0 || distance > 400) return 400;
//   return distance;
// }

// long readLeftSensor() { return readDistance(LEFT_TRIG, LEFT_ECHO); }
// long readRightSensor() { return readDistance(RIGHT_TRIG, RIGHT_ECHO); }
// long readForwardSensor() { return readDistance(FRONT_TRIG, FRONT_ECHO); }


// // ---------------- MOTOR CONTROL ----------------
// void moveForward() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, HIGH);
//   digitalWrite(IN4, LOW);
// }

// void turnLeft() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, HIGH);
//   digitalWrite(IN3, HIGH);
//   digitalWrite(IN4, LOW);
// }

// void turnRight() {
//   analogWrite(ENA, 200);
//   analogWrite(ENB, 200);
//   digitalWrite(IN1, HIGH);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, HIGH);
// }

// void stopCar() {
//   digitalWrite(IN1, LOW);
//   digitalWrite(IN2, LOW);
//   digitalWrite(IN3, LOW);
//   digitalWrite(IN4, LOW);
// }


// // ---------------- PARKING ----------------
// void performParking() {
//   Serial.println("PARKING SUCCESSFUL");
//   stopCar();
//   parked = true;
//   parkedTime = millis();
// }


// // ---------------- SETUP ----------------
// void setup() {
//   Serial.begin(115200);

//   Wire.begin(17, 19);
//   mpu.begin();
//   mpu.calcOffsets();

//   pinMode(LEFT_TRIG, OUTPUT);
//   pinMode(LEFT_ECHO, INPUT);
//   pinMode(RIGHT_TRIG, OUTPUT);
//   pinMode(RIGHT_ECHO, INPUT);
//   pinMode(FRONT_TRIG, OUTPUT);
//   pinMode(FRONT_ECHO, INPUT);

//   pinMode(IN1, OUTPUT);
//   pinMode(IN2, OUTPUT);
//   pinMode(IN3, OUTPUT);
//   pinMode(IN4, OUTPUT);
//   pinMode(ENA, OUTPUT);
//   pinMode(ENB, OUTPUT);

//   stopCar();
// }


// // ---------------- MAIN LOOP ----------------
// void loop() {

//   long L = readLeftSensor();
//   long R = readRightSensor();
//   long F = readForwardSensor();

//   Serial.print("L=");
//   Serial.print(L);
//   Serial.print("  R=");
//   Serial.print(R);
//   Serial.print("  F=");
//   Serial.println(F);


//   // ---------- UN-PARK ----------
//   if (parked) {
//     if (millis() - parkedTime > 5000) {
//       if (F > FORWARD_T) {
//         Serial.println("UNPARK → Moving again");
//         parked = false;
//       }
//     }
//     return;
//   }


//   // ---------- PARK ----------
//   if (F < 20 && L < LEFT_T && R < RIGHT_T) {
//     performParking();
//     return;
//   }


//   // ---------- SPECIAL FORCED RIGHT ----------
//   if (F < FORWARD_T && L > 35 && R > 35) {
//     Serial.println("Forward blocked but sides open → FORCED RIGHT TURN");
//     turnRight();
//     return;
//   }


//   // ---------- MOVEMENT LOGIC ----------

//   if (F < FORWARD_T) {
//     if (L > R) {
//       Serial.println("Forward blocked → LEFT");
//       turnLeft();
//     } else {
//       Serial.println("Forward blocked → RIGHT");
//       turnRight();
//     }
//     return;
//   }

//   if (L < LEFT_T && R < RIGHT_T) {
//     Serial.println("Sides blocked but forward clear → FORWARD");
//     moveForward();
//     return;
//   }

//   if (L < LEFT_T) {
//     Serial.println("Left blocked → RIGHT");
//     turnRight();
//     return;
//   }

//   if (R < RIGHT_T) {
//     Serial.println("Right blocked → LEFT");
//     turnLeft();
//     return;
//   }

//   Serial.println("All clear → FORWARD");
//   moveForward();
// }


#include <Wire.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

// Ultrasonic pins
#define LEFT_TRIG 25
#define LEFT_ECHO 33
#define RIGHT_TRIG 32
#define RIGHT_ECHO 35
#define FRONT_TRIG 26
#define FRONT_ECHO 14

// Motor pins
#define IN1 5
#define IN2 18
#define IN3 22
#define IN4 23
#define ENA 15
#define ENB 4

// Thresholds
#define LEFT_T 27
#define RIGHT_T 20
#define FORWARD_T 20

bool parked = false;
unsigned long parkedTime = 0;


// --------------------- SENSOR READ ---------------------
long readDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 25000);
  long distance = duration * 0.034 / 2;

  if (distance == 0 || distance > 400) return 400;
  return distance;
}

long readLeftSensor() { return readDistance(LEFT_TRIG, LEFT_ECHO); }
long readRightSensor() { return readDistance(RIGHT_TRIG, RIGHT_ECHO); }
long readForwardSensor() { return readDistance(FRONT_TRIG, FRONT_ECHO); }


// ---------------- MOTOR CONTROL ----------------
void moveForward() {
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Smoother turning — left motor slower, right motor normal
void turnLeft() {
  analogWrite(ENA, 120);   // left motor slower
  analogWrite(ENB, 200);   // right motor normal
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Smoother turning — right motor slower, left motor normal
void turnRight() {
  analogWrite(ENA, 200);   // left motor normal
  analogWrite(ENB, 120);   // right motor slower
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// ---------------- PARKING ----------------
void performParking() {
  Serial.println("PARKING SUCCESSFUL");
  stopCar();
  parked = true;
  parkedTime = millis();
}


// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  Wire.begin(17, 19);
  mpu.begin();
  mpu.calcOffsets();

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

  stopCar();
}


// ---------------- MAIN LOOP ----------------
void loop() {

  long L = readLeftSensor();
  long R = readRightSensor();
  long F = readForwardSensor();

  Serial.print("L=");
  Serial.print(L);
  Serial.print("  R=");
  Serial.print(R);
  Serial.print("  F=");
  Serial.println(F);


  // ---------- UN-PARK ----------
  if (parked) {
    if (millis() - parkedTime > 5000) {
      if (F > FORWARD_T) {
        Serial.println("UNPARK → Moving again");
        parked = false;
      }
    }
    return;
  }


  // ---------- PARK ----------
  if (F < 20 && L < LEFT_T && R < RIGHT_T) {
    performParking();
    return;
  }


  // ---------- SPECIAL FORCED RIGHT ----------
  if (F < FORWARD_T && L > 35 && R > 35) {
    Serial.println("Forward blocked but sides open → FORCED RIGHT TURN");
    turnRight();
    return;
  }


  // ---------- NEW CONDITION ----------
  // If both sides close but front free → move forward
  if (L < LEFT_T && R < RIGHT_T && F > FORWARD_T) {
    Serial.println("Both sides close but forward free → MOVE FORWARD");
    moveForward();
    return;
  }


  // ---------- MOVEMENT LOGIC ----------

  if (F < FORWARD_T) {
    if (L > R) {
      Serial.println("Forward blocked → LEFT");
      turnLeft();
    } else {
      Serial.println("Forward blocked → RIGHT");
      turnRight();
    }
    return;
  }

  if (L < LEFT_T) {
    Serial.println("Left blocked → RIGHT");
    turnRight();
    return;
  }

  if (R < RIGHT_T) {
    Serial.println("Right blocked → LEFT");
    turnLeft();
    return;
  }

  Serial.println("All clear → FORWARD");
  moveForward();
}


/*
 * ESP32 Obstacle-Avoiding Robot Car
 * Uses: 2x Ultrasonic + MPU6050 + L298N
 * Method: digitalWrite() and analogWrite() ONLY
 */







