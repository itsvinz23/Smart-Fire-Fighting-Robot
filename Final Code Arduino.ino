#include <Servo.h>
#include <DHT.h>

// ================= PIN DEFINITIONS =================
// Flame Sensors
#define FLAME_R 30
#define FLAME_F 32
#define FLAME_L 34

// Motors - L298N
#define enA 10
#define enB 9
#define in1 6
#define in2 5
#define in3 4
#define in4 3

// Water Pump
#define PUMP_PIN 26

// Servo
#define SERVO_PIN A4
#define SERVO_CENTER 90

// Water Level
#define WATER_LEVEL_PIN 36

// Ultrasonic
#define TRIG_PIN 25
#define ECHO_PIN 24
#define SAFE_DISTANCE 15

// DHT Sensor
#define DHTPIN 28
#define DHTTYPE DHT11

// Motor Speed & Timing
#define MOTOR_SPEED 160

// ================ GLOBAL OBJECTS ===================
DHT dht(DHTPIN, DHTTYPE);
Servo fireServo;

float temperature = 0.0;
float humidity = 0.0;

// ================= SETUP ===========================
void setup() {
  Serial.begin(9600);      
  Serial1.begin(115200);   

  dht.begin();
  fireServo.attach(SERVO_PIN);
  fireServo.write(SERVO_CENTER);

  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);

  pinMode(FLAME_R, INPUT_PULLUP);
  pinMode(FLAME_F, INPUT_PULLUP);
  pinMode(FLAME_L, INPUT_PULLUP);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  testActuators();
  Serial.println("✅ Fire Robot READY");
}

// ================= MOVEMENT ========================
void forward() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, MOTOR_SPEED);
  analogWrite(enB, MOTOR_SPEED);
}

void backward() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, MOTOR_SPEED);
  analogWrite(enB, MOTOR_SPEED);
}

void turnRight() {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, MOTOR_SPEED);
  analogWrite(enB, MOTOR_SPEED);
}

void turnLeft() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, MOTOR_SPEED);
  analogWrite(enB, MOTOR_SPEED);
}

void stopMotors() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
}

// =============== ULTRASONIC =======================
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

// ================ FIRE ============================
bool isFireDetected() {
  bool right = !digitalRead(FLAME_R);
  bool front = !digitalRead(FLAME_F);
  bool left = !digitalRead(FLAME_L);
  return right || front || left;
}

void extinguishSmart() {
  if (digitalRead(WATER_LEVEL_PIN) == HIGH) {
    Serial.println("🚫 No water to spray!");
    return;
  }

  Serial.println("🔥 Smart SPRAYING FIRE until gone...");
  digitalWrite(PUMP_PIN, HIGH);

  int fireGoneCount = 0;
  int sweepCount = 0;
  int maxSweeps = 20;

  while (sweepCount < maxSweeps) {
    fireServo.write(40); delay(300);
    fireServo.write(140); delay(300);
    fireServo.write(SERVO_CENTER); delay(200);

    if (!isFireDetected()) {
      fireGoneCount++;
      Serial.println("🟩 No Fire (" + String(fireGoneCount) + "x)");
    } else {
      fireGoneCount = 0;
      Serial.println("🟥 Fire Still Present!");
    }

    if (fireGoneCount >= 3) {
      Serial.println("✅ Fire is OUT!");
      break;
    }

    sweepCount++;
  }

  digitalWrite(PUMP_PIN, LOW);
  fireServo.write(SERVO_CENTER);
}

void approachFire() {
  Serial.println("👣 Approaching fire...");
  long dist = readDistanceCM();
  while (dist > SAFE_DISTANCE && isFireDetected()) {
    forward(); delay(200);
    stopMotors(); delay(100);
    dist = readDistanceCM();
    Serial.print("📏 Distance: "); Serial.println(dist);
  }
  stopMotors();
  Serial.println("📍 At safe spot.");
}

void retreat() {
  Serial.println("↩ Retreating...");
  backward(); delay(800);
  turnRight(); delay(500);
  stopMotors();
}

// ================ TEST ACTUATORS ==================
void testActuators() {
  Serial.println("🧪 Testing...");

  for (int i = 40; i <= 140; i += 20) {
    fireServo.write(i);
    delay(200);
  }
  fireServo.write(SERVO_CENTER);

  digitalWrite(PUMP_PIN, HIGH);
  delay(1000);
  digitalWrite(PUMP_PIN, LOW);

  forward(); delay(400);
  backward(); delay(400);
  turnLeft(); delay(400);
  turnRight(); delay(400);
  stopMotors();
}

// =================== LOOP =========================
void loop() {
  bool rightFire = !digitalRead(FLAME_R);
  bool frontFire = !digitalRead(FLAME_F);
  bool leftFire = !digitalRead(FLAME_L);

  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("❌ DHT read failed!");
    return;
  }

  // 🔥 If fire is detected, act!
  if (isFireDetected()) {
    if (frontFire) {
      approachFire();
      extinguishSmart();
      retreat();
    } else if (rightFire) {
      turnRight(); delay(500); stopMotors();
      extinguishSmart();
      retreat();
    } else if (leftFire) {
      turnLeft(); delay(500); stopMotors();
      extinguishSmart();
      retreat();
    }
  }

  // 📤 Send data to NodeMCU
  bool waterDetected = digitalRead(WATER_LEVEL_PIN) == LOW;
  String waterStatus = waterDetected ? "Water Detected" : "No Water";
  String fireStatus = isFireDetected() ? "FIRE DETECTED" : "No Fire";

  long dist = readDistanceCM();

  String data = "{";
  data += "\"temperature\":" + String(temperature, 1) + ",";
  data += "\"humidity\":" + String(humidity, 1) + ",";
  data += "\"fire_status\":\"" + fireStatus + "\",";
  data += "\"water_status\":\"" + waterStatus + "\",";
  data += "\"distance\":" + String(dist);
  data += "}";

  Serial1.println(data);
  Serial.println("📤 Sent to NodeMCU: " + data);

  delay(1000);
}
