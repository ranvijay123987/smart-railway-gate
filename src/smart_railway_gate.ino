/*
 * Smart IoT Automated Railway Crossing & Gate Management System
 * Target Board: Arduino Uno / Nano
 * Components: HC-SR04 Ultrasonic Sensor, Servo Motor, Warning Indicators
 */

#include <Servo.h>

// Pin Definitions
const int TRIG_PIN  = 5;   // Ultrasonic Sensor Trigger
const int ECHO_PIN  = 6;   // Ultrasonic Sensor Echo
const int SERVO_PIN = 7;   // Servo Motor PWM Signal
const int ALERT_LED = 10;  // High-intensity Warning LED

// Distance & Motion Parameters
const int DETECTION_THRESHOLD_CM = 10;  // Trigger barrier when train is within threshold
const int GATE_CLOSED_ANGLE      = 0;   // Barrier lowered (crossing closed)
const int GATE_OPEN_ANGLE        = 150; // Barrier raised (crossing open)
const int CLEARANCE_DELAY_MS     = 5000;// Hold gate closed for train clearance

Servo barrierServo;
long distanceBuffer[3];

void setup() {
    Serial.begin(9600);
    
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(ALERT_LED, OUTPUT);
    
    // Initialize barrier in open position
    barrierServo.attach(SERVO_PIN);
    barrierServo.write(GATE_OPEN_ANGLE);
    delay(500);
    barrierServo.detach();
    
    Serial.println(F("[SYSTEM ONLINE] Smart Railway Crossing Controller Initialized."));
}

long measureDistanceCm() {
    digitalWrite(ALERT_LED, HIGH);
    
    // Trigger ultrasonic burst
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(15);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH);
    long distance = (duration / 2) / 29.1; // Speed of sound conversion to cm
    return distance;
}

void loop() {
    // 3-sample moving average filter to reject false ultrasonic spikes
    for (int i = 0; i < 3; i++) {
        distanceBuffer[i] = measureDistanceCm();
        delay(10);
    }
    
    long filteredDistance = (distanceBuffer[0] + distanceBuffer[1] + distanceBuffer[2]) / 3;
    
    Serial.print(F("Distance (cm): "));
    Serial.println(filteredDistance);
    
    // Trigger barrier sequence when approaching object detected
    if (filteredDistance > 0 && filteredDistance < DETECTION_THRESHOLD_CM) {
        Serial.println(F("[ALERT] Approaching vehicle/train detected! Closing crossing gate..."));
        
        barrierServo.attach(SERVO_PIN);
        barrierServo.write(GATE_CLOSED_ANGLE);
        
        // Flash LED warning during clearance period
        for (int t = 0; t < CLEARANCE_DELAY_MS / 200; t++) {
            digitalWrite(ALERT_LED, HIGH);
            delay(100);
            digitalWrite(ALERT_LED, LOW);
            delay(100);
        }
        
        Serial.println(F("[INFO] Clearance complete. Opening crossing gate..."));
        barrierServo.write(GATE_OPEN_ANGLE);
        delay(1000);
        barrierServo.detach();
    }
    
    delay(50);
}
