/*  Leaky v0.3
 *
 *  NOTE: WILL NOT WORK WITHOUT THE MODIFIED MAX31856 LIBRARY
 *  This is the prototype code to be used for the Leaky Faucet Engine
 *
*/

//Includes and Defines--------------------------------------------------------

//Defining pins
const byte IGNITOR = 48;	//Ignitor
const byte WD1 = 46;		//Water deluge pins
const byte APV_AIR = 42;	//Air pneumatic valve pins
const byte APV_METH = 44;
const byte NMV = 40;		//Nitrogen main valve
const byte MPV = 38;		//Methanol purge valve

const byte sync_pin = 2;

#include <Q2HX711.h>
#include "Countdown.h"
#include "Firing.h"

//Functions-------------------------------------------------------------------
//void readLoadCells();
void waitForOn();
void safingSequence();

//Load Cells Info -------------------------------------------------------------
const int numLoadCells = 1;
const int DATA[] = { 43, 49, 45, 47 };
const int CLK_LC = 41;

float loadCellTransformationsA[] = { 1, 1, 1, 1 }; //Load Cell transformations. y = ax+b. Change a and b values based on calibrator data
float loadCellTransformationsB[] = { 0, 0, 0, 0 };
float loadCellData;


//Q2HX711 loadCells[] = {Q2HX711(DATA[0], CLK_LC)
                      //,Q2HX711(DATA[1], CLK_LC) 
                      //,Q2HX711(DATA[2], CLK_LC)
                      //,Q2HX711(DATA[3], CLK_LC)
                     // };

// Other variables
bool aborted = false;

Countdown countdown(60);
Firing firingSequence;

//Main Code-------------------------------------------------------------------
//Setup-----------------------------------------------------------------------
void setup() {
	Serial.begin(115200);
	Serial.println("Setup Initialized");
	solenoidInit();
  pinMode(sync_pin, OUTPUT);
  digitalWrite(sync_pin, LOW);

	waitForOn();
}

void loop() {
	//Abort the test if "A" is entered
	if (Serial.read() == 65) {
		countdown.Stop();
		firingSequence.Stop();
		aborted = true;

		Serial.println("TEST ABORTED");
		Serial.println("SYSTEM SAFED");
		Serial.println("YOU MAY NOW INPUT COMMANDS\n");
	}

	countdown.Update();

	if (countdown.IsDone()) {
		if (!firingSequence.IsStarted()) {
			firingSequence.Start();
		}

		firingSequence.Update();

		if (firingSequence.IsDone()) {
			safingSequence();
		}
	}

	if (aborted) {
		abortSequence();
	}

	output();
}

/*
void readLoadCells() {
	for (int i = 0; i < numLoadCells; i++) {
		//loadCellData = loadCells[i].read() * loadCellTransformationsA[i] + loadCellTransformationsB[i];
		loadCellData = loadCells[i].read();
		Serial.print(loadCellData);
		Serial.print(",");
	}
}
*/
//SYSTEM FUNCTIONS=================================================================

//Setup the output pins for the solenoids before any testing begins
void solenoidInit() {
	pinMode(WD1, OUTPUT);
	pinMode(APV_AIR, OUTPUT);
	pinMode(APV_METH, OUTPUT);
	pinMode(NMV, OUTPUT);
	pinMode(MPV, OUTPUT);
  pinMode(IGNITOR, OUTPUT);

	digitalWrite(APV_AIR, HIGH); //The relay is, for whatever reason, closed when off
	digitalWrite(APV_METH, HIGH); //litrally makes no sense to me but whatever
	digitalWrite(NMV, HIGH); //LOW = ON/RELAY POWERED
	digitalWrite(MPV, HIGH); //HIGH = OFF/RELAY UNPOWERED
	digitalWrite(WD1, HIGH);
  digitalWrite(IGNITOR, HIGH);
}

//while loop that begins the countdown when you send something in the serial port
void waitForOn() {
	Serial.println("WAITING TO START");
	Serial.println("S: START TEST");
	Serial.println("A: ABORT TEST");

	while (true) {
		if (Serial.available() > 0) {
			int command = Serial.read();
			if (command == 83) {
				Serial.println("COUNTDOWN START -------------");
				delay(20);
				Serial.println("COUNTDOWN START -------------");
				delay(20);
				Serial.println("COUNTDOWN START -------------");
				Serial.println("LC1, LC2, LC3, LC4, Countdown");

				digitalWrite(sync_pin, HIGH);
				countdown.Start();
				return;
			}
			else if (command == 65) {
				Serial.println("TEST ABORTED");
				Serial.println("SYSTEM SAFED");
				Serial.println("YOU MAY NOW INPUT COMMANDS\n");
        solenoidInit(); //de-energizes all solenoids
				aborted = true;
				return;
			}
			else {
				Serial.println("INVALID COMMAND");
			}
		}

		//readLoadCells();
		delay(50);
	}
}

//Stuff to display in the serial to reduce clutter
void output() {
	//readLoadCells();
	Serial.println(countdown.GetCount());
}

//Just says that the system is safed
void safingSequence() {
	Serial.println("SYSTEM SAFED");
	delay(200);
}

//Abort the test and allow commands from the serial
void abortSequence() {
	String command;

	delay(1000);
	Serial.println("**SYNTAX EX. OPEN MPV OR COMMAND VALVE**\n");
	Serial.println("---------- VALID COMMANDS ----------");
	Serial.println("OPEN: CAUSES SPECIFIED VALVE TO OPEN");
	Serial.println("CLOSE: CAUSES SPECIFIED VALVE TO CLOSE\n");
	Serial.println("---------- LIST OF VALVES ----------");
	Serial.println("MPV: METHANOL PURGE VALVE");
	Serial.println("MTAV: METHANOL AFT VALVE");
	Serial.println("NMV: NITROGEN MAIN VALVE");
	Serial.println("APV_AIR: AIR PNEUMATIC VALVE - AIR");
	Serial.println("APV_METH: AIR PNEUMATIC VALVE - METHANOL\n");

	while (true) {
		if (Serial.available() > 0) {
			command = Serial.readStringUntil('\n');
			break;
		}
	}

	if (command.equals("OPEN MPV")) {
		digitalWrite(MPV, HIGH);
		Serial.println("METHANOL PURGE VALVE OPENED");
	}
	if (command.equals("CLOSE MPV")) {
		digitalWrite(MPV, LOW);
		Serial.println("METHANOL PURGE VALVE CLOSED");
	}
	if (command.equals("OPEN NMV")) {
		digitalWrite(NMV, LOW);
		Serial.println("NITROGEN MAIN VALVE OPENED");
	}
	if (command.equals("CLOSE NMV")) {
		digitalWrite(NMV, HIGH);
		Serial.println("NITROGEN MAIN VALVE CLOSED");
	}
	if (command.equals("OPEN APV_AIR")) {
		digitalWrite(APV_AIR, LOW);
		Serial.println("AIR PNEUMATIC VALVE - AIR OPENED");
	}
	if (command.equals("CLOSE APV_AIR")) {
		digitalWrite(APV_AIR, HIGH);
		Serial.println("AIR PNEUMATIC VALVE - AIR CLOSED");
	}
	if (command.equals("OPEN APV_METH")) {
		digitalWrite(APV_METH, LOW);
		Serial.println("AIR PNEUMATIC VALVE - METHANOL OPENED");
	}
	if (command.equals("CLOSE APV_METH")) {
		digitalWrite(APV_METH, HIGH);
		Serial.println("AIR PNEUMATIC VALVE - METHANOL CLOSED");
	}
	if (command.equals("CLOSE WD1")) {
		digitalWrite(WD1, HIGH);
		Serial.println("WD1 CLOSED");
	}
	if (command.equals("OPEN WD1")) {
		digitalWrite(WD1, LOW);
		Serial.println("WD1 OPENED");
	}
	if (command.equals("IGN")) {
		digitalWrite(IGNITOR, LOW);
		Serial.println("IGNITOR ON");
	}
	if (command.equals("NGI")) {
		digitalWrite(IGNITOR, HIGH);
		Serial.println("IGNITOR OFF");
	}

	Serial.println();
}
