//#include "RF_communication.h"
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <stdlib.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define PIN_TOUCH_SENSOR 4
#define PIN_JOYSTICK_BUTTON 5

RF24 radio(7, 8);  //CE->PIN7, CSN->PIN8

//WIRING FOR NRF:
//VCC->3.3V   GND->GND
//CSN->8      CE->7
//MOSI->11    SCK->13
//MISO->12

bool txData(const char*);
bool rxData(char*);
bool main_send_receive(char*, char*, uint16_t);
bool user_receive_send(char*, char*, uint16_t);

const byte MAIN_ADDR[6] = "00001";
//const byte USER_ADDR[6] = "00002";

//LCD: SDA->A4 SCK->A5
LiquidCrystal_I2C lcd(0x27, 16, 2);

//##### HELPER FUNCS PROTOTYPES #####
//LCD display user output function
void LCD_Print(int, int, int);
//joystick reading for x and y axes converted to angle (-180 to 180)
int joystickReading();

//joystick reading just for x axis
int jx_Reading();

void setup() {
  Serial.begin(9600);

  radio.begin();
  radio.openWritingPipe(MAIN_ADDR);     //User to Main
  radio.openReadingPipe(0, MAIN_ADDR);  //User is listening at 0
  radio.setPALevel(RF24_PA_MIN);
  radio.flush_rx();
  radio.flush_tx();

  pinMode(PIN_TOUCH_SENSOR, INPUT_PULLUP); //touch sensor
  pinMode(PIN_JOYSTICK_BUTTON, INPUT_PULLUP); //joystick pushbutton

  lcd.init();       //LCD initialization
  lcd.backlight();  //backlight on
}

void loop() {
  //INITIAL DATA PROCESSING BLOCK
  char rxBuffer[10];    //receive buffer
  char txBuffer[10];    //transmitt buffer
  //Device is on and waiting for the first data package from MAIN
  Serial.println("Waiting for MAIN");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Waiting for MAIN");

  //->DATA TRANSMISSION USES FOLLOWING FORMAT
  //DATA in form [MODE]:[FLASH]:[ANGLE]
  //MODE: 1(flash), 2 (joystick+flash), 3(reset)
  //FLASH:0(OFF), 1(ON)
  //ANGLE: from -180 to 180

  //waiting for the first data packet from MAIN here
  //if something is received in 150 ms, USER sends out "1:0:0" packet to establish communication
  if (!user_receive_send(rxBuffer, "1:0:0", 150)) {
      delay(10);  //wait for buffer data transfer
      return; // no response in 150 ms - restart loop()
  }

  //DATA PARCING BLOCK
  //the first packed is received, parcing the data
  int mode, flash, angle; //these vars are used to store global system states
  sscanf(rxBuffer, "%d:%d:%d", &mode, &flash, &angle);  //scan a string and save data in global system states vars
  Serial.print("RX_ok.Buffer:"); //used in debugging
  Serial.println(rxBuffer);

  //USER controls software logic until MODE 3 is selected
  while(mode != 3){
    //do all digital readings first
    //get sensor buttor reading. 
    uint8_t touchSensor = digitalRead(PIN_TOUCH_SENSOR); //button press is HIGH (0 off 1 on logic)
    //get joystick pushbutton reading
    uint8_t joystickTrigger = !digitalRead(PIN_JOYSTICK_BUTTON); //button press is LOW (0 off 1 on logic)
    int joystickAngle = joystickReading();  //get joystick angle position
    
    //user state vars are used internally (mainly for LCD output)
    int userMode = 0;
    int userFlash = 0;
    int userAngle = 0;

    if (joystickTrigger) {  //if user press joystick bushbutton - control is passed to MAIN
      //STATE [3:0:0]
      userMode = 3;
      userFlash = 0;
      userAngle = 0;
    }
    else if(!joystickTrigger && joystickAngle != 0) {
      //user controls both flash and motor
      //STATE [2:FLASH:ANGLE]
      userMode = 2;
      userFlash = touchSensor;
      userAngle = joystickAngle;
    }
    else {  //user controls flash only
      //STATE [1:FLASH:0]
      userMode = 1;
      userFlash = touchSensor;
      userAngle = 0;
    }

    //DATA TX BLOCK
    //form data string first
    sprintf(txBuffer, "%d:%d:%d", userMode, userFlash, userAngle);
    //user output and debugging
    LCD_Print(userMode, userFlash, userAngle);
    Serial.print("USER_SENDS-> ");
    Serial.print(txBuffer);
    Serial.print("\t");
    //once packet is sent, start listening again
    if (!user_receive_send(rxBuffer, txBuffer, 150)) {
        Serial.println("NO DATA FROM MAIN — retrying...");
        continue;
    }

    //once MAIN responds, parce data - while reiterates
    Serial.print("USER RECEIVED <- ");
    Serial.println(rxBuffer);
    sscanf(rxBuffer, "%d:%d:%d", &mode, &flash, &angle);

    delay(10);  //wait for buffer data transfer
  }

  // mode == 3 → exit communication
  Serial.println("USER EXITING (MODE 3)");
  }

//LOCAL HELPER FUNC DEFS
void LCD_Print(int mode, int flash, int angle){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MODE: ");
  lcd.print(mode, 1);
  lcd.print(" FLASH: ");
  lcd.print(flash, 1);
  lcd.setCursor(0, 1);
  lcd.print("ANGLE: ");
  lcd.print(angle);
}

int joystickReading(){
  int xValue = analogRead(A0);
  int yValue = analogRead(A1);
  int xMapped = map(xValue, 0, 1023, -512, 512);
  int yMapped = map(yValue, 0, 1023, -512, 512);
  int filter = 30;
  if (abs(xMapped) < filter) xMapped = 0;
  if (abs(yMapped) < filter) yMapped = 0;
  return (atan2(xMapped, yMapped) * 180.0 / 3.1415);
}

int jx_Reading(){
  int xValue = analogRead(A0);
  int xMapped = map(xValue, 0, 1023, -512, 512);
  if (abs(xMapped) < filter) xMapped = 0;
  if(xMapped >0) xValue = 5;
  else if(xMapped < 0) xValue = -5;
  else xValue = 0;
  return xValue;
}


//Data transmit function with acking implementation
//@param: const cahr data - data array size 10
//@return: returns ACK state - data transmission state
bool txData(const char* data) {
	if (!data) return 0;
	bool ack = radio.write(data, strlen(data) + 1);
	if (ack){
		//data transmission is successful
		Serial.print("TX_ok:");
		Serial.print(data);
		Serial.print("\t");
	}
	else {
		//error msg
		Serial.print("--->TX FAILED(NO ACK)\n");
	}
	return ack;	//
}

//Data receive function with state return
//@param: data char array size 10
//@return: data transmission state
bool rxData(char* data) {
	if (!data) return 0;	//if data is NULL
	if(radio.available()) {
		radio.read(data, 10);	//data is received
		data[9] = '\0';		//ensure the last byte is a NULL terminator
		Serial.print("RX_ok: ");
		Serial.print(data);
		Serial.print("\t");
		return 1;
	}
	return 0;	//RX failed
}

//Main device send and receive exchange function
//@param: tx - char array size 10 to transmit 
//@param: rx - char array size 10 to store reply
//@param: ms_timeout - timeout value in milliseconds
//@return: returns 1 if reply was received, 0 if timed out
bool main_send_receive(char* tx, char* rx, uint16_t ms_timeout){
	radio.stopListening();	//switch to TX mode
	txData(tx);	//send data packet
	radio.startListening();	//switch to RX mode
	unsigned long ms_initial = millis();	//initial time
	while(millis() - ms_initial < ms_timeout){	
		//return true if reply is received 
		if (rxData(rx)) return 1;
	}
	//timeout - return false
	Serial.print("MAIN_TIMEOUT\t");
	return 0;
}

//User device receive and send exchange function
//@param: tx - char array size 10 to transmit 
//@param: rx - char array size 10 to store reply
//@param: ms_timeout - timeout value in milliseconds
//@return: returns 1 if reply was received, 0 otherwise
bool user_receive_send(char* rx, char* tx, uint16_t ms_timeout){
	radio.startListening();	//swith to RX mode
	unsigned long ms_initial = millis(); //store ititial time value
	while(millis() - ms_initial < ms_timeout){
		if(rxData(rx)){		//if packed is received - reply to MAIN
			radio.stopListening();	//switch to TX mode
			txData(tx);		//send reply
			return 1;	//data transmission is successful
		}
	}
	return 0;		//no data is received - timeout is reached
}


