// E2S-W24 Inductive Proximity Sensor Demo (2 sensors)
//
// Sensor wiring (each):
//   Brown  -> 12-24V external supply
//   Blue   -> GND (common with Arduino GND)
//   Black  -> SENSOR_PIN
//

const int SENSOR_PIN_1 = 2;//End
const int SENSOR_PIN_2 = 3;//Home

void setup() {
  pinMode(SENSOR_PIN_1, INPUT_PULLUP);
  pinMode(SENSOR_PIN_2, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("E2S-W24 proximity sensor demo");
}

void loop() {
  bool sensor1 = (digitalRead(SENSOR_PIN_1) == LOW);// low means detected, high means clear
  bool sensor2 = (digitalRead(SENSOR_PIN_2) == LOW);

  Serial.print("Sensor(End) 1: ");
  Serial.print(sensor1 ? "detected" : "clear");
  Serial.print(" | Sensor(Home) 2: ");
  Serial.println(sensor2 ? "detected" : "clear");

  delay(100);
}
