const int WATER_TANK_THRESHOLD = 50;
const int CROP_MOISTURE_THRESHOLD = 300;
const int NUM_MOISTURE_SENSORS = 8;
const int MOISTURE_SENSOR_PINS[NUM_MOISTURE_SENSORS] = {0, 1, 2, 3, 4, 5, 6, 7};
const int VALVE1_PIN = 43;
const int VALVE2_PIN = 45;
const int PUMP_PIN = 47;
const int LAMP_MOIST_1_LOW_PIN = 49;
const int LAMP_MOIST_2_LOW_PIN = 51;
const int BUZZER_PIN = 53;
const int TRIG_PIN = 12;
const int ECHO_PIN = 13;
int moistureLevels[NUM_MOISTURE_SENSORS];
int cropLeft, cropRight;
long duration, distance;

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

void loop() {
  checkWaterLevel();
  readMoistureLevels();
  controlMoisture();
  delay(1000);
}