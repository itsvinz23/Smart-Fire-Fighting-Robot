// Icluded Libraries
#include <OneWire.h>
#include <DallasTemperature.h>
  
#include <ESP8266WiFi.h> // For Wi-Fi connectivity on ESP8266
#include <ESP8266HTTPClient.h> // For making HTTP requests
#include <ArduinoJson.h// For JSON serialization

// Wi-Fi Configuration 
const char* ssid = "Name";
const char* password = "Password123";

// Firebase Cloud Function Endpoint
const char* FIREBASE_CLOUD_FUNCTION_URL = "YOUR_FIREBASE_CLOUD_FUNCTION_HTTP_TRIGGER_URL";

// Fire Detection Sensors
const int FLAME_SENSOR_PIN = D5; // NodeMCU D5 (GPIO14) 
const int GAS_SENSOR_ANALOG_PIN = A0; // NodeMCU's only analog input pin. 
const int TEMPERATURE_ONEWIRE_PIN = D1; // NodeMCU D1 (GPIO5)

// Navigation Sensors
const int ULTRASONIC_TRIG_PIN = D6; // NodeMCU D6 (GPIO12)
const int ULTRASONIC_ECHO_PIN = D7; // NodeMCU D7 (GPIO13)

const int ENCODER_LEFT_PIN = D8; //  NodeMCU D8 (GPIO15) for Left Wheel Encoder
const int ENCODER_RIGHT_PIN = D0; // NodeMCU D0 (GPIO16) for Right Wheel Encoder
const int BUILTIN_LED_PIN = D4; // NodeMCU D4 (GPIO2) 

OneWire oneWire(TEMPERATURE_ONEWIRE_PIN);       // OneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);            // OneWire reference to Dallas Temperature sensor

const int MQ2_GAS_THRESHOLD = 500;   // For now: Raw analog value from MQ-2
const float TEMP_THRESHOLD_CELSIUS = 40.0; // Temperature in Celsius 

// --- Global Variables ---
long lastSensorReadTime = 0;
const long SENSOR_READ_INTERVAL_MS = 3000; // Send data every 3 seconds

// --- Helper Functions ---
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

// --- Sensor Reading Functions ---

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
    Serial.println("Ultrasonic: Timeout or no echo.");  // pulseIn returns 0 if timeout occurs
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
  leftEncoderCount += 1;
  rightEncoderCount += 1;
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
}

// Loop Function Begin
void loop() {
  // Ensure Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    connectToWiFi();
  }

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

    // Print Sensor Data
    Serial.println(" Sensor Readings ");
    Serial.print("Flame Sensor (D5): "); Serial.println(flameDetected ? "FLAME!" : "No Flame.");
    Serial.print("Gas Sensor Raw (A0): "); Serial.println(gasRawValue);
    Serial.print("Temperature (D1): ");
    
    if (temperatureC == -127.00) Serial.println("Error!"); else Serial.print(temperatureC); Serial.println(" C");
    Serial.print("Ultrasonic Dist (D6/D7): "); Serial.print(ultrasonicDistanceCm); Serial.println(" cm");
    Serial.print("Left Encoder: "); Serial.println(leftEncoderCount);
    Serial.print("Right Encoder: "); Serial.println(rightEncoderCount);
    Serial.println("------------------------------------");

    // --- Prepare JSON Payload ---
    // Estimate JSON buffer size (adjust based on your actual data)
    // Now including ultrasonic and odometry data
    const size_t CAPACITY = JSON_OBJECT_SIZE(9) + 200; // More fields, more space needed
    StaticJsonDocument<CAPACITY> doc;

    // Fire Detection Data
    doc["flameDetected"] = flameDetected;
    doc["gasRawValue"] = gasRawValue;
    doc["temperatureC"] = temperatureC;
    doc["isFireDetected"] = isFireDetected;

    // Navigation Data
    doc["ultrasonicDistanceCm"] = ultrasonicDistanceCm;
    JsonObject odometry = doc.createNestedObject("odometry");
    odometry["leftPulses"] = leftEncoderCount;
    odometry["rightPulses"] = rightEncoderCount;
    
    odometry["deltaXMeters"] = 0.0;
    odometry["deltaYMeters"] = 0.0;
    odometry["deltaThetaRad"] = 0.0;

    doc["timestamp"] = millis(); // Milliseconds since board start

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Send Data to Firebase Cloud Function through HTTP POST 
    HTTPClient http;
    http.begin(FIREBASE_CLOUD_FUNCTION_URL);
    http.addHeader("Content-Type", "application/json");

    Serial.print("Sending data to Firebase... ");
    Serial.println(jsonPayload); // Print the JSON payload thats being sent

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println("Response from server: ");
      Serial.println(response);
    } 
    else {
      Serial.print("HTTP Error code: ");
      Serial.println(httpResponseCode);
      Serial.println(http.errorToString(httpResponseCode));
    }
    
    http.end(); // Free resources
  }
  
  delay(10); //Small Delay
}
