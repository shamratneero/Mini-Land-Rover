#include <Wire.h>
#include <MPU6050.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi credentials
const char* ssid = "HeadTracker_AP";     // Receiver AP SSID
const char* password = "mypassword123";  // Receiver AP password

// UDP setup
WiFiUDP udp;
const char* receiverIP = "192.168.4.1"; // Receiver IP (Receiver NodeMCU)
unsigned int udpPort = 12345;           // UDP port for communication

// MPU6050 setup
MPU6050 mpu;
float pitch = 0.0, yaw = 0.0;
const float alpha = 0.80; // Complementary filter coefficient
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1); // SDA, SCL pins for NodeMCU

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("Transmitter IP: ");
  Serial.println(WiFi.localIP());

  // Initialize MPU6050
  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected!");
  } else {
    Serial.println("MPU6050 failed.");
    while (1); // Halt if MPU6050 not connected
  }

  lastTime = millis();
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;

  // Read data from MPU6050
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Convert gyroscope data to degrees/second
  float gyroYawRate = gz / 131.0; // Sensitivity for 250 deg/s
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // Time in seconds
  lastTime = currentTime;

  // Calculate pitch from accelerometer
  float accelPitch = -atan2(ax, sqrt(ay * ay + az * az)) * 180 / PI;

  // Complementary filter for pitch
  pitch = alpha * (pitch + gyroYawRate * dt) + (1 - alpha) * accelPitch;

  // Integrate gyroscope data for yaw
  yaw += gyroYawRate * dt;

  // Create CSV string
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%.2f,%.2f", pitch, yaw);

  // Send data via UDP
  udp.beginPacket(receiverIP, udpPort);
  udp.write(buffer);
  udp.endPacket();

  Serial.print("Sent: ");
  Serial.println(buffer);

  delay(5);
}
