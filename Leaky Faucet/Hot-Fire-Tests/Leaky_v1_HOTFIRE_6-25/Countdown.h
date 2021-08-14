#pragma once

class Countdown {
public:
	Countdown(int count);
	void Reset(int count);
	void Update();

	void Start();
	void Stop();
	bool IsDone() const;
  int GetCount() const;

private:
	int count;
	bool running;
	unsigned long previous_millis;
};

Countdown::Countdown(int count) : count(count) {
	running = false;
	previous_millis = 0;
}

void Countdown::Reset(int count) {
	this->count = count;
	running = false;
	previous_millis = 0;
}

void Countdown::Update() {
	if (running && count > 0) {
		unsigned long current_millis = millis();

		if (current_millis - previous_millis > 999) {
			count--;
			previous_millis = current_millis;
		}

		if (count == 35) {
			digitalWrite(MPV, LOW);
			Serial.println(" MPV: ENERGIZED ");
		}
		else if (count == 30) {
			digitalWrite(NMV, LOW);
			Serial.println(" NMV: ENERGIZED ");
		}
		else if (count == 5) {
			digitalWrite(WD1, LOW);
			//digitalWrite(WD2, LOW);
			Serial.println(" WATER DELUGE ACTIVATED ");
		}
	}
}

void Countdown::Start() {
	running = true;
	previous_millis = millis();
}

void Countdown::Stop() {
	running = false;
}

bool Countdown::IsDone() const {
	return count == 0 && running;
}

int Countdown::GetCount() const {
  return count;
}
