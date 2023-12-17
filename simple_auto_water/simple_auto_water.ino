const int WATER_TANK_THRESHOLD = 50;
const int WATER_TANK_THRESHOLD_VERY_LOW = 70;
const int CROP_MOISTURE_THRESHOLD = 400;
const int NUM_MOISTURE_SENSORS = 8;
const int MOISTURE_SENSOR_PINS[NUM_MOISTURE_SENSORS] = {0, 1, 2, 3, 4, 5, 6, 7};
const int VALVE1_PIN = 8; // Replace with actual pin numbers
const int VALVE2_PIN = 9; // Replace with actual pin numbers
const int PUMP_PIN = 10;  // Replace with actual pin numbers
const int LAMP_MOIST_1_LOW_PIN = 11; // Replace with actual pin numbers
const int LAMP_MOIST_2_LOW_PIN = 12; // Replace with actual pin numbers
const int LAMP_WATER_LOW_PIN = 13; // Replace with actual pin numbers
const int LAMP_ERROR_PIN = 14; // Replace with actual pin numbers
const int BUZZER_PIN = 15; // Replace with actual pin numbers
const int TRIG_PIN = 16; // Replace with actual pin numbers
const int ECHO_PIN = 17; // Replace with actual pin numbers
int moistureLevels[NUM_MOISTURE_SENSORS];
int cropLeft, cropRight;
int prevCropLeft, prevCropRight; // Previous moisture levels
long duration, distance;
bool sensorFailed = false;
bool moistError_left = false;
bool moistError_right = false;
bool water_very_low = false;
unsigned long wateringStartTime = 0;
const unsigned long MAX_WATERING_TIME = 600000; // Maximum watering time in milliseconds (10 minutes)

void setup() {
  Serial.begin(9600);
  pinMode(VALVE1_PIN, OUTPUT);
  pinMode(VALVE2_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LAMP_MOIST_1_LOW_PIN, OUTPUT);
  pinMode(LAMP_MOIST_2_LOW_PIN, OUTPUT);
  pinMode(LAMP_WATER_LOW_PIN, OUTPUT);
  pinMode(LAMP_ERROR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  for (int i = 0; i < NUM_MOISTURE_SENSORS; i++) {
    pinMode(MOISTURE_SENSOR_PINS[i], INPUT);
  }
}

// Function to read moisture levels from all sensors and calculate crop averages
void readMoistureLevels() {
  for (int i = 0; i < NUM_MOISTURE_SENSORS; i++) {
    moistureLevels[i] = analogRead(MOISTURE_SENSOR_PINS[i]);
  }
  cropLeft = (moistureLevels[0] + moistureLevels[1] + moistureLevels[2] + moistureLevels[3]) / 4;
  cropRight = (moistureLevels[4] + moistureLevels[5] + moistureLevels[6] + moistureLevels[7]) / 4;
  Serial.print("mosist crop left: ");
  Serial.print(cropLeft);
  Serial.print("mosist crop right: ");
  Serial.print(cropRight);
}

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

  if (distance <= WATER_TANK_THRESHOLD || distance > WATER_TANK_THRESHOLD_VERY_LOW) {
    Serial.println("Water level is below the threshold");
    digitalWrite(LAMP_WATER_LOW_PIN, HIGH);
  } else {
    digitalWrite(LAMP_WATER_LOW_PIN, LOW);
  }
  if (distance <= WATER_TANK_THRESHOLD_VERY_LOW) {
    water_very_low = true;
    Serial.println("Water level is below the very low threshold. Performing automatic shutdown.");
  }
}

void checkMaximumWateringTime() {
  if (millis() - wateringStartTime >= MAX_WATERING_TIME) {
    digitalWrite(VALVE1_PIN, LOW);
    digitalWrite(VALVE2_PIN, LOW);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LAMP_MOIST_1_LOW_PIN, LOW);
    digitalWrite(LAMP_MOIST_2_LOW_PIN, LOW);
    Serial.println("Maximum watering time reached. Stopping watering.");
  }
}

void controlMoisture() {
  if (cropLeft <= CROP_MOISTURE_THRESHOLD) {
    digitalWrite(VALVE1_PIN, HIGH);
    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(LAMP_MOIST_1_LOW_PIN, HIGH);
    Serial.println("Left crop moisture level is low, opening the valve and pump.");
  } else {
    digitalWrite(VALVE1_PIN, LOW);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LAMP_MOIST_1_LOW_PIN, LOW);
  }
  // Check if moisture levels are not increasing as expected
  if (cropLeft <= prevCropLeft) {
    moistError_left= true;
    Serial.println("Moisture levels not increasing as expected for Left crop. Stopping watering.");
  }

  if (cropRight <= CROP_MOISTURE_THRESHOLD) {
    digitalWrite(VALVE2_PIN, HIGH);
    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(LAMP_MOIST_2_LOW_PIN, HIGH);
    Serial.println("Right crop moisture level is low, opening the valve and pump.");
  } else {
    digitalWrite(VALVE2_PIN, LOW);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LAMP_MOIST_2_LOW_PIN, LOW);
  }
  if (cropRight <= prevCropRight) {
    moistError_right = true;
    Serial.println("Moisture levels not increasing as expected for Right crop. Stopping watering.");
  }
}

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

void loop() {
  checkWaterLevel();
  if (water_very_low) {
    digitalWrite(LAMP_WATER_LOW_PIN, HIGH);
    digitalWrite(LAMP_ERROR_PIN, HIGH);
    delay(3000);
    digitalWrite(LAMP_ERROR_PIN, LOW);
    digitalWrite(LAMP_WATER_LOW_PIN, LOW);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(VALVE1_PIN, LOW);
    digitalWrite(VALVE2_PIN, LOW);
    delay(1000);
  } else {
    checkSensorFailures();
    if (sensorFailed){
      digitalWrite(LAMP_ERROR_PIN, HIGH);
    } else {
      digitalWrite(LAMP_ERROR_PIN, LOW);
      readMoistureLevels() ;
      checkMaximumWateringTime(); // Check for maximum watering time
      controlMoisture();
      if (moistError_left){
        digitalWrite(LAMP_MOIST_1_LOW_PIN, HIGH);
        digitalWrite(LAMP_ERROR_PIN, HIGH);
        delay(3000);
        digitalWrite(LAMP_ERROR_PIN, LOW);
        digitalWrite(LAMP_MOIST_1_LOW_PIN, LOW);
        delay(1000);
      if (moistError_right){
        digitalWrite(LAMP_MOIST_2_LOW_PIN, HIGH);
        digitalWrite(LAMP_ERROR_PIN, HIGH);
        delay(3000);
        digitalWrite(LAMP_ERROR_PIN, LOW);
        digitalWrite(LAMP_MOIST_2_LOW_PIN, LOW);
        delay(1000);
      } else {
        digitalWrite(LAMP_ERROR_PIN, LOW);
        delay(10000);
      }
    }
  }
}
}