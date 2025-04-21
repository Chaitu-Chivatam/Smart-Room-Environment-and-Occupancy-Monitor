#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <HTTPClient.h>

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// WiFi and ThingSpeak Configuration
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const unsigned long myChannelNumber = 2902740;
const char* myWriteAPIKey = "F15RE0VT3203FE3U";

// Telegram Configuration
const char* TELEGRAM_BOT_TOKEN = "7989947430:AAECQcv-Ukjh87pY_hBqS2nz5a12_u6-5_o";
const char* TELEGRAM_CHAT_ID = "7117019915";
const char* TELEGRAM_URL = "https://api.telegram.org/bot";

WiFiClient client;

// Pin Definitions
#define PIR_PIN 13
#define DHT_PIN 14
#define LDR_PIN 34
#define LED_PIN 15

// Test Control
unsigned long lastUpdate = 0;
const long TEST_DURATION = 10000;
int testCase = 0;
const int MAX_TEST_CASES = 5;
unsigned long testStartTime = 0;
bool wifiConnected = false;
bool initialTestRunComplete = false;
unsigned long lastThingSpeakUpdate = 0;
const long THINGSPEAK_INTERVAL = 15000;
unsigned long lastTelegramAlert = 0;
const long TELEGRAM_ALERT_INTERVAL = 30000;

// Environment Simulation
struct Environment {
  float temp;
  float hum;
  int light;
  int occupants;
} env = {22.0, 45.0, 2000, 0};

// Function prototypes for functions used before their definition
void printTestCases();
void displayIntroScreen();
void runTestCases(unsigned long currentMillis);
void connectToWiFi();
void resetAlert();
void generateTestData();
void printTestCaseHeader();
void updateDisplay();
void printSensorData();
void sendToThingSpeak();
void checkAlerts();
void sendTelegramAlert(String condition);
String urlEncode(String str);
void displayStatusMessage(const char* message);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize I2C with explicit pins
  Wire.begin(4, 5);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED initialization failed"));
    while (1);
  }

  display.display();
  delay(1000);

  Serial.println(F("\n┌──────────────────────────────────┐"));
  Serial.println(F("│ Smart Room Environment Monitor │"));
  Serial.println(F("│ With Telegram Alerts v2.1     │"));
  Serial.println(F("└──────────────────────────────────┘"));
  printTestCases();

  displayIntroScreen();
  delay(2000);
  testCase = 1;
  testStartTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  if (!initialTestRunComplete) {
    runTestCases(currentMillis);
  }
  else {
    if (!wifiConnected) {
      connectToWiFi();
    }

    if (wifiConnected) {
      if (currentMillis - testStartTime >= TEST_DURATION) {
        testCase = (testCase % MAX_TEST_CASES) + 1;
        testStartTime = currentMillis;
        resetAlert();
        generateTestData();
        printTestCaseHeader();
      }

      if (currentMillis - lastUpdate >= 1000) {
        lastUpdate = currentMillis;
        updateDisplay();
        printSensorData();
      }

      if (currentMillis - lastThingSpeakUpdate >= THINGSPEAK_INTERVAL) {
        lastThingSpeakUpdate = currentMillis;
        sendToThingSpeak();
      }

      checkAlerts();
    }
  }
}

void printTestCases() {
  Serial.println(F("\nTEST SCENARIOS:"));
  Serial.println(F("1. Normal Conditions"));
  Serial.println(F("2. High Temperature (>30°C)"));
  Serial.println(F("3. High Humidity (>70%)"));
  Serial.println(F("4. Low Light + Occupancy"));
  Serial.println(F("5. System Reset"));
  Serial.println(F("──────────────────────────────────"));
}

void displayIntroScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Smart Room Monitor"));
  display.setCursor(0, 10);
  display.println(F("With Telegram Alerts"));
  display.drawLine(0, 20, 128, 20, SSD1306_WHITE);
  display.setCursor(0, 25);
  display.println(F("v2.1 | Wokwi"));
  display.display();
}

void runTestCases(unsigned long currentMillis) {
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

void resetAlert() {
  digitalWrite(LED_PIN, LOW);
}

void generateTestData() {
  switch (testCase) {
    case 1:
      env = {22.0 + random(-5, 5) / 10.0, 45.0 + random(-10, 10), 2500 + random(-500, 500), 0};
      break;
    case 2:
      env = {32.0 + random(-5, 5) / 10.0, 40.0, 3000, 1};
      break;
    case 3:
      env = {24.0, 75.0 + random(-5, 5), 1500, 2};
      break;
    case 4:
      env = {21.0, 50.0, 800, 1};
      break;
    case 5:
      env = {20.0, 40.0, 2000, 0};
      break;
  }
}

void printTestCaseHeader() {
  Serial.println(F("\n──────────────────────────────────"));
  Serial.print(F(" TEST CASE "));
  Serial.print(testCase);
  Serial.println(F(" ACTIVATED "));
  Serial.println(F("──────────────────────────────────"));
}

void updateDisplay() {
  display.clearDisplay();
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print(wifiConnected ? F("WiFi+TS CONNECTED") : F("ENV & OCCUPANCY"));
  display.setTextColor(SSD1306_WHITE);
  display.drawLine(0, 13, 128, 13, SSD1306_WHITE);

  display.setCursor(0, 15);
  display.print(F("TEMP: "));
  display.print(env.temp, 1);
  display.print(F("\xDF" "C"));

  display.setCursor(70, 15);
  display.print(F("HUM: "));
  display.print(env.hum, 1);
  display.print(F("%"));

  display.setCursor(0, 27);
  display.print(F("LIGHT: "));
  display.print(map(env.light, 0, 4095, 0, 100));
  display.print(F("%"));

  display.setCursor(70, 27);
  display.print(F("OCC: "));
  display.print(env.occupants);

  display.fillRect(0, 40, 128, 24, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 42);

  if (testCase == 2 && env.temp > 30.0) {
    display.print(F("HIGH TEMP ALERT!"));
    if (wifiConnected) sendTelegramAlert("HIGH_TEMP");
  }
  else if (testCase == 3 && env.hum > 70.0) {
    display.print(F("HIGH HUMIDITY!"));
    if (wifiConnected) sendTelegramAlert("HIGH_HUMIDITY");
  }
  else if (testCase == 4 && env.light < 1500 && env.occupants > 0) {
    display.print(F("AUTO LIGHTS ON"));
    if (wifiConnected) sendTelegramAlert("LOW_LIGHT_OCCUPANCY");
  }
  else if (wifiConnected) {
    display.print(F("TS UPLOAD ACTIVE"));
  }
  else {
    display.print(F("NORMAL OPERATION"));
  }

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 55);
  display.print(F("CASE "));
  display.print(testCase);
  display.print(F("/5 "));
  display.print((TEST_DURATION - (millis() - testStartTime)) / 1000);
  display.print(F("s"));

  display.display();
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

void sendToThingSpeak() {
  if (wifiConnected) {
    ThingSpeak.setField(1, env.temp);
    ThingSpeak.setField(2, env.hum);
    ThingSpeak.setField(3, map(env.light, 0, 4095, 0, 100));
    ThingSpeak.setField(4, env.occupants);
    ThingSpeak.setField(5, digitalRead(LED_PIN));

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

void checkAlerts() {
  static bool lastAlertState = false;
  bool alertActive = false;
  String alertCondition = "";
  
  if (testCase == 2 && env.temp > 30.0) {
    alertActive = true;
    alertCondition = "HIGH_TEMP";
  }
  else if (testCase == 3 && env.hum > 70.0) {
    alertActive = true;
    alertCondition = "HIGH_HUMIDITY";
  }
  else if (testCase == 4 && env.light < 1500 && env.occupants > 0) {
    alertActive = true;
    alertCondition = "LOW_LIGHT_OCCUPANCY";
  }

  digitalWrite(LED_PIN, alertActive ? HIGH : LOW);

  if (alertActive && !lastAlertState) {
    if (wifiConnected) sendTelegramAlert(alertCondition);
  }
  
  lastAlertState = alertActive;
}

void sendTelegramAlert(String condition) {
  unsigned long currentMillis = millis();
  if (currentMillis - lastTelegramAlert < TELEGRAM_ALERT_INTERVAL) {
    return;
  }
  
  String message;
  
  if (condition == "HIGH_TEMP") {
    message = "⚠️ HIGH TEMPERATURE ALERT!\n🌡️ Temperature: " + String(env.temp, 1) + "°C\n💧 Humidity: " + String(env.hum, 1) + "%\n💡 Light: " + String(map(env.light, 0, 4095, 0, 100)) + "%\n🚶 Occupants: " + String(env.occupants);
  } 
  else if (condition == "HIGH_HUMIDITY") {
    message = "⚠️ HIGH HUMIDITY ALERT!\n🌡️ Temperature: " + String(env.temp, 1) + "°C\n💧 Humidity: " + String(env.hum, 1) + "%\n💡 Light: " + String(map(env.light, 0, 4095, 0, 100)) + "%\n🚶 Occupants: " + String(env.occupants);
  }
  else if (condition == "LOW_LIGHT_OCCUPANCY") {
    message = "⚠️ LOW LIGHT WITH OCCUPANCY!\n🌡️ Temperature: " + String(env.temp, 1) + "°C\n💧 Humidity: " + String(env.hum, 1) + "%\n💡 Light: " + String(map(env.light, 0, 4095, 0, 100)) + "%\n🚶 Occupants: " + String(env.occupants) + "\n💡 Lights automatically turned on";
  }
  else {
    return;
  }

  HTTPClient http;
  String url = String(TELEGRAM_URL) + TELEGRAM_BOT_TOKEN + "/sendMessage?chat_id=" + TELEGRAM_CHAT_ID + "&text=" + urlEncode(message);
  
  Serial.print("Sending Telegram alert: ");
  Serial.println(url);
  
  http.begin(url);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    Serial.println("Telegram alert sent successfully");
    lastTelegramAlert = currentMillis;
  } else {
    Serial.print("Telegram alert failed, error: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}

String urlEncode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  
  for (unsigned int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  
  return encodedString;
}

void displayStatusMessage(const char* message) {
  display.fillRect(0, 40, 128, 24, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 42);
  display.print(message);
  display.display();
  delay(1000);
  lastUpdate = 0;
}