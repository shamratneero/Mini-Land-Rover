#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

// Wi-Fi credentials
const char* ssid = "HeadTracker_AP"; // AP SSID (Receiver Mode)
const char* password = "mypassword123"; // AP Password

// UDP setup
WiFiUDP udp;
unsigned int udpPort = 12345; // Port to listen on
char incomingPacket[128];

// Servo pins
Servo yawServo; // Servo for yaw
Servo pitchServo; // Servo for pitch
const int yawPin = D5;
const int pitchPin = D6;

// Servo parameters
int yawPosition = 90; // Initial yaw position (center)
int pitchPosition = 90; // Initial pitch position (center)

void setup() {
    Serial.begin(115200);

    // Start Wi-Fi in AP mode
    WiFi.softAP(ssid, password);
    Serial.println("Wi-Fi AP started");
    Serial.print("Receiver IP: ");
    Serial.println(WiFi.softAPIP());

    // Start UDP
    udp.begin(udpPort);
    Serial.print("Listening on UDP port ");
    Serial.println(udpPort);

    // Attach servos
    yawServo.attach(yawPin);
    pitchServo.attach(pitchPin);
    yawServo.write(yawPosition);
    pitchServo.write(pitchPosition);
}

void loop() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
        if (len > 0) {
            incomingPacket[len] = '\0'; // Null-terminate the string
        }

        Serial.print("Received Packet: ");
        Serial.println(incomingPacket);

        // Parse received data (format: "pitch,yaw")
        float pitch, yaw;
        if (sscanf(incomingPacket, "%f,%f", &pitch, &yaw) == 2) { // Ensure both values are parsed
            Serial.print("Parsed Pitch: ");
            Serial.println(pitch);
            Serial.print("Parsed Yaw: ");
            Serial.println(yaw);

            // Map pitch and yaw to servo positions
            pitchPosition = map(constrain(pitch, -30, 30), -30, 30, 30, 150);
            yawPosition = map(constrain(yaw, -90, 90), -90, 90, 0, 180);

            // Write to servos
            yawServo.write(yawPosition);
            pitchServo.write(pitchPosition);

            // Debug servo positions
            Serial.print("Mapped Pitch Position: ");
            Serial.println(pitchPosition);
            Serial.print("Mapped Yaw Position: ");
            Serial.println(yawPosition);
        } else {
            Serial.println("Error parsing packet.");
        }
    }
} 
