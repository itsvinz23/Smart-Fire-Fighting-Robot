// Icluded Libraries
#include <OneWire.h>
#include <DallasTemperature.h>
  
#include <ESP8266WiFi.h> // For Wi-Fi connectivity on ESP8266
#include <FirebaseESP8266.h> // Header for Firebase ESP8266 library

// For JSON serialization that is needed for creating the data structure before we send data to Firebase database
#include <ArduinoJson.h>

// Wi-Fi Configuration 
const char* WIFI_SSID = "Name"; // Wifi name
const char* WIFI_PASSWORD = "Password123"; // Wifi password

// Firebase configuration to access the database directly
#define FIREBASE_HOST "firebot-16638-default-rtdb.asia-southeast1.firebasedatabase.app" // The database URL
#define FIREBASE_AUTH "something23456789" // Database secret code

// Firebase Data Object
FirebaseData firebaseData;

// FirebaseConfig
FirebaseConfig firebaseConfig;

// FirebaseAuth
FirebaseAuth firebaseAuth;

// Fire Detection Sensors
const int FLAME_SENSOR_PIN = D5; // NodeMCU D5 (GPIO14) 
const int GAS_SENSOR_ANALOG_PIN = A0; // NodeMCU's only analog input pin. 
const int TEMPERATURE_ONEWIRE_PIN = D1; // NodeMCU D1 (GPIO5)

// Navigation Sensors
const int ULTRASONIC_TRIG_PIN = D6; // NodeMCU D6 (GPIO12)
const int ULTRASONIC_ECHO_PIN = D7; // NodeMCU D7 (GPIO13)

const int ENCODER_LEFT_PIN = D8; //  NodeMCU D8 (GPIO15) for Left Wheel Encoder
const int ENCODER_RIGHT_PIN = D0; // NodeMCU D0 (GPIO16) for Right Wheel Encoder
const int BUILTIN_LED_PIN = D4; // NodeMCU D4 (GPIO2) 

OneWire oneWire(TEMPERATURE_ONEWIRE_PIN);       // OneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);            // OneWire reference to Dallas Temperature sensor

const int MQ2_GAS_THRESHOLD = 500;   // Raw analog value from MQ-2 For testing
const float TEMP_THRESHOLD_CELSIUS = 40.0; // Temperature in Celsius 

// Global variables 
long lastSensorReadTime = 0;
const long SENSOR_READ_INTERVAL_MS = 3000; // Send data every 3 seconds

// Helper functions 
void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(BUILTIN_LED_PIN, HIGH); // Blink LED during connection
    delay(200);
    digitalWrite(BUILTIN_LED_PIN, LOW);
  }

  Serial.println("\nWi-Fi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(BUILTIN_LED_PIN, LOW); // Turn off LED once connected
}

// Sensor reading functions one by one

// Ultrasonic sensor (HC-SR04)
float readUltrasonicDistanceCm() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW); //This clears the trigPin condition
  delayMicroseconds(2);
  
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH); // Sets the trigPin HIGH for 10 microseconds
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000); // 30ms timeout for ~5m range

  if (duration == 0) {
    Serial.println("Ultrasonic: Timeout or no echo.");  // pulseIn returns 0 if timeout occurs
    return -1.0; // Indicate error 
  }

  // Calculating the distance
  float distanceCm = (float)duration * 0.0343 / 2.0;
  return distanceCm;
}

//PlaceHolder
long leftEncoderCount = 0;
long rightEncoderCount = 0;

void updateEncoderCounts() {
  leftEncoderCount += 1; // Example: increment for simulation
  rightEncoderCount += 1; // Example: increment for simulation
}

// Setup Function
void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  delay(10); // Small delay for serial to stabilize
  Serial.println("\nFireBot NodeMCU System Starting...");

  // Initialize sensor pins
  pinMode(FLAME_SENSOR_PIN, INPUT_PULLUP);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  
  pinMode(ENCODER_LEFT_PIN, INPUT_PULLUP); // Encoder pins (setup as inputs, ISRs would be attached here for real usage)
  pinMode(ENCODER_RIGHT_PIN, INPUT_PULLUP);

  pinMode(BUILTIN_LED_PIN, OUTPUT); // Initializing the built-in LED as OUTPUT
  digitalWrite(BUILTIN_LED_PIN, LOW); // Turn off LED initially

  sensors.begin(); // Initialize DS18B20 sensor
  Serial.println("DS18B20 Temperature Sensor initialized.");

  // Connect to Wi-Fi
  connectToWiFi();

  // NEW: Configure Firebase
  firebaseConfig.host = FIREBASE_HOST;
  // Option A: Using Database Secret (less secure, but quick for testing)
  firebaseConfig.signer.tokens.databaseSecret = FIREBASE_AUTH;

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true); // Auto-reconnect to WiFi if disconnected

  Serial.println("Firebase initialized.");
}

// Loop Function Begin
void loop() {
  // Ensure Wi-Fi connection and Firebase is ready
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    connectToWiFi();
  }

  // NEW: Check if Firebase connection is ready
  if (Firebase.ready()) { // Make sure Firebase client is connected
    long currentTime = millis(); // Get current time in milliseconds

    if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL_MS) {
      lastSensorReadTime = currentTime; // Update last read time

      // Read Fire Detection Sensors
      bool flameDetected = !digitalRead(FLAME_SENSOR_PIN); // Assuming active LOW
      int gasRawValue = analogRead(GAS_SENSOR_ANALOG_PIN);
      sensors.requestTemperatures();
      float temperatureC = sensors.getTempCByIndex(0);

      // Read Navigation Sensors 
      float ultrasonicDistanceCm = readUltrasonicDistanceCm();
      updateEncoderCounts(); // Placeholder update for odometry


      // Fire Detection Logic 
      bool isFireDetected = false;
      if (flameDetected) {
        isFireDetected = true;
        Serial.println("PRIMARY FIRE ALERT: FLAME DETECTED!");
      } else if (gasRawValue > MQ2_GAS_THRESHOLD && temperatureC > TEMP_THRESHOLD_CELSIUS) {
        isFireDetected = true;
        Serial.println("SECONDARY FIRE ALERT: High Gas & High Temperature!");
      }

      // Update built-in LED based on fire detection status
      if (isFireDetected) {
        digitalWrite(BUILTIN_LED_PIN, HIGH);
      } else {
        digitalWrite(BUILTIN_LED_PIN, LOW);
      }

      // Print Sensor Data (for debugging on serial monitor)
      Serial.println(" Sensor Readings ");
      Serial.print("Flame Sensor (D5): "); Serial.println(flameDetected ? "FLAME!" : "No Flame.");
      Serial.print("Gas Sensor Raw (A0): "); Serial.println(gasRawValue);
      Serial.print("Temperature (D1): ");
      
      if (temperatureC == -127.00) Serial.println("Error!"); else Serial.print(temperatureC); Serial.println(" C");
      Serial.print("Ultrasonic Dist (D6/D7): "); Serial.print(ultrasonicDistanceCm); Serial.println(" cm");
      Serial.print("Left Encoder: "); Serial.println(leftEncoderCount);
      Serial.print("Right Encoder: "); Serial.println(rightEncoderCount);
      Serial.println("------------------------------------");

      // --- Prepare JSON Payload using ArduinoJson (for structuring data) ---
      // Using DynamicJsonDocument for flexibility, or StaticJsonDocument with sufficient size
      DynamicJsonDocument doc(512); // Adjust size as needed, 512 bytes is a good starting point

      // Populate sensor data
      doc["waterLevel"] = 0; // You don't have a water level sensor in this code, so I'm setting a placeholder.
                                // Add your water level sensor reading here if you have one.
      doc["flame"] = flameDetected;
      doc["temperature"] = temperatureC;
      // Determine gasSmoke status
      String gasStatus;
      if (gasRawValue > MQ2_GAS_THRESHOLD) {
        gasStatus = "Smoke Detected";
      } else {
        gasStatus = "Normal";
      }
      doc["gasSmoke"] = gasStatus;

      // Determine obstacle status
      String obstacleStatus;
      if (ultrasonicDistanceCm > 0 && ultrasonicDistanceCm < 20) { // Example: Obstacle if closer than 20cm
        obstacleStatus = "Obstacle Ahead";
      } else if (ultrasonicDistanceCm == -1.0) { // Error case from read function
          obstacleStatus = "Error Reading";
      } else {
        obstacleStatus = "Clear";
      }
      doc["obstacle"] = obstacleStatus;


      // --- Send Data to Firebase Realtime Database ---
      // Path in Firebase Realtime Database where data will be stored (e.g., "sensors")
      const char* firebasePath = "/sensors";

      Serial.print("Sending JSON data to Firebase path: ");
      Serial.println(firebasePath);

      // Send the JSON object to Firebase
      if (Firebase.set(firebaseData, firebasePath, doc)) {
        Serial.println("Firebase Set OK");
      } else {
        Serial.print("Firebase Set Failed: ");
        Serial.println(firebaseData.errorReason());
      }
    }
  } else {
    Serial.print("Firebase not ready: ");
    Serial.println(Firebase.errorReason());
  }
  
  delay(10); //Small Delay
}
