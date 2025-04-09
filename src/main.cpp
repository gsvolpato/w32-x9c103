#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>


#define INC_PIN 22
#define UD_PIN  21
#define CS_PIN  23

#define UP_BTN    27
#define DOWN_BTN  25
#define STORE_BTN 26

// Constants
#define DEBOUNCE_TIME 50       // Debounce time in milliseconds
#define POT_MAX_STEPS 99       // X9C103 has 100 wiper positions (0-99)
#define POSITION_FILE "/pot_position.json"
#define CONTINUOUS_DELAY 250   // Delay in ms before continuous mode activates
#define CONTINUOUS_SPEED 100   // Time between steps in continuous mode in ms

// Function prototypes
void resetToZero();
void setPosition(int position);
void storePosition();
void savePositionToSPIFFS();
bool loadPositionFromSPIFFS();
void initSPIFFS();

// Variables
int currentPosition = 50;  // Start in the middle position
unsigned long lastDebounceTime = 0;
unsigned long buttonPressStart = 0; // When a button was first pressed
unsigned long lastContinuousAction = 0; // Last time a continuous action was performed
bool continuousModeActive = false; // If continuous mode is currently active
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
  
  // Initialize SPIFFS and load saved position
  initSPIFFS();
  
  // Reset to zero position first
  resetToZero();
  
  // Set to initial position
  setPosition(currentPosition);
  
  Serial.println("Setup complete!");
}

void loop() {
  // Read button states (LOW when pressed due to INPUT_PULLUP)
  int upState = digitalRead(UP_BTN);
  int downState = digitalRead(DOWN_BTN);
  int storeState = digitalRead(STORE_BTN);
  
  unsigned long currentTime = millis();
  
  // Up button logic
  if (upState != lastUpState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    lastDebounceTime = currentTime;
    if (upState == LOW) {  // Button pressed (LOW due to pull-up)
      // Button just pressed
      buttonPressStart = currentTime;
      continuousModeActive = false;
      
      // Immediately increase position
      if (currentPosition < POT_MAX_STEPS) {
        currentPosition++;
        setPosition(currentPosition);
        Serial.print("Position increased to: ");
        Serial.println(currentPosition);
      } else {
        Serial.println("Already at maximum position");
      }
    }
  }
  lastUpState = upState;
  
  // Down button logic
  if (downState != lastDownState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    lastDebounceTime = currentTime;
    if (downState == LOW) {  // Button pressed (LOW due to pull-up)
      // Button just pressed
      buttonPressStart = currentTime;
      continuousModeActive = false;
      
      // Immediately decrease position
      if (currentPosition > 0) {
        currentPosition--;
        setPosition(currentPosition);
        Serial.print("Position decreased to: ");
        Serial.println(currentPosition);
      } else {
        Serial.println("Already at minimum position");
      }
    }
  }
  lastDownState = downState;
  
  // Continuous adjustment mode
  if (upState == LOW && currentTime - buttonPressStart > CONTINUOUS_DELAY) {
    // Up button held for CONTINUOUS_DELAY
    continuousModeActive = true;
    
    if (currentTime - lastContinuousAction > CONTINUOUS_SPEED) {
      lastContinuousAction = currentTime;
      
      if (currentPosition < POT_MAX_STEPS) {
        currentPosition++;
        setPosition(currentPosition);
        Serial.print("Position increased to: ");
        Serial.println(currentPosition);
      }
    }
  } 
  else if (downState == LOW && currentTime - buttonPressStart > CONTINUOUS_DELAY) {
    // Down button held for CONTINUOUS_DELAY
    continuousModeActive = true;
    
    if (currentTime - lastContinuousAction > CONTINUOUS_SPEED) {
      lastContinuousAction = currentTime;
      
      if (currentPosition > 0) {
        currentPosition--;
        setPosition(currentPosition);
        Serial.print("Position decreased to: ");
        Serial.println(currentPosition);
      }
    }
  }
  
  // Store button pressed
  if (storeState != lastStoreState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    if (storeState == LOW) {  // Button pressed (LOW due to pull-up)
      storePosition();
      savePositionToSPIFFS();
      Serial.println("Current position stored to non-volatile memory and SPIFFS");
    }
    lastDebounceTime = currentTime;
  }
  lastStoreState = storeState;
  
  delay(10);  // Small delay to prevent CPU hogging
}

// Initialize SPIFFS file system
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }
  
  Serial.println("SPIFFS mounted successfully");
  
  if (loadPositionFromSPIFFS()) {
    Serial.println("Position loaded from SPIFFS");
  } else {
    Serial.println("Using default position");
  }
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

// Save the current potentiometer position to SPIFFS
void savePositionToSPIFFS() {
  StaticJsonDocument<64> doc;
  doc["position"] = currentPosition;
  
  File file = SPIFFS.open(POSITION_FILE, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to file");
  }
  
  file.close();
  Serial.println("Position saved to SPIFFS");
}

// Load the potentiometer position from SPIFFS
bool loadPositionFromSPIFFS() {
  if (!SPIFFS.exists(POSITION_FILE)) {
    Serial.println("Position file not found");
    return false;
  }
  
  File file = SPIFFS.open(POSITION_FILE, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return false;
  }
  
  StaticJsonDocument<64> doc;
  DeserializationError error = deserializeJson(doc, file);
  
  file.close();
  
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return false;
  }
  
  currentPosition = doc["position"];
  Serial.print("Loaded position from SPIFFS: ");
  Serial.println(currentPosition);
  return true;
} 