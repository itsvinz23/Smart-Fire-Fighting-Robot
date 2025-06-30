// Included Libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>     // For Wi-Fi connectivity on ESP8266
#include <FirebaseESP8266.h> // For direct Firebase Realtime Database communication
#include <ArduinoJson.h>   // For JSON serialization

// Wi-Fi Configuration
const char* WIFI_SSID = "Name"; // My device name for the SSId 
const char* WIFI_PASSWORD = "Password123"; // my wifi passcode

// Firebase configuration for direct database access
#define FIREBASE_HOST "firebot-16638-default-rtdb.asia-southeast1.firebasedatabase.app" // The database URL
#define FIREBASE_AUTH "Firebase secret key" // Will replace later

// Define firebase Data Object
FirebaseData firebaseData;

// Define firebaseConfig
FirebaseConfig firebaseConfig;

// Define firebaseAuth
FirebaseAuth firebaseAuth;

// Sensor pin definitions
// Fire Detection Sensors
const int FLAME_SENSOR_PIN = D5;      // NodeMCU D5 (GPIO14)
const int GAS_SENSOR_ANALOG_PIN = A0; // NodeMCU's only analog input pin.
const int TEMPERATURE_ONEWIRE_PIN = D1; // NodeMCU D1 (GPIO5)

// Water level sensor pin
const int WATER_LEVEL_SENSOR_PIN = D3; // NodeMCU D3 (GPIO0) 

// Navigation sensors
const int ULTRASONIC_TRIG_PIN = D6;   // NodeMCU D6 (GPIO12)
const int ULTRASONIC_ECHO_PIN = D7;   // NodeMCU D7 (GPIO13)

const int ENCODER_LEFT_PIN = D8;      // NodeMCU D8 (GPIO15) for Left Wheel Encoder
const int ENCODER_RIGHT_PIN = D0;     // NodeMCU D0 (GPIO16) for Right Wheel Encoder
const int BUILTIN_LED_PIN = D4;       // NodeMCU D4 (GPIO2)

// Water pump control Pin
const int WATER_PUMP_PIN = D2;        // NodeMCU D2 (GPIO4)

OneWire oneWire(TEMPERATURE_ONEWIRE_PIN);      // OneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);           // OneWire reference to Dallas Temperature sensor

const int MQ2_GAS_THRESHOLD = 500;    // Raw analog value from MQ-2
const float TEMP_THRESHOLD_CELSIUS_FIRE_ALERT = 40.0; // Temperature in Celsius for fire alert logic

// Global variables 
long lastSensorReadTime = 0;
const long SENSOR_READ_INTERVAL_MS = 3000; // Send data every 3 seconds

// Placeholder for water pump state (true = activated, false = not activated)
bool isWaterPumpActivated = false;

// Helper functions 
void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(BUILTIN_LED_PIN, HIGH); // Blink LED during connection that indicates with the binks
    delay(200);
    digitalWrite(BUILTIN_LED_PIN, LOW);
  }

  Serial.println("\nWi-Fi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(BUILTIN_LED_PIN, LOW); // Turn off LED once connected
}

// Sensor reading functions

// Ultrasonic Sensor (HC-SR04)
float readUltrasonicDistanceCm() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW); // Clears the trigPin condition
  delayMicroseconds(2);

  digitalWrite(ULTRasonic_TRIG_PIN, HIGH); // Sets the trigPin HIGH for 10 microseconds
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000); // 30ms timeout for ~5m range

  if (duration == 0) {
    Serial.println("Ultrasonic: Timeout or no echo.");
    return -1.0; // Indicate error
  }

  // Calculating the distance
  float distanceCm = (float)duration * 0.0343 / 2.0;
  return distanceCm;
}

// Read Water Level Sensor (digital input)
// Returns true if water is detected, false otherwise.
bool readWaterLevel() {
  // Adjust logic based on your specific sensor (HIGH for water)
  return !digitalRead(WATER_LEVEL_SENSOR_PIN);
}

// Placeholder for encoder counts
long leftEncoderCount = 0;
long rightEncoderCount = 0;

void updateEncoderCounts() {
  leftEncoderCount += 1; // Increment for simulation
  rightEncoderCount += 1; // Increment for simulation
}

// Setup function
void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  delay(10); // Small delay for serial to stabilize
  Serial.println("\nFireBot NodeMCU System Starting...");

  // Initialize sensor pins
  pinMode(FLAME_SENSOR_PIN, INPUT_PULLUP);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT_PULLUP); // Water level sensor pin
  pinMode(WATER_PUMP_PIN, OUTPUT);             // Water pump control pin
  digitalWrite(WATER_PUMP_PIN, LOW);           // Ensure pump is off initially

  pinMode(ENCODER_LEFT_PIN, INPUT_PULLUP); // Encoder pins
  pinMode(ENCODER_RIGHT_PIN, INPUT_PULLUP);

  pinMode(BUILTIN_LED_PIN, OUTPUT); // Initializing the built-in LED as OUTPUT
  digitalWrite(BUILTIN_LED_PIN, LOW); // Turn off LED initially

  sensors.begin(); // Initialize DS18B20 sensor
  Serial.println("DS18B20 Temperature Sensor initialized.");

  // Connect to Wi-Fi
  connectToWiFi();

  // Configure firebase for direct realtime database access
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.databaseSecret = FIREBASE_AUTH; // Using Database Secret key

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true); // Auto-reconnect to WiFi if it get disconnected

  Serial.println("Firebase initialized.");
}

// Loop function begin
void loop() {
  //to make sure the Wi-Fi connection and Firebase is ready
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    connectToWiFi();
  }

  // Check if Firebase client is connected and authenticated
  if (Firebase.ready()) {
    long currentTime = millis(); // Get current time in milliseconds

    if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL_MS) {
      lastSensorReadTime = currentTime; // Update last read time

      // Read Fire Detection Sensors
      bool flameDetected = !digitalRead(FLAME_SENSOR_PIN); 
      int gasRawValue = analogRead(GAS_SENSOR_ANALOG_PIN);
      sensors.requestTemperatures();
      float temperatureC = sensors.getTempCByIndex(0); // The temperature value

      // Read Water Level
      bool waterInBottle = readWaterLevel();

      // Read Navigation Sensors
      float ultrasonicDistanceCm = readUltrasonicDistanceCm();
      updateEncoderCounts(); // Placeholder update for odometry

      // Fire Detection Logic
      bool isFireDetected = false;
      if (flameDetected) {
        isFireDetected = true;
        Serial.println("PRIMARY FIRE ALERT: FLAME DETECTED!");
      } else if (gasRawValue > MQ2_GAS_THRESHOLD && temperatureC > TEMP_THRESHOLD_CELSIUS_FIRE_ALERT) {
        isFireDetected = true;
        Serial.println("SECONDARY FIRE ALERT: High Gas & High Temperature!");
      }

      // Water pump activation logic (based on fire and water availability)
      if (isFireDetected && waterInBottle) {
        digitalWrite(WATER_PUMP_PIN, HIGH); // Turn on pump
        isWaterPumpActivated = true;
      } else {
        digitalWrite(WATER_PUMP_PIN, LOW); // Turn off pump
        isWaterPumpActivated = false;
      }

      // Update built-in LED based on fire detection status
      if (isFireDetected) {
        digitalWrite(BUILTIN_LED_PIN, HIGH);
      } else {
        digitalWrite(BUILTIN_LED_PIN, LOW);
      }

      // Print Sensor Data (for debugging on serial monitor)
      Serial.println("--- Sensor Readings ---");
      Serial.print("Water In Bottle: "); Serial.println(waterInBottle ? "Yes" : "No");
      Serial.print("Flame Detected: "); Serial.println(flameDetected ? "Yes" : "No");
      Serial.print("Gas Sensor Raw (A0): "); Serial.println(gasRawValue);
      Serial.print("Temperature (D1): ");
      
      if (temperatureC == -127.00) Serial.println("Error!"); else Serial.print(temperatureC); Serial.println(" C");
      Serial.print("Ultrasonic Dist (D6/D7): "); Serial.print(ultrasonicDistanceCm); Serial.println(" cm");
      Serial.print("Left Encoder: "); Serial.println(leftEncoderCount);
      Serial.print("Right Encoder: "); Serial.println(rightEncoderCount);
      Serial.print("Water Pump Status: "); Serial.println(isWaterPumpActivated ? "Activated" : "Not Activated");
      Serial.println("-----------------------");

  
      // Using DynamicJsonDocument for flexibility, adjust size as needed
      DynamicJsonDocument doc(512);

      // Populate sensor data with keys matching your web dashboard's app.js expectations
      doc["waterLevel"] = waterInBottle; // Boolean: true/false. Boolean as the data type
      doc["flame"] = flameDetected;      // Boolean: true/false
      doc["temperature"] = temperatureC; // Numeric: temperature value

      String gasStatus;
      if (gasRawValue > MQ2_GAS_THRESHOLD) {
        gasStatus = "Smoke Detected";
      } else {
        gasStatus = "Normal";
      }
      doc["gasSmoke"] = gasStatus;

      String obstacleStatus;
      if (ultrasonicDistanceCm > 0 && ultrasonicDistanceCm < 20) { //If the Obstacle if closer than 20cm
        obstacleStatus = "Obstacle Ahead";
      } else if (ultrasonicDistanceCm == -1.0) { // Error case from read function
        obstacleStatus = "Error Reading";
      } else {
        obstacleStatus = "Clear";
      }
      doc["obstacle"] = obstacleStatus;

      doc["waterPump"] = isWaterPumpActivated; // Boolean: true/false

      //This send the data to the firebase real time dtabase 
      const char* firebasePath = "/sensors"; // Path in the firebase realtime database

      Serial.print("Sending JSON data directly to Firebase path: ");
      Serial.println(firebasePath);

      // Use Firebase.set() to write the entire JSON object to the specified path
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

  delay(10); // Small Delay
}
