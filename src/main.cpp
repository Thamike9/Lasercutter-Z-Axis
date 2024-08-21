#include <Arduino.h>
#include <AccelStepper.h>

// Define pin connections
#define DIR_PIN 13                  // Direction pin for stepper driver (GPIO5)
#define STEP_PIN 14                 // Step pin for stepper driver (GPIO4)
#define LIMIT_SWITCH_POSITIVE_PIN 0 // Positive Z-axis limit switch (GPIO2)
#define LIMIT_SWITCH_NEGATIVE_PIN 2 // Negative Z-axis limit switch (GPIO0)
#define BUTTON_UP_PIN 5             // Button for manual up movement (GPIO14)
#define BUTTON_DOWN_PIN 4           // Button for manual down movement (GPIO12)
#define BUTTON_AUTOFOCUS_PIN 12     // Button for autofocus (GPIO13)

// Parameters
#define MOTOR_INTERFACE_TYPE 1 // Use 1 for a driver that needs step and direction
#define MANUAL_SPEED 5000      // Speed for manual control (steps per second)
#define AUTOFOCUS_SPEED 5000   // Speed for autofocus (steps per second)
#define RETRACT_STEPS 1000     // Number of steps to retract when the limit switch is triggered

// Initialize stepper
AccelStepper stepper(MOTOR_INTERFACE_TYPE, STEP_PIN, DIR_PIN);

enum Mode
{
  MODE_MANUAL,
  MODE_AUTOFOCUS
};

Mode currentMode = MODE_MANUAL; // Start in manual mode
bool autofocusInProgress = false;

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize pins
  pinMode(LIMIT_SWITCH_POSITIVE_PIN, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_NEGATIVE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_AUTOFOCUS_PIN, INPUT_PULLUP);

  // Set maximum speed and acceleration for the stepper motor
  stepper.setMaxSpeed(MANUAL_SPEED);
  stepper.setAcceleration(5000); // Higher acceleration for faster speed changes

  Serial.println("System Ready");
  ESP.getResetReason();
}

void loop()
{
  // Serial.printf("UP: %d, DOWN: %d, AUTO: %d, Z+: %d, Z-: %d\n", digitalRead(BUTTON_UP_PIN), digitalRead(BUTTON_DOWN_PIN), digitalRead(BUTTON_AUTOFOCUS_PIN), digitalRead(LIMIT_SWITCH_POSITIVE_PIN), digitalRead(LIMIT_SWITCH_NEGATIVE_PIN));

  switch (currentMode)
  {
  case MODE_MANUAL:
    if (digitalRead(BUTTON_AUTOFOCUS_PIN) == LOW)
    {
      currentMode = MODE_AUTOFOCUS;
      autofocusInProgress = true;
      Serial.println("Switching to Autofocus Mode.");
    }
    else
    {
      // Manual movement mode
      if (digitalRead(BUTTON_UP_PIN) == LOW && digitalRead(LIMIT_SWITCH_POSITIVE_PIN) == HIGH)
      {
        stepper.setSpeed(MANUAL_SPEED);
        stepper.runSpeed();
        Serial.println("Manual Moving UP..");
      }
      else if (digitalRead(BUTTON_DOWN_PIN) == LOW && digitalRead(LIMIT_SWITCH_NEGATIVE_PIN) == HIGH)
      {
        stepper.setSpeed(-MANUAL_SPEED);
        stepper.runSpeed();
        Serial.println("Manual Moving DOWN..");
      }
      else
      {
        stepper.stop();
        stepper.setCurrentPosition(stepper.distanceToGo());
        Serial.printf("Distance to go: %d\n", stepper.distanceToGo());
      }
    }
    break;

  case MODE_AUTOFOCUS:
    if (autofocusInProgress)
    {
      // Move up until the positive limit switch is triggered
      stepper.setSpeed(AUTOFOCUS_SPEED);
      while (digitalRead(LIMIT_SWITCH_POSITIVE_PIN) == HIGH)
      {
        stepper.runSpeed();
        Serial.println("Autofocus Moving UP..");
        Serial.println(digitalRead(LIMIT_SWITCH_POSITIVE_PIN));
      }

      // Stop and retract
      Serial.println("Positive limit switch triggered. Retracting...");
      stepper.stop();
      stepper.move(-RETRACT_STEPS);
      while (stepper.distanceToGo() != 0)
      {
        stepper.run();
      }
      Serial.println("Autofocus Retract Complete.");

      // Autofocus sequence is now complete
      autofocusInProgress = false;
    }
    else if (digitalRead(BUTTON_AUTOFOCUS_PIN) == HIGH)
    {
      // Return to manual mode once the autofocus button is released
      currentMode = MODE_MANUAL;
      Serial.println("Autofocus Complete. Returning to Manual Mode.");
    }
    break;
  }
}