const int numTransducers = 8;
int pressurePins[numTransducers] = {A0, A1, A2, A3, A4, A5, A6, A7};

/*The way this works is that the array stores the max pressure reading of each transducer.This corresponds to a reading of 1023 from analogRead().
* So, when we get the raw value, we multiply by max / 1023 to get the value in psi.
*/
float pressureMax[numTransducers] = {0/1023, 500/1023, 500/1023, 0/1023, 3000/1023, 0/1023, 3000/1023};

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < numTransducers; i++) {
      if (i == 7) {
          float pressure = 3000 * (5 * read / 1023 - 0.5) / 5;
          Serial.print(pressure);
      }
      else {
          Serial.print(pressureMax[i] * analogRead(pressurePins[i]));
          Serial.print(', ');
      }
  }
  Serial.println();
}
