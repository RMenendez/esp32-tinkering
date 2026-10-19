// ESP32-S3 PWM test
// Motor A: GPIO7 to IN1 and GPIO6 to IN2
// Motor B: GPIO4 to IN3 and GPIO5 to IN4
// Frequency: 10 kHz
// Resolution: 8 bits (0-255)

#include <Arduino.h>

// Driver Pin Definitions
const int MOTOR_A_IN1_PIN = 7; // ESP32-S3 GPIO7 -> DRV8833 IN1
const int MOTOR_A_IN2_PIN = 6; // ESP32-S3 GPIO6 -> DRV8833 IN2
const int MOTOR_B_IN3_PIN = 4; // ESP32-S3 GPIO4 -> DRV8833 IN3
const int MOTOR_B_IN4_PIN = 5; // ESP32-S3 GPIO5 -> DRV8833 IN4

enum Motor {
  MOTOR_A,
  MOTOR_B
};

// PWM Configuration Parameters
const int PWM_FREQ = 10000;    // 10 kHz (silent, non-audible frequency)
const int PWM_RES  = 8;        // 8-bit resolution (Duty cycle values: 0 to 255)

// --- DRV8833 Direction Control Functions ---

// Forward: PWM on IN1, IN2 set to LOW
void driveForward(Motor motor, uint8_t speed) {
  const int firstPin = motor == MOTOR_A ? MOTOR_A_IN1_PIN : MOTOR_B_IN3_PIN;
  const int secondPin = motor == MOTOR_A ? MOTOR_A_IN2_PIN : MOTOR_B_IN4_PIN;
  ledcWrite(firstPin, speed);
  ledcWrite(secondPin, 0);
}

// Reverse: IN1 set to LOW, PWM on IN2
void driveReverse(Motor motor, uint8_t speed) {
  const int firstPin = motor == MOTOR_A ? MOTOR_A_IN1_PIN : MOTOR_B_IN3_PIN;
  const int secondPin = motor == MOTOR_A ? MOTOR_A_IN2_PIN : MOTOR_B_IN4_PIN;
  ledcWrite(firstPin, 0);
  ledcWrite(secondPin, speed);
}

// Coast/Brake: Both inputs set to LOW
void driveBrake(Motor motor) {
  const int firstPin = motor == MOTOR_A ? MOTOR_A_IN1_PIN : MOTOR_B_IN3_PIN;
  const int secondPin = motor == MOTOR_A ? MOTOR_A_IN2_PIN : MOTOR_B_IN4_PIN;
  ledcWrite(firstPin, 0);
  ledcWrite(secondPin, 0);
}

// Helper function to ramp speed up and down in a given direction
void runRampSequence(const char* motorLabel, Motor motor,
                     const char* directionLabel,
                     void (*driveFunc)(Motor, uint8_t)) {
  const uint8_t steps[] = {0, 64, 128, 192, 255};
  const int numSteps = sizeof(steps) / sizeof(steps[0]);
  const int stepDurationMs = 1200; // Hold each step for 1.2 seconds

  // 1. RAMP UP
  Serial.print("\n=== Starting RAMP UP (");
  Serial.print(motorLabel);
  Serial.print(" ");
  Serial.print(directionLabel);
  Serial.println(") ===");
  for (int i = 0; i < numSteps; i++) {
    uint8_t speed = steps[i];
    Serial.print("Step ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.print(numSteps);
    Serial.print(" -> Speed: ");
    Serial.print(speed);
    Serial.print(" (");
    Serial.print((speed * 100) / 255);
    Serial.println("%)");

    driveFunc(motor, speed);
    delay(stepDurationMs);
  }

  // 2. RAMP DOWN
  Serial.print("=== Starting RAMP DOWN (");
  Serial.print(motorLabel);
  Serial.print(" ");
  Serial.print(directionLabel);
  Serial.println(") ===");
  for (int i = numSteps - 1; i >= 0; i--) {
    uint8_t speed = steps[i];
    Serial.print("Step ");
    Serial.print(numSteps - i);
    Serial.print("/");
    Serial.print(numSteps);
    Serial.print(" -> Speed: ");
    Serial.print(speed);
    Serial.print(" (");
    Serial.print((speed * 100) / 255);
    Serial.println("%)");

    driveFunc(motor, speed);
    delay(stepDurationMs);
  }

  // Brief brake after ramp sequence completes
  driveBrake(motor);
  delay(500);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("DRV8833 + ESP32-S3 Forward & Reverse Test Initialized.");

  // Attach GPIO pins to PWM generator with frequency and resolution
  ledcAttach(MOTOR_A_IN1_PIN, PWM_FREQ, PWM_RES);
  ledcAttach(MOTOR_A_IN2_PIN, PWM_FREQ, PWM_RES);
  ledcAttach(MOTOR_B_IN3_PIN, PWM_FREQ, PWM_RES);
  ledcAttach(MOTOR_B_IN4_PIN, PWM_FREQ, PWM_RES);

  driveBrake(MOTOR_A);
  driveBrake(MOTOR_B);
}

void loop() {
  // Test ramp up and down in both directions for each motor
  runRampSequence("MOTOR A", MOTOR_A, "FORWARD", driveForward);
  runRampSequence("MOTOR A", MOTOR_A, "REVERSE", driveReverse);
  runRampSequence("MOTOR B", MOTOR_B, "FORWARD", driveForward);
  runRampSequence("MOTOR B", MOTOR_B, "REVERSE", driveReverse);

  // Pause at the end of the full loop
  Serial.println("\nFull test loop complete. Waiting 3 seconds before restart...\n");
  delay(3000);
}