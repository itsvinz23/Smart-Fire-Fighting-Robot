#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 9
#define DHTTYPE DHT11


DHT dht(DHTPIN, DHTTYPE);

// Use Serial1 on Mega: 18 (TX), 19 (RX)
SoftwareSerial espSerial(18, 19); // RX, TX (connected to NodeMCU via level converter)

void setup() {
  dht.begin();
  
  Serial.begin(9600);      
  espSerial.begin(115200);  
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  bool fireDetected = digitalRead(FIRE_SENSOR_PIN) == HIGH;

  // Check for sensor errors
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read DHT!");
    return;
  }

  // JSON format
 String data = "{\"temp\":" + String(temp, 1) +
              ",\"humidity\":" + String(hum, 1) + "}";


  espSerial.println(data); 
  Serial.println(data);   

  delay(2000);
}
