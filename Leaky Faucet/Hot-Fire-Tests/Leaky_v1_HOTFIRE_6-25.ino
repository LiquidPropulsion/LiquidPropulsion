/*  Leaky v0.3
 *   
 *  NOTE: WILL NOT WORK WITHOUT THE MODIFIED MAX31856 LIBRARY
 *  This is the prototype code to be used for the Leaky Faucet Engine
 *  
*/

//Includes and Defines--------------------------------------------------------
#include <Adafruit_MAX31856.h>
#include <Wire.h>
#include <Q2HX711.h>

//Functions-------------------------------------------------------------------
//void thermosInit();
//void readThermos();
void readLoadCells();
void waitForOn();
void countdown();
void safingSequence();

//Global Variables------------------------------------------------------------

//Camera
const byte XT3 = 50; //output pin for camera trigger
const byte testLED = 52; //output pin for led in camera 

//Thermocouples info -----------------------------------------------------------

//READ THERMOCOUPLES EVERY SECOND OR TWO IN ABORT
const byte numThermos = 2;                 //Change based on thermocouples
const int CS[] = {10, 9, 8, 7};     //Also change based on thermocouples
const byte DI = 11;
const byte DL = 12;
const byte CLK_T = 13;

Adafruit_MAX31856 thermos[] = {Adafruit_MAX31856(CS[0],DI,DL,CLK_T)
                               ,Adafruit_MAX31856(CS[1],DI,DL,CLK_T)
                               //,Adafruit_MAX31856(CS[2],DI,DL,CLK_T)
                               //,Adafruit_MAX31856(CS[3],DI,DL,CLK_T)
                               };

//Load Cells Info -------------------------------------------------------------
const int numLoadCells = 4;
const int DATA[] = {43, 49, 45, 47};
const int CLK_LC = 41;

float loadCellTransformationsA[] = {1, 1, 1, 1}; //Load Cell transformations. y = ax+b. Change a and b values based on calibrator data
float loadCellTransformationsB[] = {0, 0, 0, 0};
float loadCellData; 


Q2HX711 loadCells[] = {Q2HX711(DATA[0], CLK_LC)
                      ,Q2HX711(DATA[1], CLK_LC) 
                      ,Q2HX711(DATA[2], CLK_LC)
                      ,Q2HX711(DATA[3], CLK_LC)
                      };


//Solenoids ------------------------------------------------------------------
//Defining pins
const byte IGNITOR = 46; //ignitor
const byte WD1 = 44; //Water deluge pins
const byte WD2 = 48;
const byte APV_AIR = 40; //Air pneumatic valve pins
const byte APV_METH = 42;
const byte NMV = 38; //Nitrogen main valve
const byte MPV = 36; //Methanol purge valve

//Interrupt
const byte onOffPin = 3;  //Interrupt pin
int switchVal;

//millis
unsigned long currentMillis = 0;
unsigned long previousMillisCount = 0;
unsigned long previousMillisFire = 0;

// Other variables
int status = 0; //code-state flag
int count = -60; //initial countdown 
int stage = 0; //firing sequence staging
int condition = 0; //abort status switch case
String command;

//Main Code-------------------------------------------------------------------
//Setup-----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Setup Initialized");
  //thermosInit();
  solenoidInit(); 
  cameraInit();
  Serial.println("Beginning Testing");
}

//Main Loop-------------------------------------------------------------------
//s: start test
//a: abort test
void loop() {
  
  //status switch (main loop)
  //case 0: serial input triggers sequence
  //case 1: perform a 5 second countdown to trigger firing sequence
  //case 2: firing sequence triggers safing when done
  //case 3: safing sequence (end)
  //case 4: abort sequence (alt ending) [AT ABORT SIGNAL ALLOW FOR SERIAL COMMANDS]
  
  //stage switch (firing sequence)
  //case 0: open nitrogen valve (confirm LOW = open)
  //case 1: open methanol air pneumatic valve (confirm LOW = open)
  //case 2: close methanol air pneumatic valve (confirm HIGH = closed)
  //case 3: open methanol purge valve (confirm HIGH = open) AND close nitrogen main valve (confirm HIGH = closed)
  //case 4: confirm test has ended AND triggers safing
  
  currentMillis = millis();

  //abort the test if "a" is entered
  if (Serial.read() == 65) {
    status = 4;
  }
  
  switch(status) {
    case 0:
    waitForOn();
    break;
    case 1:
    delay(100);
    if (currentMillis - previousMillisCount > 999) {
      countdown();
    }
    break;
    case 2:
    switch(stage) {
      case 0:
      digitalWrite(XT3, LOW); //test turning camera on
      digitalWrite(testLED, LOW); 
      digitalWrite(APV_METH, LOW);
      Serial.println(" METHANOL APV: ENERGIZED ");
      stage++;
      break;
      case 1:
      if (currentMillis - previousMillisFire > 600) { //Remmebr to change that number if necessary
        digitalWrite(APV_AIR, LOW);
        Serial.println(" AIR APV: ENERGIZED");
        digitalWrite(IGNITOR, LOW);
        Serial.println(" IGNITION ");
        stage++;
      }
      break;
      case 2:
      if (currentMillis - previousMillisFire > 850) { //850
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
      if (currentMillis - previousMillisFire > 4850) {
        digitalWrite(MPV, HIGH);
        Serial.println(" MPV: DE-ENERGIZED ");
        digitalWrite(WD1, HIGH);
        //digitalWrite(WD2, HIGH);
        Serial.println(" WATER DELUGE DE-ACTIVATED ");
        stage++;
      }
      break;
      case 4: 
      if (currentMillis - previousMillisFire > 7850) {
        digitalWrite(NMV, HIGH);
        Serial.println(" NMV: DE-ENERGIZED ");
        stage++;
      }
      case 5:
      if (currentMillis - previousMillisFire > 8850) {
        Serial.print(" TEST END ");
        status++;
      }
      break;   
    }
    break;
    case 3:
    safingSequence();
    break;
    case 4:
    abortSequence();
    break;
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
// READ LOAD CELLS

void readLoadCells(){
  //Getting Load Cell data. Comment out as necessary
  for (int i = 0; i < numLoadCells; i++){
    //Getting Data
    //loadCellData = loadCells[i].read() * loadCellTransformationsA[i] + loadCellTransformationsB[i];
    loadCellData = loadCells[i].read();
    //Creating serial output 
    Serial.print(loadCellData);
    Serial.print(",");
  }
} 

//SYSTEM FUNCTIONS=================================================================

//setup the output pins for the camera before any testing begins
void cameraInit() {
  pinMode(XT3, OUTPUT);
  pinMode(testLED, OUTPUT);
  digitalWrite(XT3, HIGH);
  digitalWrite(testLED, HIGH);
  
}

//setup the output pins for the solenoids before any testing begins
void solenoidInit(){
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
void waitForOn(){
  Serial.println(" WAITING TO START ");
  Serial.println(" S: START TEST ");
  Serial.println(" A: ABORT TEST ");
  bool waitingForOn = true;
  while(waitingForOn){
    if (Serial.available() > 0) {
      if (Serial.read() == 83) {
        waitingForOn = false;
        Serial.println("COUNTDOWN START -------------");
        delay(20);
        Serial.println("COUNTDOWN START -------------");
        delay(20);
        Serial.println("COUNTDOWN START -------------");
        previousMillisCount = millis(); 
        status++;
        Serial.println("LC1, LC2, LC3, LC4, millis, prevMill, Countdown");
      }
      else {
        Serial.println(" INVALID COMMAND ");
      }
    }
    //readThermos();
    //readLoadCells();
    delay(50);
  }
  
}

//stuff to display in the serial to reduce clutter
void output() {
  readLoadCells();
  Serial.print(currentMillis);
  Serial.print(",");
  Serial.print(previousMillisFire);
  Serial.print(",");
  Serial.println(count);
}
//countdown is executed once per second in the countdown phase of the main loop
void countdown(){
  count++;
  previousMillisCount = currentMillis;
  if (count == -35) {
    digitalWrite(MPV, LOW);
    Serial.println(" MPV: ENERGIZED ");
  }
  if (count == -30) {
    digitalWrite(NMV, LOW);
    Serial.println(" NMV: ENERGIZED ");
  }
  if (count == -5) {
    digitalWrite(WD1, LOW);
    //digitalWrite(WD2, LOW);
    Serial.println(" WATER DELUGE ACTIVATED ");
  }
  
  // Engaging countdown sequence
  if (count == 1) {
    status++; 
    previousMillisFire = millis();
  }
}

//just says that the system is safed
void safingSequence() {
  Serial.println(" SYSTEM SAFED ");
  delay(200);
}

//abort the test and allow commands from the serial
void abortSequence() {
  switch(condition) {
    case 0: 
    Serial.println(" TEST ABORTED ");
    Serial.println(" SAFING SYSTEM ");
    Serial.println(" YOU MAY NOW INPUT COMMANDS ");
    Serial.println();
    condition++;
    break;
    case 1:
    delay(1000);
    Serial.println(" **SYNTAX EX. OPEN MPV OR COMMAND VALVE** ");
    Serial.println();
    Serial.println(" ---------- VALID COMMANDS ---------- ");
    Serial.println(" OPEN: CAUSES SPECIFIED VALVE TO OPEN ");
    Serial.println(" CLOSE: CAUSES SPECIFIED VALVE TO CLOSE ");
    Serial.println();
    Serial.println(" ---------- LIST OF VALVES ----------" );
    Serial.println(" MPV: METHANOL PURGE VALVE ");
    Serial.println(" MTAV: METHANOL AFT VALVE ");
    Serial.println(" NMV: NITROGEN MAIN VALVE ");
    Serial.println(" APV_AIR: AIR PNEUMATIC VALVE - AIR ");
    Serial.println(" APV_METH: AIR PNEUMATIC VALVE - METHANOL ");
    Serial.println();
    bool waitingForCommand = true;
    while (waitingForCommand) {
      if (Serial.available() > 0) {
      waitingForCommand = false;
      }
    }
    command = Serial.readStringUntil('\n');
    if (command.equals("OPEN MPV")) {
      digitalWrite(MPV,HIGH);
      Serial.println("METHANOL PURGE VALVE OPENED");
    }
    if (command.equals("CLOSE MPV")) {
      digitalWrite(MPV,LOW);
      Serial.println("METHANOL PURGE VALVE CLOSED");
    }
    if (command.equals("OPEN NMV")) {
      digitalWrite(NMV,LOW);
      Serial.println("NITROGEN MAIN VALVE OPENED");
    }
    if (command.equals("CLOSE NMV")) {
      digitalWrite(NMV,HIGH);
      Serial.println("NITROGEN MAIN VALVE CLOSED");
    }
    if (command.equals("OPEN APV_AIR")) {
      digitalWrite(APV_AIR,LOW);
      Serial.println("AIR PNEUMATIC VALVE - AIR OPENED");
    }
    if (command.equals("CLOSE APV_AIR")) {
      digitalWrite(APV_AIR,HIGH);
      Serial.println("AIR PNEUMATIC VALVE - AIR CLOSED");
    }
    if (command.equals("OPEN APV_METH")) {
      digitalWrite(APV_METH,LOW);
      Serial.println("AIR PNEUMATIC VALVE - METHANOL OPENED");
    }
    if (command.equals("CLOSE APV_METH")) {
      digitalWrite(APV_METH,HIGH);
      Serial.println("AIR PNEUMATIC VALVE - METHANOL CLOSED");
      
    }
    if (command.equals("CLOSE WD1")) {
      digitalWrite(WD1,HIGH);
      Serial.println("WD1 CLOSED");
    }
    if (command.equals("OPEN WD1")) {
      digitalWrite(WD1,LOW);
      Serial.println("WD1 OPEN");
    }
    if (command.equals("IGN")) {
      digitalWrite(IGNITOR,LOW);
      Serial.println("IGNITOR ON");
    }
    if (command.equals("NGI")) {
      digitalWrite(IGNITOR,HIGH);
      Serial.println("IGNITOR OFF");
    }
    if (command.equals("test")) {
      digitalWrite(XT3,LOW);
      digitalWrite(testLED, LOW);
      delay(1000);
      digitalWrite(XT3,HIGH);
      digitalWrite(testLED, HIGH);
    }
    Serial.println();
    break;
    }
}
