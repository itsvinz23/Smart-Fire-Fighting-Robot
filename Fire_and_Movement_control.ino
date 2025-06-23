#define PUMP D5
#define ENA D6
#define IN1 D3
#define IN2 D4
#define ir_R D7
#define ir_F D8
#define ir_L D0

void setup() {
  pinMode(PUMP, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ir_R INPUT);
  pinMode(ir_F INPUT);
  pinMode(ir_L INPUT);

  for (int angle = 90; angle <= 140; angle += 5) servoPulse(servo, angle);
  for (int angle = 140; angle >= 40; angle -= 5) servoPulse(servo, angle);
  for (int angle = 40; angle <= 95; angle += 5) servoPulse(servo, angle);
}

void loop() {
  S1 = digitalRead(ir_R);
  S2 = digitalRead(ir_F);
  S3 = digitalRead(ir_L);

  pumpWater();
}

void pumpWater(){
  if (S1 == LOW) {
    Stop();
    digitalWrite(pump, HIGH);
    for (int angle = 90; angle >= 40; angle -= 3) servoPulse(servo, angle);
    for (int angle = 40; angle <= 90; angle += 3) servoPulse(servo, angle);
  }
  else if (S2 == LOW) {
    Stop();
    digitalWrite(pump, HIGH);
    for (int angle = 90; angle <= 140; angle += 3) servoPulse(servo, angle);
    for (int angle = 140; angle >= 40; angle -= 3) servoPulse(servo, angle);
    for (int angle = 40; angle <= 90; angle += 3) servoPulse(servo, angle);
  }
  else if (S3 == LOW) {
    Stop();
    digitalWrite(pump, HIGH);
    for (int angle = 90; angle <= 140; angle += 3) servoPulse(servo, angle);
    for (int angle = 140; angle >= 90; angle -= 3) servoPulse(servo, angle);
  }
  else if (S1 == HIGH) {
    digitalWrite(pump, LOW);
    backword(); delay(100);
    turnRight(); delay(200);
  }
  else if (S2 == HIGH) {
    digitalWrite(pump, LOW);
    forword();
  }
  else if (S3 == HIGH) {
    digitalWrite(pump, LOW);
    backword(); delay(100);
    turnLeft(); delay(200);
  }
  else {
    digitalWrite(pump, LOW);
    Stop();
  }
  delay(10);
 }

void forword() {
 digitalWrite(in1, HIGH);
 digitalWrite(in2, LOW);
 digitalWrite(in3, LOW);
 digitalWrite(in4, HIGH);
}
 void backword() {
 digitalWrite(in1, LOW);
 digitalWrite(in2, HIGH);
 digitalWrite(in3, HIGH);
 digitalWrite(in4, LOW);
}
 void turnRight() {
 digitalWrite(in1, LOW);
 digitalWrite(in2, HIGH);
 digitalWrite(in3, LOW);
 digitalWrite(in4, HIGH);
}
 void turnLeft() {
 digitalWrite(in1, HIGH);
 digitalWrite(in2, LOW);
 digitalWrite(in3, HIGH);
 digitalWrite(in4, LOW);
}
void Stop() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

 void servoPulse(int pin, int angle) {
  int pwm = (angle * 11) + 500;
  digitalWrite(pin, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(pin, LOW);
  delay(50);
}
//because of less pins, only one pmw is currently using here
