const int waterSensorPin = A8; 
const int buzzerPin = D20;      
const int ledPin = D21;  

void setup() {
  Serial.begin(9600);             
  pinMode(buzzerPin, OUTPUT);      
  pinMode(ledPin, OUTPUT);         
  digitalWrite(buzzerPin, LOW);    
  digitalWrite(ledPin, LOW);       

  Serial.println("-----Water Level Monitoring Started-----");
}

void loop() {
  int waterValue = analogRead(waterSensorPin); 
  Serial.print("Water Sensor Value: ");
  Serial.println(waterValue);

  // Convert sensor value to percentage
  int percentage = map(waterValue, 0, 1023, 0, 100);
  Serial.print("Water Level: ");
  Serial.print(percentage);
  Serial.println("%");

  // Trigger alert if water level is too low
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
  delay(1000); // Wait 1 second before next check
}
