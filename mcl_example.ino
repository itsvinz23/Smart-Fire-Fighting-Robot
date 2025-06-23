#include <ESP8266WiFi.h>  // Or appropriate lib for your robot
#include <math.h>
#include <vector>

#define NUM_PARTICLES 20
#define MAP_WIDTH 10
#define MAP_HEIGHT 10

struct Particle {
  float x;
  float y;
  float theta;
  float weight;
};

Particle particles[NUM_PARTICLES];

int map[MAP_HEIGHT][MAP_WIDTH] = {
  // 0 = free space, 1 = wall/obstacle
  {1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,1,0,0,0,1},
  {1,0,0,1,0,1,0,0,0,1},
  {1,0,0,1,0,1,0,0,0,1},
  {1,0,0,1,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1}
};

void initializeParticles() {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].x = random(1, MAP_WIDTH - 1);
    particles[i].y = random(1, MAP_HEIGHT - 1);
    particles[i].theta = random(0, 360) * DEG_TO_RAD;
    particles[i].weight = 1.0 / NUM_PARTICLES;
  }
}

void moveParticles(float deltaDist, float deltaTheta) {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].x += deltaDist * cos(particles[i].theta) + random(-10,10)/100.0;
    particles[i].y += deltaDist * sin(particles[i].theta) + random(-10,10)/100.0;
    particles[i].theta += deltaTheta + random(-5,5) * DEG_TO_RAD;
  }
}

float getSensorReadingSimulated(float x, float y, float theta) {
  // Basic raycasting: stop at wall or edge
  int cx = (int)x, cy = (int)y;
  int steps = 0;
  while (cx >= 0 && cy >= 0 && cx < MAP_WIDTH && cy < MAP_HEIGHT && map[cy][cx] == 0 && steps < 10) {
    x += 0.1 * cos(theta);
    y += 0.1 * sin(theta);
    cx = (int)x;
    cy = (int)y;
    steps++;
  }
  return steps * 0.1;  // simulated distance
}

float getRealSensorReading() {
  // Example: return distance from IR sensor (you must define this)
  return analogRead(A0) / 100.0;  // Convert to distance in meters
}

void updateWeights() {
  float realReading = getRealSensorReading();
  float totalWeight = 0;
  for (int i = 0; i < NUM_PARTICLES; i++) {
    float simReading = getSensorReadingSimulated(particles[i].x, particles[i].y, particles[i].theta);
    float error = fabs(realReading - simReading);
    particles[i].weight = exp(-error * error / 0.1);  // Gaussian-like
    totalWeight += particles[i].weight;
  }

  // Normalize weights
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].weight /= totalWeight;
  }
}

void resampleParticles() {
  Particle newParticles[NUM_PARTICLES];
  for (int i = 0; i < NUM_PARTICLES; i++) {
    float r = random(0, 1000) / 1000.0;
    float cumsum = 0;
    for (int j = 0; j < NUM_PARTICLES; j++) {
      cumsum += particles[j].weight;
      if (r < cumsum) {
        newParticles[i] = particles[j];
        break;
      }
    }
  }
  memcpy(particles, newParticles, sizeof(particles));
}

Particle estimatePose() {
  Particle estimate = {0, 0, 0, 0};
  for (int i = 0; i < NUM_PARTICLES; i++) {
    estimate.x += particles[i].x * particles[i].weight;
    estimate.y += particles[i].y * particles[i].weight;
    estimate.theta += particles[i].theta * particles[i].weight;
  }
  return estimate;
}

void setup() {
  Serial.begin(9600);
  initializeParticles();
}

void loop() {
  float forward = 0.2;        // Assume moved 20 cm forward
  float rotate = 0;           // No rotation

  moveParticles(forward, rotate);
  updateWeights();
  resampleParticles();
  Particle pose = estimatePose();

  Serial.print("Estimated Pose: ");
  Serial.print(pose.x); Serial.print(", ");
  Serial.print(pose.y); Serial.print(", ");
  Serial.println(pose.theta * 180 / PI);

  delay(1000);
}
