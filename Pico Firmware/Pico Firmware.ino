/*
  Raspberry Pi Pico + HLK-LD2420 Presence Sensor + 5V Relay + 0.91" OLED (128x32)
  Human detected in range -> Relay ON
  No human for 15 seconds  -> Relay OFF
  Push button cycles modes: DESK -> ROOM -> OUTDOOR

  Board package : "Raspberry Pi Pico/RP2040" by Earle F. Philhower III
  Libraries     : Adafruit SSD1306, Adafruit GFX

  WIRING
  ------------------------------------------------------------
  LD2420 3V3  -> Pico 3V3 (pin 36)      (sensor is 3.3V only!)
  LD2420 GND  -> Pico GND
  LD2420 OT1 (TX) -> Pico GP1 (UART0 RX, pin 2)
  LD2420 RX   -> Pico GP0 (UART0 TX, pin 1)

  Relay VCC   -> Pico VBUS (5V, pin 40)
  Relay GND   -> Pico GND
  Relay IN    -> Pico GP15

  OLED VCC    -> Pico 3V3
  OLED GND    -> Pico GND
  OLED SDA    -> Pico GP4 (pin 6)
  OLED SCL    -> Pico GP5 (pin 7)

  Button      -> one leg GP14, other leg GND (internal pull-up used)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Pins ----------
#define RELAY_PIN   15
#define BUTTON_PIN  14

// Most 5V relay modules are ACTIVE LOW. Set to false if yours is active HIGH.
#define RELAY_ACTIVE_LOW false

// ---------- OLED ----------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 32
#define OLED_ADDR     0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Timing ----------
const unsigned long OFF_DELAY_MS   = 15000;  // relay off after 15 s without detection
const unsigned long DEBOUNCE_MS    = 50;
const unsigned long DISPLAY_MS     = 200;
const unsigned long STALE_DATA_MS  = 1500;   // sensor data older than this = not detected

// ---------- Modes ----------
enum Mode { DESK_MODE = 0, ROOM_MODE, OUTDOOR_MODE, MODE_COUNT };

struct ModeCfg {
  const char* name;
  int maxRangeCm;   // detection only counts inside this distance
};

const ModeCfg modes[MODE_COUNT] = {
  { "DESK",    100 },   // up to 1 m
  { "ROOM",    400 },   // up to 4 m
  { "OUTDOOR", 700 }    // up to 7 m (LD2420 max ~8 m)
};

Mode currentMode = DESK_MODE;

// ---------- State ----------
bool relayOn = false;
bool sensorActive = false;       // sensor reports "ON"
int  lastRangeCm = 0;
unsigned long lastSeenMs = 0;    // last time a valid in-range detection occurred
unsigned long lastDataMs = 0;    // last time any sensor data arrived
unsigned long lastDisplayMs = 0;

// Button
bool lastBtnReading = HIGH;
bool btnState = HIGH;
unsigned long lastDebounceMs = 0;

String rxLine = "";

// ---------- Relay ----------
void setRelay(bool on) {
  relayOn = on;
  if (on) {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);   // pull LOW = relay ON
  } else {
    pinMode(RELAY_PIN, INPUT);      // release pin = relay OFF
  }
}

// ---------- Sensor parsing ----------
// LD2420 default output (115200 baud) sends lines like:
//   "ON"      -> presence
//   "Range 85" -> distance in cm
//   "OFF"     -> no presence
void processLine(String line) {
  line.trim();
  if (line.length() == 0) return;
  lastDataMs = millis();

  if (line.startsWith("ON")) {
    sensorActive = true;
  } else if (line.startsWith("OFF")) {
    sensorActive = false;
    lastRangeCm = 0;
  } else if (line.startsWith("Range")) {
    lastRangeCm = line.substring(5).toInt();
    sensorActive = true;
    if (lastRangeCm > 0 && lastRangeCm <= modes[currentMode].maxRangeCm) {
      lastSeenMs = millis();
    }
  }
}

void readSensor() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      processLine(rxLine);
      rxLine = "";
    } else if (c != '\r') {
      if (rxLine.length() < 40) rxLine += c;
      else rxLine = "";               // overflow guard (binary data)
    }
  }
}

// ---------- Button ----------
void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastBtnReading) lastDebounceMs = millis();

  if ((millis() - lastDebounceMs) > DEBOUNCE_MS && reading != btnState) {
    btnState = reading;
    if (btnState == LOW) {            // pressed
      currentMode = (Mode)((currentMode + 1) % MODE_COUNT);
      lastSeenMs = 0;                 // re-evaluate with the new range
      lastRangeCm = 0;
      showModeSplash();
    }
  }
  lastBtnReading = reading;
}

// ---------- Display ----------
void showModeSplash() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(modes[currentMode].name);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Range: <= ");
  display.print(modes[currentMode].maxRangeCm);
  display.print(" cm");
  display.display();
  delay(800);
}

void updateDisplay(bool detected, unsigned long remainingMs) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Line 1: mode + relay
  display.setCursor(0, 0);
  display.print("Mode:");
  display.print(modes[currentMode].name);
  display.setCursor(86, 0);
  display.print("RLY:");
  display.print(relayOn ? "ON" : "OFF");

  // Line 2: presence + distance
  display.setCursor(0, 11);
  display.print(detected ? "HUMAN DETECTED" : "No Human");

  // Line 3: distance / countdown
  display.setCursor(0, 22);
  if (lastRangeCm > 0) {
    display.print("Dist:");
    display.print(lastRangeCm);
    display.print("cm ");
  }
  if (relayOn && !detected) {
    display.print("OFF in ");
    display.print(remainingMs / 1000 + 1);
    display.print("s");
  }
  display.display();
}

// ---------- Setup / Loop ----------
void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);

  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(115200);          // LD2420 default baud

  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found!");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Presence Relay");
  display.println("LD2420 starting...");
  display.display();
  delay(1500);
  showModeSplash();
}

void loop() {
  readSensor();
  handleButton();

  unsigned long now = millis();

  // Detected = in-range reading within the last 15 s window,
  // and the sensor data stream is alive.
  bool freshInRange = (lastSeenMs != 0) && (now - lastSeenMs < STALE_DATA_MS);
  bool detected = freshInRange && sensorActive;

  if (detected) {
    if (!relayOn) setRelay(true);
  } else if (relayOn) {
    // Keep ON until 15 s after the last valid detection
    if (lastSeenMs == 0 || (now - lastSeenMs) >= OFF_DELAY_MS) {
      setRelay(false);
    }
  }

  if (now - lastDisplayMs >= DISPLAY_MS) {
    lastDisplayMs = now;
    unsigned long remaining = 0;
    if (relayOn && lastSeenMs != 0 && (now - lastSeenMs) < OFF_DELAY_MS)
      remaining = OFF_DELAY_MS - (now - lastSeenMs);
    updateDisplay(detected, remaining);
  }
}
