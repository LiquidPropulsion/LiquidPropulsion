const int numTransducers = 8;
int pressurePins[numTransducers] = {A0, A1, A2, A3, A4, A5, A6, A7};
const byte sync_pin = 2;
bool print_time = false;
unsigned long start_millis;

/*The way this works is that the array stores the max pressure reading of each transducer.This corresponds to a reading of 1023 from analogRead().
* So, when we get the raw value, we multiply by max / 1023 to get the value in psi.
*/
float pressureMax[numTransducers] = {0/1023, 500/1023, 500/1023, 0/1023, 3000/1023, 0/1023, 3000/1023};

void setup() {
  Serial.begin(115200);
  pinMode(sync_pin, INPUT);
}

void loop() {
  if (digitalRead(sync_pin) == HIGH) {
    print_time = true;
    start_millis = millis();
  }

  if (print_time) {
    Serial.print(millis() - start_millis);
    Serial.print(',');
  }
  
  for (int i = 0; i < numTransducers; i++) {
      if (i == 7) {
          float pressure = 3000 * (5 * analogRead(pressurePins[i]) / 1023 - 0.5) / 5;
          Serial.print(pressure);
      }
      else {
          Serial.print(pressureMax[i] * analogRead(pressurePins[i]));
          Serial.print(',');
      }
  }
  Serial.println();
}
