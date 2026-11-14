#include <ServoEasing.hpp>


//Testing with GeekServo Grey
// - 360 degrees rotation
// - PWM frequency 50Hz and width 500 to 2500 usec  (1500 usec should be 180 degreees)

// Create servo object
ServoEasing myservo2;

// GPIO pin used to connect the servo
int servoPin2 = 1;

// Servo calibration
const int SERVO_MIN_US = 500;   // 0 degrees
const int SERVO_MAX_US = 2500;  // 180 degrees

// Movement parameters
const int TARGET_ANGLE = 180;
const int MOVEMENT_DURATION_MS = 1500;
const int PAUSE_MS = 3000;
const int ROTATIONAL_SPEED = 70;


void setup() {

  Serial.begin(115200);

  Serial.println("GeekServo ServoEasing test");

  // Attach servo:
  // GPIO, initial position, minimum pulse, maximum pulse,
  // minimum angle, maximum angle
  myservo2.attach(
    servoPin2,
    15,           // Initial position in degrees or microseconds
    SERVO_MIN_US, // 500 us for GeekServo grey (0 degrees)
    SERVO_MAX_US, // 2500 us for GeekServo grey (360 degrees)
    0,            // Min angle = 0
    360           // Max angle = 360
  );

  // Select cubic acceleration/deceleration
  //myservo2.setEasingType(EASE_CUBIC_IN_OUT);
  myservo2.setEasingType(EASE_SINE_IN_OUT);

  // Set speed in degrees per second for all movements
  // In this case you can specify the movement with startEaseTo(180);.
  // An alternativa is to specify startEaseTo(180, 60) to specify the 
  // speed in degrees per second in each of the movement commands
  // A different way of specifying speed is to use stareEaseToD(180, 1500)
  //  where D is for Duration and the second parm 1500 is duration in ms
  myservo2.setSpeed(ROTATIONAL_SPEED);  // setSpeed(60) to set 60 degrees per second

  Serial.println("Servo initialized at 0 degrees");

  delay(1000);
}


void loop() {

  // Move from 0 to 90 degrees
  Serial.println("Moving to 90 degrees...");

  //myservo2.startEaseToD(
  //  TARGET_ANGLE,
  //  MOVEMENT_DURATION_MS
  //);

  myservo2.startEaseTo(
    TARGET_ANGLE
  );

  // Wait until movement finishes
  while (myservo2.isMoving()) {
    delay(1);
  }

  Serial.println("Reached 90 degrees");

  // Wait 3 seconds
  delay(PAUSE_MS);


  // Move back from 90 to 0 degrees
  Serial.println("Moving back to 0 degrees...");

  //myservo2.startEaseToD(
  //  0,
  //  MOVEMENT_DURATION_MS
  //);

  myservo2.startEaseTo(
    0
  );

  // Wait until movement finishes
  while (myservo2.isMoving()) {
    delay(1);
  }

  delay(1000);

  myservo2.startEaseTo(
    90
  );

  // Wait until movement finishes
  while (myservo2.isMoving()) {
    delay(1);
  }

  // Wait 3 seconds
  delay(PAUSE_MS);

  myservo2.startEaseTo(
    0
  );

  // Wait until movement finishes
  while (myservo2.isMoving()) {
    delay(1);
  }

  Serial.println("Reached 0 degrees");

  delay(1000);
}
