#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ThingSpeak.h>


//Fields in ThingSpeak
//Temperature (°C)
//Humidity (%)
//Light Level (%)
//Occupants
//Alert Status

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// WiFi and ThingSpeak Configuration
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const unsigned long myChannelNumber = 2902740; // Replace with your channel number
const char* myWriteAPIKey = "F15RE0VT3203FE3U";   // Replace with your API key

WiFiClient client;

// Pin Definitions
#define PIR_PIN 13
#define DHT_PIN 14
#define LDR_PIN 34
#define LED_PIN 15

// Test Control
unsigned long lastUpdate = 0;
const long TEST_DURATION = 10000; // 10 seconds per test case
int testCase = 0;
const int MAX_TEST_CASES = 5;
unsigned long testStartTime = 0;
bool wifiConnected = false;
bool initialTestRunComplete = false;
unsigned long lastThingSpeakUpdate = 0;
const long THINGSPEAK_INTERVAL = 15000; // 15 seconds between ThingSpeak updates

// Environment Simulation
struct Environment {
  float temp;
  float hum;
  int light;
  int occupants;
} env = {22.0, 45.0, 2000, 0};

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize I2C with explicit pins
  Wire.begin(4, 5); // SDA=GPIO4, SCL=GPIO5

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED initialization failed"));
    while (1);
  }

  // Show initial display
  display.display();
  delay(1000);

  // Serial Monitor Header
  Serial.println(F("\n┌──────────────────────────────────┐"));
  Serial.println(F("│ Smart Room Environment and     │"));
  Serial.println(F("│ Occupancy Monitor v2.0        │"));
  Serial.println(F("│ Now with WiFi & ThingSpeak    │"));
  Serial.println(F("└──────────────────────────────────┘"));
  printTestCases();

  displayIntroScreen();
  delay(2000);
  testCase = 1;
  testStartTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // First run through all test cases without WiFi
  if (!initialTestRunComplete) {
    runTestCases(currentMillis);
  }
  else {
    // After initial test run, connect to WiFi and start sending to ThingSpeak
    if (!wifiConnected) {
      connectToWiFi();
    }

    if (wifiConnected) {
      // Continue cycling through test cases
      if (currentMillis - testStartTime >= TEST_DURATION) {
        testCase = (testCase % MAX_TEST_CASES) + 1;
        testStartTime = currentMillis;
        resetAlert();
        generateTestData();
        printTestCaseHeader();
      }

      // Update display and serial output every second
      if (currentMillis - lastUpdate >= 1000) {
        lastUpdate = currentMillis;
        updateDisplay();
        printSensorData();
      }

      // Send data to ThingSpeak periodically
      if (currentMillis - lastThingSpeakUpdate >= THINGSPEAK_INTERVAL) {
        lastThingSpeakUpdate = currentMillis;
        sendToThingSpeak();
      }

      checkAlerts();
    }
  }
}

void runTestCases(unsigned long currentMillis) {
  // Rotate test cases every TEST_DURATION
  if (currentMillis - testStartTime >= TEST_DURATION) {
    testCase++;
    testStartTime = currentMillis;
    resetAlert();
    generateTestData();
    printTestCaseHeader();

    if (testCase > MAX_TEST_CASES) {
      initialTestRunComplete = true;
      testCase = 1;
      Serial.println(F("\n┌──────────────────────────────────┐"));
      Serial.println(F("│ Initial Test Run Complete       │"));
      Serial.println(F("│ Connecting to WiFi...          │"));
      Serial.println(F("└──────────────────────────────────┘"));
      return;
    }
  }

  // Update every second
  if (currentMillis - lastUpdate >= 1000) {
    lastUpdate = currentMillis;
    updateDisplay();
    printSensorData();
  }

  checkAlerts();
}

void connectToWiFi() {
  Serial.print(F("Connecting to WiFi"));
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Connecting to WiFi"));
  display.display();

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(F("."));
    display.print(F("."));
    display.display();
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    ThingSpeak.begin(client);
    Serial.println(F("\nWiFi connected!"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("WiFi Connected!"));
    display.setCursor(0, 10);
    display.print(F("IP: "));
    display.println(WiFi.localIP());
    display.setCursor(0, 20);
    display.println(F("ThingSpeak Ready"));
    display.display();
    delay(2000);
  } else {
    Serial.println(F("\nFailed to connect to WiFi"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("WiFi Connection"));
    display.setCursor(0, 10);
    display.println(F("Failed!"));
    display.display();
    delay(2000);
  }
}

void sendToThingSpeak() {
  if (wifiConnected) {
    // Set the fields with the values
    ThingSpeak.setField(1, env.temp);
    ThingSpeak.setField(2, env.hum);
    ThingSpeak.setField(3, map(env.light, 0, 4095, 0, 100));
    ThingSpeak.setField(4, env.occupants);
    ThingSpeak.setField(5, digitalRead(LED_PIN));

    // Write to ThingSpeak
    int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (response == 200) {
      Serial.println(F("Data sent to ThingSpeak successfully!"));
      displayStatusMessage("Data sent to TS!");
    } else {
      Serial.print(F("Problem sending to ThingSpeak. HTTP error code "));
      Serial.println(response);
      displayStatusMessage("TS Send Failed");
    }
  }
}

void generateTestData() {
  switch (testCase) {
    case 1: // Normal conditions
      env = {22.0 + random(-5, 5) / 10.0,
             45.0 + random(-10, 10),
             2500 + random(-500, 500),
             0
          };
      break;
    case 2: // High temp alert
      env = {32.0 + random(-5, 5) / 10.0,
             40.0,
             3000,
             1
          };
      break;
    case 3: // High humidity alert
      env = {24.0,
             75.0 + random(-5, 5),
             1500,
             2
          };
      break;
    case 4: // Low light + occupancy
      env = {21.0,
             50.0,
             800,
             1
          };
      break;
    case 5: // Reset
      env = {20.0, 40.0, 2000, 0};
      break;
  }
}

void checkAlerts() {
  bool alertActive = false;
  // Case 2: High temp alert
  if (testCase == 2 && env.temp > 30.0) alertActive = true;
  // Case 3: High humidity alert
  if (testCase == 3 && env.hum > 70.0) alertActive = true;
  // Case 4: Low light + occupancy
  if (testCase == 4 && env.light < 1500 && env.occupants > 0) alertActive = true;

  digitalWrite(LED_PIN, alertActive ? HIGH : LOW);
}

void updateDisplay() {
  display.clearDisplay();

  // Header Bar
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print(wifiConnected ? F("WiFi+TS CONNECTED") : F("ENV & OCCUPANCY"));
  display.setTextColor(SSD1306_WHITE);

  // Data Grid
  display.drawLine(0, 13, 128, 13, SSD1306_WHITE);

  // Environment Data
  display.setCursor(0, 15);
  display.print(F("TEMP: "));
  display.print(env.temp, 1);
  display.print(F("\xDF" "C")); // Degree symbol

  display.setCursor(70, 15);
  display.print(F("HUM: "));
  display.print(env.hum, 1);
  display.print(F("%"));

  // Occupancy Data
  display.setCursor(0, 27);
  display.print(F("LIGHT: "));
  display.print(map(env.light, 0, 4095, 0, 100));
  display.print(F("%"));

  display.setCursor(70, 27);
  display.print(F("OCC: "));
  display.print(env.occupants);

  // Status Panel
  display.fillRect(0, 40, 128, 24, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 42);

  if (testCase == 2 && env.temp > 30.0) {
    display.print(F("HIGH TEMP ALERT!"));
  }
  else if (testCase == 3 && env.hum > 70.0) {
    display.print(F("HIGH HUMIDITY!"));
  }
  else if (testCase == 4 && env.light < 1500 && env.occupants > 0) {
    display.print(F("AUTO LIGHTS ON"));
  }
  else if (wifiConnected) {
    display.print(F("TS UPLOAD ACTIVE"));
  }
  else {
    display.print(F("NORMAL OPERATION"));
  }

  // Footer
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 55);
  display.print(F("CASE "));
  display.print(testCase);
  display.print(F("/5 "));
  display.print((TEST_DURATION - (millis() - testStartTime)) / 1000);
  display.print(F("s"));

  display.display();
}

void displayStatusMessage(const char* message) {
  // Quick status message on OLED
  display.fillRect(0, 40, 128, 24, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 42);
  display.print(message);
  display.display();
  delay(1000);
  // Force a display update to return to normal
  lastUpdate = 0;
}

void printSensorData() {
  Serial.println(F("\n┌──────────────────────────────────┐"));
  Serial.print(F("│ TIMESTAMP: "));
  Serial.print(millis() / 1000);
  Serial.println(F("s                  │"));
  Serial.println(F("├──────────────────────────────────┤"));
  Serial.print(F("│ TEMP: "));
  Serial.print(env.temp, 1);
  Serial.print(F("\xDF" "C  HUM: "));
  Serial.print(env.hum, 1);
  Serial.println(F("%       │"));
  Serial.print(F("│ LIGHT: "));
  Serial.print(map(env.light, 0, 4095, 0, 100));
  Serial.print(F("%  OCCUPANTS: "));
  Serial.print(env.occupants);
  Serial.println(F("    │"));
  Serial.println(F("├──────────────────────────────────┤"));

  if (testCase == 2 && env.temp > 30.0) {
    Serial.println(F("│ ! ALERT: HIGH TEMPERATURE !       │"));
  }
  else if (testCase == 3 && env.hum > 70.0) {
    Serial.println(F("│ ! ALERT: HIGH HUMIDITY !         │"));
  }
  else if (testCase == 4 && env.light < 1500 && env.occupants > 0) {
    Serial.println(F("│ AUTO: LIGHTS ACTIVATED           │"));
  }
  else if (wifiConnected) {
    Serial.println(F("│ STATUS: THINGSPEAK ACTIVE       │"));
  }
  else {
    Serial.println(F("│ STATUS: NORMAL OPERATION        │"));
  }

  Serial.print(F("│ LED: "));
  Serial.print(digitalRead(LED_PIN) ? F("ACTIVE (") : F("INACTIVE ("));
  Serial.print(testCase);
  Serial.println(F(")            │"));
  Serial.print(F("│ WiFi: "));
  Serial.print(wifiConnected ? F("CONNECTED") : F("DISCONNECTED"));
  Serial.println(F("         │"));
  Serial.println(F("└──────────────────────────────────┘"));
}

void displayIntroScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Smart Room Env &"));
  display.setCursor(0, 10);
  display.println(F("Occupancy Monitor"));
  display.drawLine(0, 20, 128, 20, SSD1306_WHITE);
  display.setCursor(0, 25);
  display.println(F("WiFi+ThingSpeak v2"));
  display.setCursor(0, 55);
  display.println(F("v2.0 | Wokwi"));
  display.display();
}

void printTestCaseHeader() {
  Serial.println(F("\n──────────────────────────────────"));
  Serial.print(F(" TEST CASE "));
  Serial.print(testCase);
  Serial.println(F(" ACTIVATED "));
  Serial.println(F("──────────────────────────────────"));
}

void resetAlert() {
  digitalWrite(LED_PIN, LOW);
}

void printTestCases() {
  Serial.println(F("\nTEST SCENARIOS:"));
  Serial.println(F("1. Normal Conditions"));
  Serial.println(F("2. High Temperature (>30°C)"));
  Serial.println(F("3. High Humidity (>70%)"));
  Serial.println(F("4. Low Light + Occupancy"));
  Serial.println(F("5. System Reset"));
  Serial.println(F("──────────────────────────────────"));
  Serial.println(F("After initial test run,"));
  Serial.println(F("system will connect to WiFi"));
  Serial.println(F("and upload to ThingSpeak"));
  Serial.println(F("──────────────────────────────────"));
}
