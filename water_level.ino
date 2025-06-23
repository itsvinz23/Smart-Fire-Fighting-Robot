const int waterSensorPin = A0; // analog input
const int buzzerPin = D1;      
const int ledPin = D2;         

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);

  Serial.println("-----Water Level Monitoring Started-----");
}

void loop() {
  int waterValue = analogRead(waterSensorPin); //reading sensor values
  Serial.print("Water Sensor Value: ");
  Serial.println(waterValue);

  // converting the water value into percentage
  int percentage = map(waterValue, 0, 1023, 0, 100);
  Serial.print("Water Level: ");
  Serial.print(percentage);
  Serial.println("%");

  // trigger alert when water level is in LOW
  if (waterValue < 200) {
    Serial.println("Water Level: LOW - ALERT");
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("Water Level: OK");
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, LOW);
  }

  Serial.println("----------------------------");
  delay(1000); //check for every second
}
