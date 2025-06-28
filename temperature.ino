void setup() 
{
  Serial.begin(115200); // Initialize Serial Monitor
}

void loop() 
{
  int analogValue = analogRead(A0); // Read raw analog input

  // Convert analog value to voltage (0-5V range)
  float voltage = analogValue * (5.0 / 1023.0);

  // Calculate temperature in Celsius (LM35 outputs 10mV per °C)
  float temperatureC = voltage * 100;

  Serial.print("Analog: ");
  Serial.print(analogValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V | Temp: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  delay(1000); 
}
