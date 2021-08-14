#pragma once

class Firing {
public:
	Firing();
	void Update();

	void Start();
	void Stop();
	bool IsDone() const;
	bool IsStarted() const;

private:
	unsigned long previous_millis;
	bool done, started;
	int stage;
};

Firing::Firing() {
	previous_millis = 0;
	done = false;
	started = false;
	stage = 0;
}

void Firing::Update() {
	if (!done) {
		unsigned long current_millis = millis();

		//stage switch (firing sequence)
		//case 0: open nitrogen valve (confirm LOW = open)
		//case 1: open methanol air pneumatic valve (confirm LOW = open)
		//case 2: close methanol air pneumatic valve (confirm HIGH = closed)
		//case 3: open methanol purge valve (confirm HIGH = open) AND close nitrogen main valve (confirm HIGH = closed)
		//case 4: confirm test has ended AND triggers safing

		switch (stage) {
		case 0:
			digitalWrite(XT3, LOW); //test turning camera on
			digitalWrite(testLED, LOW);
			digitalWrite(APV_METH, LOW);
			Serial.println(" METHANOL APV: ENERGIZED ");
			stage++;
			break;
		case 1:
			if (current_millis - previous_millis > 600) { //Remmebr to change that number if necessary
				digitalWrite(APV_AIR, LOW);
				Serial.println(" AIR APV: ENERGIZED");
				digitalWrite(IGNITOR, LOW);
				Serial.println(" IGNITION ");
				stage++;
			}
			break;
		case 2:
			if (current_millis - previous_millis > 850) {
				digitalWrite(APV_METH, HIGH);
				digitalWrite(APV_AIR, HIGH);
				digitalWrite(XT3, HIGH); //test turning camera off
				digitalWrite(IGNITOR, HIGH);
				Serial.println(" METHANOL APV: DE-ENERGIZED ");
				Serial.println(" AIR APV: DE-ENERGIZED ");
				stage++;
			}
			break;
		case 3:
			if (current_millis - previous_millis > 4850) {
				digitalWrite(MPV, HIGH);
				Serial.println(" MPV: DE-ENERGIZED ");
				digitalWrite(WD1, HIGH);
				//digitalWrite(WD2, HIGH);
				Serial.println(" WATER DELUGE DE-ACTIVATED ");
				stage++;
			}
			break;
		case 4:
			if (current_millis - previous_millis > 7850) {
				digitalWrite(NMV, HIGH);
				Serial.println(" NMV: DE-ENERGIZED ");
				stage++;
			}
     break;
		case 5:
			if (current_millis - previous_millis > 8850) {
				Serial.print(" TEST END ");
				done = true;
			}
			break;
		}
	}
}

void Firing::Start() {
	done = false;
	started = true;
	previous_millis = millis();
	stage = 0;
}

void Firing::Stop() {
	done = true;
	started = false;
}

bool Firing::IsDone() const {
	return done;
}

bool Firing::IsStarted() const {
  return started;
}
