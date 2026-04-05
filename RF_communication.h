#pragma once
#ifndef _RF_communication_H
#define _RF_communication_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//WIRING FOR NRF:
//VCC->3.3V   GND->GND
//CSN->8      CE->7
//MOSI->11    SCK->13
//MISO->12

//RF24 radio(7, 8); - declared in INO
extern RF24 radio;

bool txData(const char*);
bool rxData(char*);
bool main_send_receive(char*, char*, uint16_t);
bool user_receive_send(char*, char*, uint16_t);

#endif // !1
