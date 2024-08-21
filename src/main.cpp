#include <Arduino.h>
#include <AccelStepper.h>

// Define pin connections
#define DIR_PIN D1                   // Direction pin for stepper driver (GPIO5)
#define STEP_PIN D2                  // Step pin for stepper driver (GPIO4)
#define LIMIT_SWITCH_POSITIVE_PIN D4 // Positive Z-axis limit switch (GPIO0)
#define LIMIT_SWITCH_NEGATIVE_PIN D0 // Negative Z-axis limit switch (GPIO2)
#define BUTTON_UP_PIN D5             // Button for manual up movement (GPIO14)
#define BUTTON_DOWN_PIN D6           // Button for manual down movement (GPIO12)

// Parameters
#define MOTOR_INTERFACE_TYPE 1 // Use 1 for a driver that needs step and direction
#define MANUAL_SPEED 10000     // Speed for manual control (steps per second)
#define RETRACT_STEPS 500      // Number of steps to retract when the limit switch is triggered

// Initialize stepper
AccelStepper stepper(MOTOR_INTERFACE_TYPE, STEP_PIN, DIR_PIN);

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize pins
  pinMode(LIMIT_SWITCH_POSITIVE_PIN, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_NEGATIVE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);

  // Set maximum speed and acceleration for the stepper motor
  stepper.setMaxSpeed(MANUAL_SPEED);
  stepper.setAcceleration(10000); // Higher acceleration for faster speed changes

  Serial.println("System Ready");
}

void loop()
{
  // Check the UP button
  if (digitalRead(BUTTON_UP_PIN) == LOW)
  {
    if (digitalRead(LIMIT_SWITCH_POSITIVE_PIN) == HIGH)
    { // Only move if the limit switch is not triggered
      stepper.setSpeed(MANUAL_SPEED);
      stepper.runSpeed(); // Run the motor upwards
    }
  }
  // Check the DOWN button
  else if (digitalRead(BUTTON_DOWN_PIN) == LOW)
  {
    if (digitalRead(LIMIT_SWITCH_NEGATIVE_PIN) == HIGH)
    { // Only move if the limit switch is not triggered
      stepper.setSpeed(-MANUAL_SPEED);
      stepper.runSpeed(); // Run the motor downwards
    }
    else
    {
      // If the negative limit switch is triggered, retract by the defined number of steps
      Serial.println("Negative limit switch triggered. Retracting...");
      stepper.stop();              // Stop the motor first
      stepper.move(RETRACT_STEPS); // Move the motor back by RETRACT_STEPS
      while (stepper.distanceToGo() > 0)
      {
        stepper.run(); // Run the motor to complete the retraction
      }
      Serial.println("Retraction complete.");
    }
  }
  else
  {
    stepper.stop();
  }
}
