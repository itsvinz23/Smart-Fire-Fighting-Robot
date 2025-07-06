#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoJson.h>

// WiFi Credentials
#define WIFI_SSID "Rookie's net"
#define WIFI_PASSWORD "Qwas1236"

// Firebase Credentials
#define FIREBASE_HOST "fir-12aea-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "dohNQAhOmVXFZZqY5ERwDqG0hHHniAlwBwVv6d0u"

// Create Firebase config and auth objects
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(" Booting...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("✅ WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Firebase config setup
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Connected. Waiting for Arduino data...");
}

void loop() {
  if (Serial.available()) {
    String jsonStr = Serial.readStringUntil('\n');

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);

    if (error) {
      Serial.print("❌ JSON Error: ");
      Serial.println(error.c_str());
      return;
    }

    // Extract data from JSON
    float temperature = doc["temperature"] | 0.0;
    float humidity = doc["humidity"] | 0.0;
    String fireStatus = doc["fire_status"] | "No";
    String waterStatus = doc["water_status"] | "No";
    float distance = doc["distance"] | 0.0;

    Serial.println("📤 Uploading to Firebase...");

    Firebase.setFloat(firebaseData, "/sensors/fireDistance", distance);
    Firebase.setString(firebaseData, "/sensors/flameDetected", fireStatus);
    Firebase.setFloat(firebaseData, "/sensors/humidity", humidity);
    Firebase.setFloat(firebaseData, "/sensors/temperature", temperature);
    Firebase.setString(firebaseData, "/sensors/waterTank", waterStatus);

    if (firebaseData.httpCode() == 200) {
      Serial.println("✅ Uploaded to Firebase!");
    } else {
      Serial.print("⚠️ Upload Failed: ");
      Serial.println(firebaseData.errorReason());
    }

    Serial.println("----------------------");
  }
}
