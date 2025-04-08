#include <Arduino.h>

#define INC_PIN 22
#define UD_PIN  21
#define CS_PIN  23

#define UP_BTN    27
#define DOWN_BTN  25
#define STORE_BTN 26

// Constants
#define DEBOUNCE_TIME 50  // Debounce time in milliseconds
#define POT_MAX_STEPS 99  // X9C103 has 100 wiper positions (0-99)

// Function prototypes
void resetToZero();
void setPosition(int position);
void storePosition();

// Variables
int currentPosition = 50;  // Start in the middle position
unsigned long lastDebounceTime = 0;
int lastUpState = HIGH;
int lastDownState = HIGH;
int lastStoreState = HIGH;

void setup() {
  Serial.begin(115200);
  Serial.println("X9C103 Digital Potentiometer Control");
  
  // Initialize X9C103 pins
  pinMode(INC_PIN, OUTPUT);
  pinMode(UD_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  
  // Set initial states
  digitalWrite(INC_PIN, HIGH);
  digitalWrite(CS_PIN, HIGH);
  
  // Initialize input pins with internal pull-up resistors
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(STORE_BTN, INPUT_PULLUP);
  
  Serial.println("Initializing digital potentiometer...");
  
  // Reset to zero position first
  resetToZero();
  
  // Set to initial position (middle)
  setPosition(currentPosition);
  
  Serial.println("Setup complete!");
}

void loop() {
  // Read button states (LOW when pressed due to INPUT_PULLUP)
  int upState = digitalRead(UP_BTN);
  int downState = digitalRead(DOWN_BTN);
  int storeState = digitalRead(STORE_BTN);
  
  // Check for button presses with debounce
  unsigned long currentTime = millis();
  
  // Up button pressed
  if (upState != lastUpState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    if (upState == LOW) {  // Button pressed (LOW due to pull-up)
      if (currentPosition < POT_MAX_STEPS) {
        currentPosition++;
        setPosition(currentPosition);
        Serial.print("Position increased to: ");
        Serial.println(currentPosition);
      } else {
        Serial.println("Already at maximum position");
      }
    }
    lastDebounceTime = currentTime;
  }
  lastUpState = upState;
  
  // Down button pressed
  if (downState != lastDownState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    if (downState == LOW) {  // Button pressed (LOW due to pull-up)
      if (currentPosition > 0) {
        currentPosition--;
        setPosition(currentPosition);
        Serial.print("Position decreased to: ");
        Serial.println(currentPosition);
      } else {
        Serial.println("Already at minimum position");
      }
    }
    lastDebounceTime = currentTime;
  }
  lastDownState = downState;
  
  // Store button pressed
  if (storeState != lastStoreState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    if (storeState == LOW) {  // Button pressed (LOW due to pull-up)
      storePosition();
      Serial.println("Current position stored to non-volatile memory");
    }
    lastDebounceTime = currentTime;
  }
  lastStoreState = storeState;
  
  delay(10);  // Small delay to prevent CPU hogging
}

// Set the wiper to a specific position (0-99)
void setPosition(int position) {
  // Ensure position is within valid range
  position = constrain(position, 0, POT_MAX_STEPS);
  
  // Reset to zero and then increment to target position
  resetToZero();
  
  // Enable the chip
  digitalWrite(CS_PIN, LOW);
  
  // Set direction to up (increment)
  digitalWrite(UD_PIN, HIGH);
  
  // Increment to desired position
  for (int i = 0; i < position; i++) {
    digitalWrite(INC_PIN, LOW);
    delayMicroseconds(1);
    digitalWrite(INC_PIN, HIGH);
    delayMicroseconds(1);
  }
  
  // Disable the chip
  digitalWrite(CS_PIN, HIGH);
  
  // Update current position
  currentPosition = position;
}

// Reset the wiper position to zero
void resetToZero() {
  // Enable the chip
  digitalWrite(CS_PIN, LOW);
  
  // Set direction to down (decrement)
  digitalWrite(UD_PIN, LOW);
  
  // Increment enough times to ensure we reach zero from any position
  for (int i = 0; i < POT_MAX_STEPS + 1; i++) {
    digitalWrite(INC_PIN, LOW);
    delayMicroseconds(1);
    digitalWrite(INC_PIN, HIGH);
    delayMicroseconds(1);
  }
  
  // Disable the chip
  digitalWrite(CS_PIN, HIGH);
}

// Store current wiper position to non-volatile memory
void storePosition() {
  // Enable the chip
  digitalWrite(CS_PIN, LOW);
  
  // Store the current value - requires at least 20ms CS low per datasheet
  delay(20);
  
  // Disable the chip
  digitalWrite(CS_PIN, HIGH);
} 