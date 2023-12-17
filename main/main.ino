#include <avr/sleep.h>

const int WATER_TANK_THRESHOLD = 50;
const int WATER_TANK_THRESHOLD_VERY_LOW = 70;
const int CROP_MOISTURE_THRESHOLD = 300;
const int NUM_MOISTURE_SENSORS = 8;
const int MOISTURE_SENSOR_PINS[NUM_MOISTURE_SENSORS] = {0, 1, 2, 3, 4, 5, 6, 7};
const int VALVE1_PIN = 41;
const int VALVE2_PIN = 43;
const int PUMP_PIN = 45;
const int LAMP_MOIST_1_LOW_PIN = 47;
const int LAMP_MOIST_2_LOW_PIN = 49;
const int LAMP_WATER_LOW_PIN = 51;
const int BUZZER_PIN = 53;
const int TRIG_PIN = 12;
const int ECHO_PIN = 13;
int moistureLevels[NUM_MOISTURE_SENSORS];
int cropLeft, cropRight;
long duration, distance;
bool sensorFailed = false;

void setup() {
  Serial.begin(9600);
  pinMode(VALVE1_PIN, OUTPUT);
  pinMode(VALVE2_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LAMP_MOIST_1_LOW_PIN, OUTPUT);
  pinMode(LAMP_MOIST_2_LOW_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  for (int i = 0; i < NUM_MOISTURE_SENSORS; i++) {
    pinMode(MOISTURE_SENSOR_PINS[i], INPUT);
  }
  sensorFailed = false;
}

// Function to check water level and activate buzzer if it's low
void checkWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.0343 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance <= WATER_TANK_THRESHOLD) {
    Serial.println("Water level is below the threshold");
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// Function to read moisture levels from all sensors and calculate crop averages
void readMoistureLevels() {
  for (int i = 0; i < NUM_MOISTURE_SENSORS; i++) {
    moistureLevels[i] = analogRead(MOISTURE_SENSOR_PINS[i]);
  }
  cropLeft = (moistureLevels[0] + moistureLevels[1] + moistureLevels[2] + moistureLevels[3]) / 4;
  cropRight = (moistureLevels[4] + moistureLevels[5] + moistureLevels[6] + moistureLevels[7]) / 4;
}

// Function to control moisture levels and valves
void controlMoisture() {
  if (cropLeft <= CROP_MOISTURE_THRESHOLD) {
    digitalWrite(VALVE1_PIN, HIGH);
    digitalWrite(PUMP_PIN, HIGH);
    Serial.println("Left crop moisture level is low, opening the valve and pump.");
  } else {
    digitalWrite(VALVE1_PIN, LOW);
  }

  if (cropRight <= CROP_MOISTURE_THRESHOLD) {
    digitalWrite(VALVE2_PIN, HIGH);
    digitalWrite(PUMP_PIN, HIGH);
    Serial.println("Right crop moisture level is low, opening the valve and pump.");
  } else {
    digitalWrite(VALVE2_PIN, LOW);
  }
}

// Function to check if any of the moisture sensors has failed
void checkSensorFailures() {
  for (int i = 0; i < NUM_MOISTURE_SENSORS; i++) {
    moistureLevels[i] = analogRead(MOISTURE_SENSOR_PINS[i]);
    if (moistureLevels[i] == 0 || moistureLevels[i] > 1023) {
      sensorFailed = true;
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.println(" has failed.");
    }
  }
}

// Function to implement automatic shutdown when water tank is empty
void automaticShutdown() {
  checkWaterLevel();
  if (distance <= WATER_TANK_THRESHOLD_VERY_LOW) {
    Serial.println("Water level is below the very low threshold. Performing automatic shutdown.");
    digitalWrite(VALVE1_PIN, LOW);
    digitalWrite(VALVE2_PIN, LOW);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(3000); // Buzz for 3 seconds
    digitalWrite(BUZZER_PIN, LOW);
    enterSleepMode();
  } else {
    sleep_disable();
  }
}

// Function to enter sleep mode
void enterSleepMode() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
}

void loop() {
  checkSensorFailures();
  if (sensorFailed) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    automaticShutdown();
    checkWaterLevel();
    readMoistureLevels();
    controlMoisture();
  }
  delay(1000);
}
