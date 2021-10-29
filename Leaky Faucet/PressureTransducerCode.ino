const int numTransducers = 7;
int pressurePins[numTransducers] = {A0, A1, A2, A3, A4, A5, A6};

/*The way this works is that the array stores the max pressure reading of each transducer.This corresponds to a reading of 1023 from analogRead().
* So, when we get the raw value, we multiply by max / 1023 to get the value in psi.
*/
float pressureMax[numTransducers] = {100, 200, 300, 400, 500, 600, 700};

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < numTransducers; i++) {
    Serial.print(pressureMax[i] * analogRead(pressurePins[i]) / 1023);
    Serial.print(', ');
  }
  Serial.println();
}
