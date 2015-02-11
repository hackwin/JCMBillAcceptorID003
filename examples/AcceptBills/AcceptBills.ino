// JCM WB-13-SS ID-003, Bill Acceptor
// Connected to Serial1 on Arduino Mega
// Bill Acceptor 20 pin interface, TTL: pins 16 RX and 19 TX, 9600 bps 8E1
// Arduino C++, written on www.codebender.cc

// By Jesse Campbell
// January 2015
// http://www.jbcse.com

#include <EEPROM.h>
#include <Arduino.h>
#include "jcmbillacceptorid003.h"

JCMBillAcceptorID003 billAcceptor (Serial1);

void paymentReceived(int amount){
	Serial.print("Received: $");
	Serial.println(amount);
	Serial.println("Vend the good or service!");
	Serial.print("Total cash received: $");
	Serial.println(billAcceptor.totalCashReceived());
}

void setup(){
	Serial.begin(115200);
	
	//Uncomment to reset the counters back to zero, comment afterwards
	//if (true){billAcceptor.resetEEPROMCounters(); while(true){};}

	//print hex bytes in Serial, Host to Acceptor and vice versa
	billAcceptor.PRINT_BYTES = false;

	//print detailed debug information
	billAcceptor.DEBUG = false;

	//bill acceptor will not boot until one or more characters is sent to the serial port
	billAcceptor.PAUSE_UNTIL_RX = false;
	
	//time to sleep until checking for the bill acceptor's message (poll mode)
	billAcceptor.WAIT_FOR_RESPONSE_DELAY = 50;

	//specify your function to do something when a payment is successful
	billAcceptor.setPaymentConfirmedCallback(paymentReceived);

	billAcceptor.bootup();
	Serial.print("Boot Count: ");
	Serial.println(billAcceptor.bootCount());
}

void loop(){
	billAcceptor.poll();
}    