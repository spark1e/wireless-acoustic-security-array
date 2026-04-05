#include "RF_communication.h"
#include <string.h>
#include <stdlib.h>

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
