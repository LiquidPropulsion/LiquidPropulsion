/*  Leaky v0.3
 *
 *  NOTE: WILL NOT WORK WITHOUT THE MODIFIED MAX31856 LIBRARY
 *  This is the prototype code to be used for the Leaky Faucet Engine
 *
*/

//Includes and Defines--------------------------------------------------------
//Camera
const byte XT3 = 50; //output pin for camera trigger
const byte testLED = 52; //output pin for led in camera

//Defining pins
const byte IGNITOR = 46; //ignitor
const byte WD1 = 44; //Water deluge pins
const byte WD2 = 48;
const byte APV_AIR = 40; //Air pneumatic valve pins
const byte APV_METH = 42;
const byte NMV = 38; //Nitrogen main valve
const byte MPV = 36; //Methanol purge valve

//#include <Adafruit_MAX31856.h>
#include <Wire.h>
//#include <Q2HX711.h>
#include "Countdown.h"
#include "Firing.h"

//Functions-------------------------------------------------------------------
//void thermosInit();
//void readThermos();
//void readLoadCells();
void waitForOn();
void safingSequence();

//Global Variables------------------------------------------------------------

//Thermocouples info -----------------------------------------------------------

//READ THERMOCOUPLES EVERY SECOND OR TWO IN ABORT
const byte numThermos = 2;                 //Change based on thermocouples
const int CS[] = { 10, 9, 8, 7 };     //Also change based on thermocouples
const byte DI = 11;
const byte DL = 12;
const byte CLK_T = 13;

//Adafruit_MAX31856 thermos[] = {Adafruit_MAX31856(CS[0],DI,DL,CLK_T)
//                               ,Adafruit_MAX31856(CS[1],DI,DL,CLK_T)
//                               //,Adafruit_MAX31856(CS[2],DI,DL,CLK_T)
//                               //,Adafruit_MAX31856(CS[3],DI,DL,CLK_T)
//                               };

//Load Cells Info -------------------------------------------------------------
const int numLoadCells = 4;
const int DATA[] = { 43, 49, 45, 47 };
const int CLK_LC = 41;

float loadCellTransformationsA[] = { 1, 1, 1, 1 }; //Load Cell transformations. y = ax+b. Change a and b values based on calibrator data
float loadCellTransformationsB[] = { 0, 0, 0, 0 };
float loadCellData;


//Q2HX711 loadCells[] = {Q2HX711(DATA[0], CLK_LC)
//                      ,Q2HX711(DATA[1], CLK_LC) 
//                      ,Q2HX711(DATA[2], CLK_LC)
//                      ,Q2HX711(DATA[3], CLK_LC)
//                      };

//Interrupt
const byte onOffPin = 3;  //Interrupt pin
int switchVal;

// Other variables
bool aborted = false;
String command;

Countdown countdown(60);
Firing firingSequence;

//Main Code-------------------------------------------------------------------
//Setup-----------------------------------------------------------------------
void setup() {
	Serial.begin(115200);
	Serial.println("Setup Initialized");
	//thermosInit();
	solenoidInit();
	cameraInit();

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
//INSTRUMENTATION FUNCTIONS========================================================
void thermosInit(void){
  for(int i = 0; i < numThermos; i++){
	thermos[i].begin();
	//thermos[i].config();
	thermos[i].setThermocoupleType(MAX31856_TCTYPE_K);
	// Thermocouple needs to be a K type to work. Otherwise, Code can't work.
	Serial.print("Thermocouple type: ");
	switch ( thermos[i].getThermocoupleType() ) {
	  case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
	  case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
	  case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
	  case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
	  case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
	  case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
	  case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
	  case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
	  case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
	  case MAX31856_VMODE_G32: Serial.println("Voltage x8 Gain mode"); break;
	  default: Serial.println("Unknown"); break;
	 }
   }
}

//==================================================
// READ THERMOS

void readThermos(){
  for(int i = 0; i < numThermos; i++){
	Serial.print("Thermocouple ");
	Serial.print(i+1);
	//Serial.print(" - Cold Junction Temp: ");
	//Serial.print(thermos[i].readCJTemperature());
	Serial.print(thermos[i].readThermocoupleTemperature());

	//Check and print any faults
	uint8_t fault = thermos[i].readFault();
	if (fault) {
	  if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
	  if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
	  if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
	  if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
	  if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
	  if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
	  if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
	  if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
	}
  }
}
*/

//void readLoadCells() {
//	//Getting Load Cell data. Comment out as necessary
//	for (int i = 0; i < numLoadCells; i++) {
//		//loadCellData = loadCells[i].read() * loadCellTransformationsA[i] + loadCellTransformationsB[i];
//		loadCellData = loadCells[i].read();
//		Serial.print(loadCellData);
//		Serial.print(",");
//	}
//}

//SYSTEM FUNCTIONS=================================================================

//Setup the output pins for the camera before any testing begins
void cameraInit() {
	pinMode(XT3, OUTPUT);
	pinMode(testLED, OUTPUT);
	digitalWrite(XT3, HIGH);
	digitalWrite(testLED, HIGH);
}

//Setup the output pins for the solenoids before any testing begins
void solenoidInit() {
	pinMode(WD1, OUTPUT);
	pinMode(WD2, OUTPUT);
	pinMode(APV_AIR, OUTPUT);
	pinMode(APV_METH, OUTPUT);
	pinMode(NMV, OUTPUT);
	pinMode(MPV, OUTPUT);


	digitalWrite(APV_AIR, HIGH); //The relay is, for whatever reason, closed when off
	digitalWrite(APV_METH, HIGH); //litrally makes no sense to me but whatever
	digitalWrite(NMV, HIGH); //LOW = ON/RELAY POWERED
	digitalWrite(MPV, HIGH); //HIGH = OFF/RELAY UNPOWERED
	digitalWrite(WD1, HIGH);
	digitalWrite(WD2, HIGH);
}

//while loop that begins the countdown when you send something in the serial port
void waitForOn() {
	Serial.println("WAITING TO START");
	Serial.println("S: START TEST");
	Serial.println("A: ABORT TEST");

	while (true) {
		if (Serial.available() > 0) {
			if (Serial.read() == 83) {
				Serial.println("COUNTDOWN START -------------");
				delay(20);
				Serial.println("COUNTDOWN START -------------");
				delay(20);
				Serial.println("COUNTDOWN START -------------");
				Serial.println("LC1, LC2, LC3, LC4, Countdown");

				countdown.Start();
				return;
			}
			else if (Serial.read() == 65) {
				Serial.println("TEST ABORTED");
				Serial.println("SYSTEM SAFED");
				Serial.println("YOU MAY NOW INPUT COMMANDS\n");
				aborted = true;
				return;
			}
			else {
				Serial.println("INVALID COMMAND");
			}
		}
		//readThermos();
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
	if (command.equals("test")) {
		digitalWrite(XT3, LOW);
		digitalWrite(testLED, LOW);
		delay(1000);
		digitalWrite(XT3, HIGH);
		digitalWrite(testLED, HIGH);
	}

	Serial.println();
}
