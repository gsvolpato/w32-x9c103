#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ESP32Encoder.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1       // Reset pin (or -1 if sharing Arduino reset pin)
#define I2C_ADDRESS 0x3C    // 0x3C for most SH1106 displays
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// X9C103 Digital Potentiometer Pins - Bass Control
#define BASS_INC_PIN 22
#define BASS_UD_PIN  21
#define BASS_CS_PIN  23

// X9C103 Digital Potentiometer Pins - Treble Control
#define TREBLE_INC_PIN 19
#define TREBLE_UD_PIN  18
#define TREBLE_CS_PIN  5

// Rotary Encoder Pins
#define BASS_ENC_A 32
#define BASS_ENC_B 33
#define BASS_ENC_SW 25  // Switch/button inside encoder

#define TREBLE_ENC_A 26
#define TREBLE_ENC_B 27
#define TREBLE_ENC_SW 14  // Switch/button inside encoder

// Footswitch Pins
#define BYPASS_FS_PIN 12
#define BOOST_FS_PIN 13

// Transistor Base Control Pins
#define BASS_TRANSISTOR_PIN 4
#define TREBLE_TRANSISTOR_PIN 2
#define BOOST_TRANSISTOR_PIN 15

// Status LED Pin
#define STATUS_LED_PIN 16

// Constants
#define DEBOUNCE_TIME 50       // Debounce time in milliseconds
#define POT_MAX_STEPS 99       // X9C103 has 100 wiper positions (0-99)
#define POSITION_FILE "/pot_positions.json"
#define BASS_CENTER 50         // Center position for bass control
#define TREBLE_CENTER 50       // Center position for treble control
#define DISPLAY_UPDATE_INTERVAL 100 // Display update interval in milliseconds

// Function prototypes
void resetToZero(uint8_t inc_pin, uint8_t ud_pin, uint8_t cs_pin);
void setPosition(uint8_t inc_pin, uint8_t ud_pin, uint8_t cs_pin, int position, int* currentPosition);
void storePosition(uint8_t cs_pin);
void savePositionsToSPIFFS();
bool loadPositionsFromSPIFFS();
void initSPIFFS();
void handleBassEncoder();
void handleTrebleEncoder();
void handleFootswitches();
void updateLED();
void toggleBypass();
void toggleBoost();
void initDisplay();
void updateDisplay();
void drawBar(int x, int y, int width, int height, int value, int maxValue);

// Variables
int bassPosition = BASS_CENTER;      // Start in the middle position
int treblePosition = TREBLE_CENTER;  // Start in the middle position

// Encoder instances
ESP32Encoder bassEncoder;
ESP32Encoder trebleEncoder;

// Switch states
unsigned long lastDebounceTime = 0;
unsigned long lastDisplayUpdateTime = 0;
bool bypassActive = false;
bool boostActive = false;
bool displayNeedsUpdate = true;

int lastBassEncSwState = HIGH;
int lastTrebleEncSwState = HIGH;
int lastBypassFsState = HIGH;
int lastBoostFsState = HIGH;

// Encoder previous values
int32_t lastBassEncValue = 0;
int32_t lastTrebleEncValue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Active Baxandall Tone Control");
  
  // Initialize I2C for OLED display
  Wire.begin();
  
  // Initialize OLED display
  initDisplay();
  
  // Initialize X9C103 pins for Bass control
  pinMode(BASS_INC_PIN, OUTPUT);
  pinMode(BASS_UD_PIN, OUTPUT);
  pinMode(BASS_CS_PIN, OUTPUT);
  
  // Initialize X9C103 pins for Treble control
  pinMode(TREBLE_INC_PIN, OUTPUT);
  pinMode(TREBLE_UD_PIN, OUTPUT);
  pinMode(TREBLE_CS_PIN, OUTPUT);
  
  // Set initial states for potentiometers
  digitalWrite(BASS_INC_PIN, HIGH);
  digitalWrite(BASS_CS_PIN, HIGH);
  digitalWrite(TREBLE_INC_PIN, HIGH);
  digitalWrite(TREBLE_CS_PIN, HIGH);
  
  // Initialize encoder pins with pull-up resistors
  pinMode(BASS_ENC_SW, INPUT_PULLUP);
  pinMode(TREBLE_ENC_SW, INPUT_PULLUP);
  
  // Initialize footswitch pins with pull-up resistors
  pinMode(BYPASS_FS_PIN, INPUT_PULLUP);
  pinMode(BOOST_FS_PIN, INPUT_PULLUP);
  
  // Initialize transistor control pins
  pinMode(BASS_TRANSISTOR_PIN, OUTPUT);
  pinMode(TREBLE_TRANSISTOR_PIN, OUTPUT);
  pinMode(BOOST_TRANSISTOR_PIN, OUTPUT);
  
  // Initialize status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Set initial states for outputs
  digitalWrite(BASS_TRANSISTOR_PIN, HIGH);
  digitalWrite(TREBLE_TRANSISTOR_PIN, HIGH);
  digitalWrite(BOOST_TRANSISTOR_PIN, LOW);
  updateLED();
  
  // Initialize encoders
  ESP32Encoder::useInternalWeakPullResistors = UP;
  bassEncoder.attachFullQuad(BASS_ENC_A, BASS_ENC_B);
  trebleEncoder.attachFullQuad(TREBLE_ENC_A, TREBLE_ENC_B);
  bassEncoder.setCount(0);
  trebleEncoder.setCount(0);
  
  Serial.println("Initializing digital potentiometers...");
  
  // Initialize SPIFFS and load saved positions
  initSPIFFS();
  
  // Reset potentiometers to zero position first
  resetToZero(BASS_INC_PIN, BASS_UD_PIN, BASS_CS_PIN);
  resetToZero(TREBLE_INC_PIN, TREBLE_UD_PIN, TREBLE_CS_PIN);
  
  // Set to initial positions
  setPosition(BASS_INC_PIN, BASS_UD_PIN, BASS_CS_PIN, bassPosition, &bassPosition);
  setPosition(TREBLE_INC_PIN, TREBLE_UD_PIN, TREBLE_CS_PIN, treblePosition, &treblePosition);
  
  // Show initial display
  updateDisplay();
  
  Serial.println("Setup complete!");
}

void loop() {
  // Handle encoders
  handleBassEncoder();
  handleTrebleEncoder();
  
  // Handle footswitches and encoder buttons
  handleFootswitches();
  
  // Update display if needed
  unsigned long currentTime = millis();
  if ((displayNeedsUpdate && currentTime - lastDisplayUpdateTime > DISPLAY_UPDATE_INTERVAL) || 
      currentTime - lastDisplayUpdateTime > 1000) { // Force update every second
    updateDisplay();
    lastDisplayUpdateTime = currentTime;
    displayNeedsUpdate = false;
  }
  
  // Small delay to prevent CPU hogging
  delay(10);
}

// Initialize the OLED display
void initDisplay() {
  display.begin(I2C_ADDRESS, true); // Address 0x3C default
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Baxandall Tone Control");
  display.display();
  delay(2000); // Show splash screen
}

// Update the display with current settings
void updateDisplay() {
  display.clearDisplay();
  
  // Display title
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Baxandall Tone Control");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  // Status section
  display.setCursor(0, 14);
  display.print("Status: ");
  if (bypassActive) {
    display.println("BYPASS");
  } else {
    display.println("ACTIVE");
  }
  
  display.setCursor(0, 24);
  display.print("Boost: ");
  display.println(boostActive ? "ON" : "OFF");
  
  // Bass and Treble controls
  display.setCursor(0, 36);
  display.println("BASS:");
  drawBar(30, 36, 98, 8, bassPosition, POT_MAX_STEPS);
  
  display.setCursor(0, 50);
  display.println("TREBLE:");
  drawBar(30, 50, 98, 8, treblePosition, POT_MAX_STEPS);
  
  // Draw center markers
  int centerX = 30 + (98 * BASS_CENTER / POT_MAX_STEPS);
  display.drawLine(centerX, 36, centerX, 44, SH110X_WHITE);
  centerX = 30 + (98 * TREBLE_CENTER / POT_MAX_STEPS);
  display.drawLine(centerX, 50, centerX, 58, SH110X_WHITE);
  
  display.display();
}

// Draw a horizontal bar graph
void drawBar(int x, int y, int width, int height, int value, int maxValue) {
  // Draw the outline
  display.drawRect(x, y, width, height, SH110X_WHITE);
  
  // Calculate the fill width
  int fillWidth = map(value, 0, maxValue, 0, width - 2);
  
  // Fill the bar
  if (fillWidth > 0) {
    display.fillRect(x + 1, y + 1, fillWidth, height - 2, SH110X_WHITE);
  }
}

// Handle Bass Encoder rotation and button
void handleBassEncoder() {
  // Read encoder
  int32_t encValue = bassEncoder.getCount();
  
  // Check if encoder value changed
  if (encValue != lastBassEncValue) {
    int change = encValue - lastBassEncValue;
    lastBassEncValue = encValue;
    
    // Update position based on encoder movement
    bassPosition = constrain(bassPosition + change, 0, POT_MAX_STEPS);
    
    // Set the new position
    setPosition(BASS_INC_PIN, BASS_UD_PIN, BASS_CS_PIN, bassPosition, &bassPosition);
    
    Serial.print("Bass position: ");
    Serial.println(bassPosition);
    
    // Mark display for update
    displayNeedsUpdate = true;
  }
  
  // Read encoder switch
  int bassEncSwState = digitalRead(BASS_ENC_SW);
  unsigned long currentTime = millis();
  
  // Check for button press with debounce
  if (bassEncSwState != lastBassEncSwState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    if (bassEncSwState == LOW) {  // Button pressed
      // Reset bass to center position
      bassPosition = BASS_CENTER;
      setPosition(BASS_INC_PIN, BASS_UD_PIN, BASS_CS_PIN, bassPosition, &bassPosition);
      Serial.println("Bass reset to center position");
      
      // Mark display for update
      displayNeedsUpdate = true;
    }
    lastDebounceTime = currentTime;
  }
  lastBassEncSwState = bassEncSwState;
}

// Handle Treble Encoder rotation and button
void handleTrebleEncoder() {
  // Read encoder
  int32_t encValue = trebleEncoder.getCount();
  
  // Check if encoder value changed
  if (encValue != lastTrebleEncValue) {
    int change = encValue - lastTrebleEncValue;
    lastTrebleEncValue = encValue;
    
    // Update position based on encoder movement
    treblePosition = constrain(treblePosition + change, 0, POT_MAX_STEPS);
    
    // Set the new position
    setPosition(TREBLE_INC_PIN, TREBLE_UD_PIN, TREBLE_CS_PIN, treblePosition, &treblePosition);
    
    Serial.print("Treble position: ");
    Serial.println(treblePosition);
    
    // Mark display for update
    displayNeedsUpdate = true;
  }
  
  // Read encoder switch
  int trebleEncSwState = digitalRead(TREBLE_ENC_SW);
  unsigned long currentTime = millis();
  
  // Check for button press with debounce
  if (trebleEncSwState != lastTrebleEncSwState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    if (trebleEncSwState == LOW) {  // Button pressed
      // Reset treble to center position
      treblePosition = TREBLE_CENTER;
      setPosition(TREBLE_INC_PIN, TREBLE_UD_PIN, TREBLE_CS_PIN, treblePosition, &treblePosition);
      Serial.println("Treble reset to center position");
      
      // Mark display for update
      displayNeedsUpdate = true;
    }
    lastDebounceTime = currentTime;
  }
  lastTrebleEncSwState = trebleEncSwState;
}

// Handle footswitches
void handleFootswitches() {
  int bypassFsState = digitalRead(BYPASS_FS_PIN);
  int boostFsState = digitalRead(BOOST_FS_PIN);
  unsigned long currentTime = millis();
  
  // Check bypass footswitch with debounce
  if (bypassFsState != lastBypassFsState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    if (bypassFsState == LOW) {  // Footswitch pressed
      toggleBypass();
      
      // Mark display for update
      displayNeedsUpdate = true;
    }
    lastDebounceTime = currentTime;
  }
  lastBypassFsState = bypassFsState;
  
  // Check boost footswitch with debounce
  if (boostFsState != lastBoostFsState && currentTime - lastDebounceTime > DEBOUNCE_TIME) {
    if (boostFsState == LOW) {  // Footswitch pressed
      toggleBoost();
      
      // Mark display for update
      displayNeedsUpdate = true;
    }
    lastDebounceTime = currentTime;
  }
  lastBoostFsState = boostFsState;
}

// Toggle bypass state
void toggleBypass() {
  bypassActive = !bypassActive;
  updateLED();
  
  // Control transistors based on bypass state
  digitalWrite(BASS_TRANSISTOR_PIN, bypassActive ? LOW : HIGH);
  digitalWrite(TREBLE_TRANSISTOR_PIN, bypassActive ? LOW : HIGH);
  
  Serial.print("Bypass: ");
  Serial.println(bypassActive ? "ON" : "OFF");
  
  // Save position if switching to active mode
  if (!bypassActive) {
    savePositionsToSPIFFS();
  }
}

// Toggle boost state
void toggleBoost() {
  boostActive = !boostActive;
  
  // Control boost transistor
  digitalWrite(BOOST_TRANSISTOR_PIN, boostActive ? HIGH : LOW);
  
  Serial.print("Boost: ");
  Serial.println(boostActive ? "ON" : "OFF");
}

// Update LED status
void updateLED() {
  // LED is ON when effect is active (not bypassed)
  digitalWrite(STATUS_LED_PIN, bypassActive ? LOW : HIGH);
}

// Initialize SPIFFS file system
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }
  
  Serial.println("SPIFFS mounted successfully");
  
  if (loadPositionsFromSPIFFS()) {
    Serial.println("Positions loaded from SPIFFS");
  } else {
    Serial.println("Using default positions");
  }
}

// Set the wiper to a specific position (0-99)
void setPosition(uint8_t inc_pin, uint8_t ud_pin, uint8_t cs_pin, int position, int* currentPosition) {
  // Ensure position is within valid range
  position = constrain(position, 0, POT_MAX_STEPS);
  
  // Reset to zero and then increment to target position
  resetToZero(inc_pin, ud_pin, cs_pin);
  
  // Enable the chip
  digitalWrite(cs_pin, LOW);
  
  // Set direction to up (increment)
  digitalWrite(ud_pin, HIGH);
  
  // Increment to desired position
  for (int i = 0; i < position; i++) {
    digitalWrite(inc_pin, LOW);
    delayMicroseconds(1);
    digitalWrite(inc_pin, HIGH);
    delayMicroseconds(1);
  }
  
  // Disable the chip
  digitalWrite(cs_pin, HIGH);
  
  // Update current position
  *currentPosition = position;
}

// Reset the wiper position to zero
void resetToZero(uint8_t inc_pin, uint8_t ud_pin, uint8_t cs_pin) {
  // Enable the chip
  digitalWrite(cs_pin, LOW);
  
  // Set direction to down (decrement)
  digitalWrite(ud_pin, LOW);
  
  // Increment enough times to ensure we reach zero from any position
  for (int i = 0; i < POT_MAX_STEPS + 1; i++) {
    digitalWrite(inc_pin, LOW);
    delayMicroseconds(1);
    digitalWrite(inc_pin, HIGH);
    delayMicroseconds(1);
  }
  
  // Disable the chip
  digitalWrite(cs_pin, HIGH);
}

// Store current wiper position to non-volatile memory
void storePosition(uint8_t cs_pin) {
  // Enable the chip
  digitalWrite(cs_pin, LOW);
  
  // Store the current value - requires at least 20ms CS low per datasheet
  delay(20);
  
  // Disable the chip
  digitalWrite(cs_pin, HIGH);
}

// Save both potentiometer positions to SPIFFS
void savePositionsToSPIFFS() {
  StaticJsonDocument<128> doc;
  doc["bass"] = bassPosition;
  doc["treble"] = treblePosition;
  
  File file = SPIFFS.open(POSITION_FILE, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to file");
  }
  
  file.close();
  Serial.println("Positions saved to SPIFFS");
  
  // Also store to non-volatile memory
  storePosition(BASS_CS_PIN);
  storePosition(TREBLE_CS_PIN);
}

// Load potentiometer positions from SPIFFS
bool loadPositionsFromSPIFFS() {
  if (!SPIFFS.exists(POSITION_FILE)) {
    Serial.println("Position file not found");
    return false;
  }
  
  File file = SPIFFS.open(POSITION_FILE, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return false;
  }
  
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, file);
  
  file.close();
  
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return false;
  }
  
  bassPosition = doc["bass"];
  treblePosition = doc["treble"];
  Serial.print("Loaded bass position: ");
  Serial.println(bassPosition);
  Serial.print("Loaded treble position: ");
  Serial.println(treblePosition);
  return true;
} 