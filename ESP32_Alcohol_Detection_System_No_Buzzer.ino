#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ========== WIFI ACCESS POINT ==========
const char* ssid = "MQ3-Monitor";
const char* password = "MQ3test123";

WebServer server(80);

// ========== OLED ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ========== PIN CONNECTIONS ==========
const int mq3Pin = 34;
const int greenLED = 25;
const int redLED = 26;
const int buttonPin = 18;
const int motorIN1 = 19;
const int motorIN2 = 23;
const int motorENA = 5;

// ========== SENSOR THRESHOLDS ==========
const int threshold = 500;
const int releaseThreshold = 450;

// ========== VARIABLES ==========
int sensorValue = 0;
bool alcoholDetected = false;
bool motorRunning = false;
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 40;

// ========== MOTOR CONTROL ==========
void stopMotor() {
  digitalWrite(motorENA, LOW);
  digitalWrite(motorIN1, LOW);
  digitalWrite(motorIN2, LOW);
  motorRunning = false;
}

void startMotor() {
  // Educational prototype interlock: motor runs only when status is clear.
  if (alcoholDetected) {
    stopMotor();
    return;
  }

  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);
  digitalWrite(motorENA, HIGH);
  motorRunning = true;
}

// ========== OLED DISPLAY ==========
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("MQ-3 ALCOHOL MONITOR");
  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

  display.setCursor(0, 19);
  display.print("Sensor: ");
  display.println(sensorValue);

  display.setCursor(0, 32);
  display.print("Status: ");
  display.println(alcoholDetected ? "DETECTED" : "CLEAR");

  display.setCursor(0, 45);
  display.print("Motor: ");
  display.println(motorRunning ? "ON" : "OFF");

  display.display();
}

// ========== WEB DASHBOARD ==========
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MQ-3 Monitor</title>
<style>
body {
  font-family: Arial, sans-serif;
  text-align: center;
  background: #f1f4f8;
  margin: 0;
  padding: 20px;
}
.card {
  background: white;
  max-width: 380px;
  margin: 20px auto;
  padding: 22px;
  border-radius: 15px;
  box-shadow: 0 3px 12px #cccccc;
}
h2 { color: #263746; }
.value {
  font-size: 32px;
  font-weight: bold;
}
.status {
  font-size: 22px;
  font-weight: bold;
}
</style>
</head>
<body>
<div class="card">
  <h2>Alcohol Detection</h2>
  <p>MQ-3 Sensor Reading</p>
  <div class="value" id="sensor">--</div>
  <p>Detection Status</p>
  <div class="status" id="status">Loading...</div>
  <p>Motor Status</p>
  <div class="status" id="motor">--</div>
  <p>Motor control is available using the physical push button.</p>
</div>
<script>
async function updateData() {
  try {
    const response = await fetch('/status');
    const data = await response.json();

    document.getElementById('sensor').textContent = data.sensor;
    document.getElementById('status').textContent =
      data.alcohol ? 'DETECTED' : 'CLEAR';
    document.getElementById('status').style.color =
      data.alcohol ? 'red' : 'green';
    document.getElementById('motor').textContent =
      data.motor ? 'ON' : 'OFF';
    document.getElementById('motor').style.color =
      data.motor ? 'green' : 'gray';
  } catch (error) {
    document.getElementById('status').textContent = 'Connection lost';
  }
}
updateData();
setInterval(updateData, 1000);
</script>
</body>
</html>
)rawliteral";

// ========== WEB STATUS ==========
void handleStatus() {
  String json = "{";
  json += "\"sensor\":" + String(sensorValue) + ",";
  json += "\"alcohol\":" + String(alcoholDetected ? "true" : "false") + ",";
  json += "\"motor\":" + String(motorRunning ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);
  pinMode(motorENA, OUTPUT);

  stopMotor();

  analogReadResolution(12);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("MQ-3 Monitor");
    display.println("Starting...");
    display.display();
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("Wi-Fi Started");
  Serial.print("Connect to: ");
  Serial.println(ssid);
  Serial.print("Dashboard IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", []() {
    server.send_P(200, "text/html", webpage);
  });
  server.on("/status", handleStatus);
  server.begin();

  Serial.println("System Ready");
}

// ========== MAIN LOOP ==========
void loop() {
  server.handleClient();

  sensorValue = analogRead(mq3Pin);

  // Hysteresis helps reduce rapid status changes near the threshold.
  if (!alcoholDetected && sensorValue >= threshold) {
    alcoholDetected = true;
  } else if (alcoholDetected && sensorValue <= releaseThreshold) {
    alcoholDetected = false;
  }

  digitalWrite(greenLED, alcoholDetected ? LOW : HIGH);
  digitalWrite(redLED, alcoholDetected ? HIGH : LOW);

  if (alcoholDetected) {
    stopMotor();
  }

  // Debounced push button (INPUT_PULLUP; pressed = LOW).
  bool reading = digitalRead(buttonPin);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      if (stableButtonState == LOW) {
        if (!alcoholDetected) {
          if (motorRunning) {
            stopMotor();
          } else {
            startMotor();
          }
        } else {
          stopMotor();
        }
      }
    }
  }
  lastButtonReading = reading;

  static unsigned long lastOLEDUpdate = 0;
  if (millis() - lastOLEDUpdate >= 250) {
    lastOLEDUpdate = millis();
    updateOLED();
  }

  static unsigned long lastSerialUpdate = 0;
  if (millis() - lastSerialUpdate >= 500) {
    lastSerialUpdate = millis();
    Serial.print("Sensor: ");
    Serial.print(sensorValue);
    Serial.print(" | Detection: ");
    Serial.print(alcoholDetected ? "DETECTED" : "CLEAR");
    Serial.print(" | Motor: ");
    Serial.println(motorRunning ? "ON" : "OFF");
  }

  delay(10);
}
