// Included Libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>     // For Wi-Fi connectivity on ESP8266
#include <FirebaseESP8266.h> // For direct Firebase Realtime Database communication
#include <ArduinoJson.h>   // For JSON serialization
#include <DHT.h>           // NEW: For DHT temperature/humidity sensor

// Wi-Fi Configuration
const char* WIFI_SSID = "Name"; // Wifi name
const char* WIFI_PASSWORD = "Password123"; // wifi passcode

// Firebase Configuration for Direct Database Access
#define FIREBASE_HOST "firebot-16638-default-rtdb.asia-southeast1.firebasedatabase.app" // Database URL
#define FIREBASE_AUTH "IZInwxAyzE635h5T9SSFjt4z2G4oxgdo4hZrY5Zm" //Database secret key to connect the database (Firebase)

// Firebase Data Objects
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// Sensor pin definitions
//Flame sensors (Its either High or Low. So assuming active Low)
#define FLAME_R D5 // NodeMCU D5 (GPIO14)  
#define FLAME_F D6 // NodeMCU D6 (GPIO12) 
#define FLAME_L D7 // NodeMCU D7 (GPIO13) 

// Water pump
#define PUMP_PIN D2 // NodeMCU D2 (GPIO4) which connects to the water line

// Water level
#define WATER_LEVEL_PIN D3 // NodeMCU D3 (GPIO0)

// Ultrasonic (According to the code firebot is running on)
const int ULTRASONIC_TRIG_PIN = D6; //if D6 is used for FLAME_F
const int ULTRASONIC_ECHO_PIN = D7; //if D7 is used for FLAME_L

// Encoder pins
const int ENCODER_LEFT_PIN = D8; // NodeMCU D8 (GPIO15)
const int ENCODER_RIGHT_PIN = D0; // NodeMCU D0 (GPIO16)

const int BUILTIN_LED_PIN = D4; // NodeMCU D4 (GPIO2)

// Temperature sensor 
#define DHTPIN D1    // Pin where the DHT11 is connected
#define DHTTYPE DHT11 // Type of DHT sensor 

DHT dht(DHTPIN, DHTTYPE); // DHT sensor object (Tempreture)

const int MQ2_GAS_THRESHOLD = 500;    // Raw analog value from MQ-2
const float TEMP_THRESHOLD_CELSIUS_FIRE_ALERT = 40.0; // Temperature in Celsius for fire alert logic

// assigning the global variables 
long lastSensorReadTime = 0;
const long SENSOR_READ_INTERVAL_MS = 3000; // Send data every 3 seconds

// Placeholder for water pump state (true = activated, false = not activated)
bool isWaterPumpActivated = false;

//  Helper functions for wifi cnnectivity
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

// Functions for reding the sensors

// Ultrasonic Sensor (HC-SR04)
float readUltrasonicDistanceCm() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW); // Clears the trigPin condition
  delayMicroseconds(2);

  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH); // Sets the trigPin HIGH for 10 microseconds
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

// Read the water level sensor data
// Returns true if water is detected, false otherwise.
bool readWaterLevel() {
  // The sensor where LOW means water present (meaning the float switch closed)
  return !digitalRead(WATER_LEVEL_PIN);
}

// Placeholder for encoder counts
long leftEncoderCount = 0;
long rightEncoderCount = 0;

void updateEncoderCounts() {
  leftEncoderCount += 1; // Increment for simulation
  rightEncoderCount += 1; // Increment for simulation
}

// Setup Function
void setup() {
  Serial.begin(115200); // Initializing the serial communication for debugging
  delay(10); // Small delay for serial to stabilize
  Serial.println("\nFireBot NodeMCU System Starting...");

  // Initializing sensor pins
  pinMode(FLAME_SENSOR_PIN, INPUT_PULLUP); // Main flame sensor
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP); // Water Level Sensor Pin
  pinMode(PUMP_PIN, OUTPUT);             // Water Pump Control Pin
  digitalWrite(PUMP_PIN, LOW);           // To ensure pump is off initially

  pinMode(ENCODER_LEFT_PIN, INPUT_PULLUP); // Encoder pins
  pinMode(ENCODER_RIGHT_PIN, INPUT_PULLUP);

  pinMode(BUILTIN_LED_PIN, OUTPUT); // Initializing the built-in LED as OUTPUT
  digitalWrite(BUILTIN_LED_PIN, LOW); // Turn off LED initially

  dht.begin(); //Initializing DHT sensor
  Serial.println("DHT Temperature/Humidity Sensor initialized.");

  // Connecting to Wi-Fi
  connectToWiFi();

  // Configure Firebase for direct Realtime Database access
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.databaseSecret = FIREBASE_AUTH; // Using Database Secret

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true); // Auto-reconnect to WiFi if disconnected

  Serial.println("Firebase initialized.");
}

// Loop Function Begin
void loop() {
  // Ensure Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    connectToWiFi();
  }

  // Check if Firebase client is connected and authenticated
  if (Firebase.ready()) {
    long currentTime = millis(); // Get current time in milliseconds

    if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL_MS) {
      lastSensorReadTime = currentTime; // To update last read time

      // Read Fire Detection Sensors
      // Using the single FLAME_SENSOR_PIN for "Flame Detected" status to Firebase
      bool flameDetected = !digitalRead(FLAME_SENSOR_PIN); // Assuming active is present as Low

      int gasRawValue = analogRead(GAS_SENSOR_ANALOG_PIN);

      // Read temperature from DHT sensor
      float temperatureC = dht.readTemperature(); // Read temperature in Celsius
      if (isnan(temperatureC)) { // Check if any NaN (Not a Number) value
        Serial.println("Failed to read temperature from DHT sensor!");
        temperatureC = -999.0; // Indicate an error value
      }

      // Read Water Level
      bool waterInBottle = readWaterLevel();

      // Read Navigation Sensors
      float ultrasonicDistanceCm = readUltrasonicDistanceCm();
      updateEncoderCounts(); // Placeholder update for odometry

      // Fire Detection Logic (for data reporting)
      // if a general "fire detected" state exists
      bool isFireDetectedLogic = false;
      if (flameDetected) { // If the main flame sensor detects fire
        isFireDetectedLogic = true;
      } else if (gasRawValue > MQ2_GAS_THRESHOLD && temperatureC > TEMP_THRESHOLD_CELSIUS_FIRE_ALERT) {
        isFireDetectedLogic = true;
      }

      // Water pump activation logic (based on fire and water availability)
      if (isFireDetectedLogic && waterInBottle) {
        digitalWrite(PUMP_PIN, HIGH); // Turn on pump
        isWaterPumpActivated = true;
      } else {
        digitalWrite(PUMP_PIN, LOW); // Turn off pump
        isWaterPumpActivated = false;
      }

      // Update built-in LED based on fire detection status
      if (isFireDetectedLogic) {
        digitalWrite(BUILTIN_LED_PIN, HIGH);
      } else {
        digitalWrite(BUILTIN_LED_PIN, LOW);
      }

      // Print Sensor Data (for debugging on serial monitor)
      Serial.println(" Sensor Readings ");
      Serial.print("Water In Bottle: "); Serial.println(waterInBottle ? "Yes" : "No");
      Serial.print("Flame Detected: "); Serial.println(flameDetected ? "Yes" : "No"); // This is the status sent to Firebase
      Serial.print("Gas Sensor Raw (A0): "); Serial.println(gasRawValue);
      Serial.print("Temperature (D1/DHT): ");
      if (temperatureC == -999.0) Serial.println("Error!"); else Serial.print(temperatureC); Serial.println(" C");
      Serial.print("Ultrasonic Dist (D6/D7): "); Serial.print(ultrasonicDistanceCm); Serial.println(" cm");
      Serial.print("Left Encoder: "); Serial.println(leftEncoderCount);
      Serial.print("Right Encoder: "); Serial.println(rightEncoderCount);
      Serial.print("Water Pump Status: "); Serial.println(isWaterPumpActivated ? "Activated" : "Not Activated");
      Serial.println("-----------------------");

      //  Prepare JSON Payload for Firebase Realtime Database 
      // Using DynamicJsonDocument for flexibility, adjust size as needed
      DynamicJsonDocument doc(512);

      // Populate sensor data with keys matching according to the dashboard
      doc["waterLevel"] = waterInBottle; // Boolean: true/false (for "Water In Bottle: Yes/No")
      doc["flame"] = flameDetected;      // Boolean: true/false (for "Flame Detected: Yes/No")
      doc["temperature"] = temperatureC; // Numeric: temperature value (for "Fire Proximity (Temp)")

      String gasStatus;
      if (gasRawValue > MQ2_GAS_THRESHOLD) {
        gasStatus = "Smoke Detected";
      } else {
        gasStatus = "Normal";
      }
      doc["gasSmoke"] = gasStatus;

      String obstacleStatus;
      if (ultrasonicDistanceCm > 0 && ultrasonicDistanceCm < 20) { //If the obstacle is closer than 20cm
        obstacleStatus = "Obstacle Ahead";
      } else if (ultrasonicDistanceCm == -1.0) { // Error case from read function
        obstacleStatus = "Error Reading";
      } else {
        obstacleStatus = "Clear";
      }
      doc["obstacle"] = obstacleStatus;

      doc["waterPump"] = isWaterPumpActivated; //true/false for "Water Pump: Activated/Not Activated"

      // Send Data Directly to Firebase Realtime Database 
      const char* firebasePath = "/sensors"; // Path in to the Firebase Database

      Serial.print("Sending JSON data directly to Firebase path: ");
      Serial.println(firebasePath);

      // Use Firebase.set() to write the entire JSON object to the corect path
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

  delay(10); // Adding a small Delay
}
