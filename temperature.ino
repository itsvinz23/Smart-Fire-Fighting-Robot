void setup() 
{
  Serial.begin(115200); 
}

void loop() 
{
  int analogValue = analogRead(A0); // Read raw analog input 

  float voltage = analogValue * (1.0 / 1023.0); // Convert analog value to voltage (0–1V)

  float temperatureC = voltage * 100; // Convert voltage to temperature in °C 

 
  Serial.print("Analog: ");
  Serial.print(analogValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3); 
  Serial.print(" V | Temp: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  delay(1000); 
}
