#include <Servo.h>

Servo liftServo;  // Create a servo object

// Define pins for buttons (Analog pins A0, A1, A2 on Arduino Mega)
const int button1Pin = A0;
const int button2Pin = A1;
const int button3Pin = A2;

// Define positions for each floor (servo angles)
const int floor1Position = 0;    // Servo angle for Floor 1
const int floor2Position = 90;   // Servo angle for Floor 2
const int floor3Position = 180;  // Servo angle for Floor 3

// Variable to track the current position of the lift (1 = Floor 1, 2 = Floor 2, 3 = Floor 3)
int currentFloor = 1;
int targetFloor = 0;

// Variables to store requests for each floor
bool requestFloor1 = false;
bool requestFloor2 = false;
bool requestFloor3 = false;

// Define 7-segment display pins
const int segA = 5;
const int segB = 6;
const int segC = 7;
const int segD = 8;
const int segE = 9;
const int segF = 10;
const int segG = 11;

// 7-segment digit encoding (0-3)
const byte digitCodes[4][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
};

// Define IR sensor pins for each level
const int irSensor1Pin = 2;  // Sensor for Floor 1
const int irSensor2Pin = 3;  // Sensor for Floor 2
const int irSensor3Pin = 4;  // Sensor for Floor 3

// Function to update the 7-segment display based on the current floor
void updateDisplay(int floor) {
  for (int i = 0; i < 7; i++) {
    pinMode(segA + i, OUTPUT);
    digitalWrite(segA + i, digitCodes[floor - 1][i]);
  }
}

void setup() {
  // Attach the servo to pin 12 on Arduino Mega
  liftServo.attach(12);
 
  // Set buttons as input (Analog pins can be used as digital inputs)
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);

  // Set IR sensor pins as input
  pinMode(irSensor1Pin, INPUT);
  pinMode(irSensor2Pin, INPUT);
  pinMode(irSensor3Pin, INPUT);
 
  // Initialize the lift at the first floor
  liftServo.write(floor1Position);
  updateDisplay(currentFloor);
}

void loop() {
  // Check if any button is pressed and set the corresponding floor request
  if (digitalRead(button1Pin) == LOW && currentFloor != 1) {
    requestFloor1 = true;
  }
  if (digitalRead(button2Pin) == LOW && currentFloor != 2) {
    requestFloor2 = true;
  }
  if (digitalRead(button3Pin) == LOW && currentFloor != 3) {
    requestFloor3 = true;
  }

  // If there is no target floor set, find the next target based on the requests
  if (targetFloor == 0) {
    targetFloor = getNextTarget();
  }

  // If a target floor is set, move the lift
  if (targetFloor != 0) {
    moveToFloor(targetFloor);
  }

  delay(100);  // Small delay to debounce button press
}

// Function to move the lift to the target floor
void moveToFloor(int target) {
  int targetPosition = 0;

  // Determine the servo angle for the target floor
  if (target == 1) {
    targetPosition = floor1Position;
  } else if (target == 2) {
    targetPosition = floor2Position;
  } else if (target == 3) {
    targetPosition = floor3Position;
  }

  // Move the servo to the target position
  int currentPosition = liftServo.read();
 
  // Move smoothly towards the target floor
  if (currentPosition < targetPosition) {
    for (int pos = currentPosition; pos <= targetPosition; pos++) {
      liftServo.write(pos);  // Move servo up in small steps
      delay(20);  // Adjust delay to control speed
    }
  } else if (currentPosition > targetPosition) {
    for (int pos = currentPosition; pos >= targetPosition; pos--) {
      liftServo.write(pos);  // Move servo down in small steps
      delay(20);  // Adjust delay to control speed
    }
  }

  // Recheck the IR sensor just before stopping
  if (checkFloorConditions(target)) {
    currentFloor = target;
  } else {
    targetFloor = getNextTarget(); // If no object is detected, move to the next target
    if (targetFloor != 0) {
      moveToFloor(targetFloor);  // Move to the next floor
    }
  }

  updateDisplay(currentFloor); // Update the display to show the current floor
  clearFloorRequest(target);
  targetFloor = 0;  // Reset target floor after reaching it
}

// Function to get the next target floor based on requests and IR sensor detection
int getNextTarget() {
  // Prioritize floors based on the current position of the lift
  if (currentFloor == 1) {
    if (requestFloor2 && digitalRead(irSensor2Pin) == LOW) return 2;
    if (requestFloor3 && digitalRead(irSensor3Pin) == LOW) return 3;
  } else if (currentFloor == 2) {
    if (requestFloor1 && digitalRead(irSensor1Pin) == LOW) return 1;
    if (requestFloor3 && digitalRead(irSensor3Pin) == LOW) return 3;
  } else if (currentFloor == 3) {
    if (requestFloor2 && digitalRead(irSensor2Pin) == LOW) return 2;
    if (requestFloor1 && digitalRead(irSensor1Pin) == LOW) return 1;
  }

  return 0;  // No valid target found
}

// Function to check floor request and IR sensor before allowing the lift to stop
bool checkFloorConditions(int target) {
  if (target == 1 && requestFloor1 && digitalRead(irSensor1Pin) == LOW) {
    return true; // Stop at Floor 1 if both conditions are met
  }
  if (target == 2 && requestFloor2 && digitalRead(irSensor2Pin) == LOW) {
    return true; // Stop at Floor 2 if both conditions are met
  }
  if (target == 3 && requestFloor3 && digitalRead(irSensor3Pin) == LOW) {
    return true; // Stop at Floor 3 if both conditions are met
  }
  return false; // Otherwise, don't stop at this floor
}

// Function to clear the request for a floor once it's reached
void clearFloorRequest(int floor) {
  if (floor == 1) requestFloor1 = false;
  if (floor == 2) requestFloor2 = false;
  if (floor == 3) requestFloor3 = false;
}
