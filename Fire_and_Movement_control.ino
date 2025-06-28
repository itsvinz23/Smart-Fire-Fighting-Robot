// Mega 2560

#include <SoftwareSerial.h>

// Sensor Pins
#define TRIG_PIN 8
#define ECHO_PIN 9
#define DO_PIN_FIRE 10
#define PUMP_PIN 11
#define ENA 6
#define ENB 7
#define IN1 3
#define IN2 4
#define IN3 5
#define IN4 6
#define IR_R 12
#define IR_F 13
#define IR_L 14

// Software Serial for ESP communication (RX, TX)
SoftwareSerial espSerial(18, 19); // Mega RX1 (18), TX1 (19)

void setup() {
  // Initialize pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(DO_PIN_FIRE, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(IR_R, INPUT);
  pinMode(IR_F, INPUT);
  pinMode(IR_L, INPUT);
  
  // Motor control pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Serial communication
  Serial.begin(9600);     // For debugging
  espSerial.begin(115200); // Communication with ESP8266

  Serial.println("Arduino Mega Initialized");
}

void loop() {
  // 1. Read Ultrasonic Distance
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;

  // 2. Read Fire Sensor
  bool fireDetected = digitalRead(DO_PIN_FIRE) == HIGH;

  // 3. Read IR Sensors
  bool irRight = digitalRead(IR_R) == LOW;  // LOW means obstacle detected
  bool irFront = digitalRead(IR_F) == LOW;
  bool irLeft = digitalRead(IR_L) == LOW;

  // 4. Create JSON payload
  String payload = "{";
  payload += "\"distance\":" + String(distance) + ",";
  payload += "\"fire\":" + String(fireDetected ? "true" : "false") + ",";
  payload += "\"ir_right\":" + String(irRight ? "true" : "false") + ",";
  payload += "\"ir_front\":" + String(irFront ? "true" : "false") + ",";
  payload += "\"ir_left\":" + String(irLeft ? "true" : "false");
  payload += "}";

  // 5. Send to ESP8266
  espSerial.println(payload);
  
  // Debug output
  Serial.println("Sent to ESP: " + payload);
  
  delay(500); // Adjust based on your needs
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NodeMCU ESP8266

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// WiFi Credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Firebase Configuration
#define FIREBASE_HOST "YOUR_PROJECT.firebaseio.com"
#define FIREBASE_AUTH "YOUR_DATABASE_SECRET"

// Software Serial for Arduino Communication
#include <SoftwareSerial.h>
SoftwareSerial arduinoSerial(D2, D3); // RX, TX (D2=GPIO4, D3=GPIO0)

// Sensor Data Variables
int distance = 0;
bool fireDetected = false;
bool irRight = false;
bool irFront = false;
bool irLeft = false;

void setup() {
  // Initialize serial communications
  Serial.begin(9600);
  arduinoSerial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Firebase Initialized");

  // Set initial values
  Firebase.setInt("sensors/distance", 0);
  Firebase.setBool("sensors/fire", false);
  Firebase.setBool("sensors/ir_right", false);
  Firebase.setBool("sensors/ir_front", false);
  Firebase.setBool("sensors/ir_left", false);
}

void loop() {
  // Check for incoming data from Arduino
  if (arduinoSerial.available()) {
    String payload = arduinoSerial.readStringUntil('\n');
    Serial.println("Received from Arduino: " + payload);

    // Parse JSON (simple parsing for this example)
    parsePayload(payload);

    // Update Firebase
    updateFirebase();
  }

  // Handle Firebase errors
  if (Firebase.failed()) {
    Serial.print("Firebase error: ");
    Serial.println(Firebase.error());
    delay(1000);
    // Try to reconnect
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  }

  delay(100);
}

void parsePayload(String payload) {
  // Simple JSON parsing (for more complex JSON, use ArduinoJson library)
  distance = getValue(payload, "\"distance\":", ",").toInt();
  fireDetected = getValue(payload, "\"fire\":", ",") == "true";
  irRight = getValue(payload, "\"ir_right\":", ",") == "true";
  irFront = getValue(payload, "\"ir_front\":", ",") == "true";
  irLeft = getValue(payload, "\"ir_left\":", "}") == "true";
}

String getValue(String data, String key, String delimiter) {
  int startIndex = data.indexOf(key) + key.length();
  int endIndex = data.indexOf(delimiter, startIndex);
  return data.substring(startIndex, endIndex);
}

void updateFirebase() {
  Firebase.setInt("sensors/distance", distance);
  Firebase.setBool("sensors/fire", fireDetected);
  Firebase.setBool("sensors/ir_right", irRight);
  Firebase.setBool("sensors/ir_front", irFront);
  Firebase.setBool("sensors/ir_left", irLeft);
  
  if (Firebase.failed()) {
    Serial.print("Firebase update failed: ");
    Serial.println(Firebase.error());
  } else {
    Serial.println("Firebase updated successfully");
  }
}
