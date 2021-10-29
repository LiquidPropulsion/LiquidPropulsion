int numDucers = 7;
int pressurePins[] = {A0, A1, A2, A3, A4, A5, A6};

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < numDucers; i++) {
    Serial.print(analogRead(pressurePins[i]));
    Serial.print(',');
  }
  Serial.println();
}
