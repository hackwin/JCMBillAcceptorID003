// JCM WB-13-SS ID-003, Bill Acceptor
// Connected to Serial1 on Arduino Mega
// Bill Acceptor 20 pin interface, TTL: pins 16 RX and 19 TX, 9600 bps 8E1
// Arduino C++, written on www.codebender.cc

// By Jesse Campbell
// January 2015
// http://www.jbcse.com

#include <EEPROM.h>
#include <Arduino.h>
//#include "JCMBillAcceptorID003.h"

#ifndef _JCMBILLACCEPTORID003_
#define _JCMBILLACCEPTORID003_

class JCMBillAcceptorID003{
	
	public:
		boolean PRINT_BYTES;
		boolean DEBUG;
		boolean PAUSE_UNTIL_RX;
		int WAIT_FOR_RESPONSE_DELAY;
		
		JCMBillAcceptorID003(HardwareSerial &hs): hardwareSerial(hs){
			
			if (&hardwareSerial == &Serial1){
				BILL_ACCEPTOR_RX = 18;
				BILL_ACCEPTOR_TX = 19;
			}
			else if (&hardwareSerial == &Serial2){
				BILL_ACCEPTOR_RX = 20;
				BILL_ACCEPTOR_TX = 21;
			}
			else if (&hardwareSerial == &Serial3){
				BILL_ACCEPTOR_RX = 22;
				BILL_ACCEPTOR_TX = 23;
			}
			
			pinMode(BILL_ACCEPTOR_RX, INPUT);
			digitalWrite(BILL_ACCEPTOR_RX, INPUT_PULLUP);
			pinMode(BILL_ACCEPTOR_TX, INPUT);
			digitalWrite(BILL_ACCEPTOR_TX, INPUT_PULLUP);
			
			billReceived = 0;
			PRINT_BYTES = false;
			DEBUG = false;
			PAUSE_UNTIL_RX = false;
			WAIT_FOR_RESPONSE_DELAY = 50;
			
			CASH_TOTAL_RECEIVED_EEPROM_ADDR = 0; // EEPROM address 0 to address 3; 4 bytes
			BOOT_COUNT_EEPROM_ADDR = 4; 
			
			hardwareSerial.begin(9600, SERIAL_8E1); //on bill acceptor TTL pins 16 RX and 19 TX
		}
		
		void bootup(){
			if (PAUSE_UNTIL_RX){ //pause until a char is sent to the serial port
				while(Serial.available() == 0);
				while(Serial.available()) Serial.read();
			}
			
			increment(BOOT_COUNT_EEPROM_ADDR, 1);
			
			{ byte req[] = {RESET}; beginTrans(req,sizeof(req)/sizeof(req[0])); }
			
			
			//{ byte req[] = {STATUS_REQUEST}; beginTrans(req,sizeof(req)/sizeof(req[0])); }
			
			
			{ byte req[] = {SET_SECURITY_DENOMINATIONS, 
						   SECURITY_DENOMINATIONS_ALL_STANDARD, 
						   DATA_BYTE_UNUSED}; 
						   beginTrans(req,sizeof(req)/sizeof(req[0])); 
			}
			/*
			{ byte req[] = {SET_ENABLE_DISABLE_DENOMINATIONS, 
						    DENOMINATIONS_ALL, //DENOMINATIONS_ONES_FIVES,
						    DATA_BYTE_UNUSED}; 
						    beginTrans(req,sizeof(req)/sizeof(req[0])); 
			}
			*/
			{ byte req[] = {OPTIONAL_FUNCTION, 
						    OPTION1_HANGING & OPTION2_POWER_RECOVERY, 
						    DATA_BYTE_UNUSED};
						    beginTrans(req,sizeof(req)/sizeof(req[0])); 
			}
			
			
			//{ byte req[] = {VERSION_REQUEST}; beginTrans(req,sizeof(req)/sizeof(req[0])); }
			
			
			//{ byte req[] = {CURRENCY_ASSIGNMENT}; beginTrans(req,sizeof(req)/sizeof(req[0])); }
			
			/*{ byte req[] = {BARCODE_FUNCTION, 
						    INTERLEAVED_TWO_OF_FIVE, 
						    NUMBER_OF_CHARACTERS};
						    beginTrans(req,sizeof(req)/sizeof(req[0])); 
			}
			*/
			{ byte req[] = {BAR_INHIBIT, 
						    ENABLE_CURRENCY_ACCEPTANCE & DISABLE_BARCODE_TICKET_ACCEPTANCE};
						    beginTrans(req,sizeof(req)/sizeof(req[0])); 
			}
			
			{ byte req[] = {COMMUNICATION_MODE, 
						    POLLED};
						    beginTrans(req,sizeof(req)/sizeof(req[0])); 
			}
			
			{ byte req[] = {INHIBIT_ACCEPTOR, 
						    ENABLE_ACCEPTOR};
						    beginTrans(req,sizeof(req)/sizeof(req[0])); 
			}
			
		}
		
		void setPaymentConfirmedCallback (void (*callbackFunction)(int amount)){
			this->callbackFunction = callbackFunction;
		}
		
		void poll(){
			{ byte req[] = {STATUS_REQUEST}; beginTrans(req,sizeof(req)/sizeof(req[0])); }
			//t.update();
		}
		
		void resetEEPROMCounters(){
			resetCounter(CASH_TOTAL_RECEIVED_EEPROM_ADDR);
			resetCounter(BOOT_COUNT_EEPROM_ADDR);
			//Serial.println("Counters have been reset");
		}
		
		unsigned long totalCashReceived(){
			//Serial.print("Total cash: $");
			return EEPROMReadlong(CASH_TOTAL_RECEIVED_EEPROM_ADDR);
		}
		
		unsigned long bootCount(){
			//Serial.print("Boot counter: ");	
			return EEPROMReadlong(BOOT_COUNT_EEPROM_ADDR);
		}
	private:
		static const byte STATUS_REQUEST = 0x11;
		static const byte ACK = 0x50;
		static const byte RESET = 0x40;
		static const byte SET_SECURITY_DENOMINATIONS = 0xC1;
		static const byte SECURITY_DENOMINATIONS_ALL_STANDARD = 0x82;
		static const byte SET_ENABLE_DISABLE_DENOMINATIONS = 0xC0;
		static const byte DENOMINATIONS_ALL = B00000000;
		static const byte DENOMINATIONS_ONES_FIVES = B11111010; //0 is disable, 1 is enable; see pdf
		static const byte OPTIONAL_FUNCTION = 0xC5;
		static const byte VERSION_REQUEST = 0x88;
		static const byte CURRENCY_ASSIGNMENT = 0x8A;
		static const byte BARCODE_FUNCTION = 0xC6;
		static const byte BAR_INHIBIT = 0xC7;
		static const byte COMMUNICATION_MODE = 0xC2;
		static const byte INHIBIT_ACCEPTOR = 0xC3;
		static const byte ENABLED_IDLING = 0x11;
		static const byte ACCEPTING = 0x12;
		static const byte ESCROW = 0x13;
		static const byte STACKING = 0x14;
		static const byte VEND_VALID = 0x15;
		static const byte STACKED = 0x16;
		static const byte REJECTING = 0x17;
		static const byte POWER_UP = 0x40;
		static const byte VERSION_RESPONSE = 0x88;
		static const byte INITIALIZING = 0x1B;
		static const byte ONE_DOLLAR = 0x61;
		static const byte FIVE_DOLLARS = 0x63;
		static const byte TEN_DOLLARS = 0x64;
		static const byte TWENTY_DOLLARS = 0x65;
		static const byte FIFTY_DOLLARS = 0x66;
		static const byte ONE_HUNDRED_DOLLARS = 0x67;
		static const byte STACK_1 = 0x41;
		static const byte INVALID_COMMAND = 0x4B; 
		static const byte DATA_BYTE_UNUSED = 0x00;
		static const byte OPTION1_HANGING = 0x01;
		static const byte OPTION2_POWER_RECOVERY = 0x02;
		static const byte INTERLEAVED_TWO_OF_FIVE = 0x01;
		static const byte NUMBER_OF_CHARACTERS = 0x12;
		static const byte ENABLE_CURRENCY_ACCEPTANCE = 0x00;
		static const byte DISABLE_CURRENCY_ACCEPTANCE = 0x01;
		static const byte ENABLE_BARCODE_TICKET_ACCEPTANCE = 0x00;
		static const byte DISABLE_BARCODE_TICKET_ACCEPTANCE = 0x02;
		static const byte POLLED = 0x00;
		static const byte INTERRUPT_MODE1 = 0x01;
		static const byte INTERRUPT_MODE2 = 0x02;
		static const byte ENABLE_ACCEPTOR = 0x00;
		static const byte DISABLE_ACCEPTOR = 0x01;
		static const byte INSERTION_ERROR_CROOKED = 0x71;
		static const byte MAGNETIC_PATTERN_ERROR_CENTER = 0x72;
		static const byte OTHER_SENSORS_DETECTED_SOMETHING = 0x73;
		static const byte DATA_AMPLITUDE_ERROR = 0x74;
		static const byte FEED_ERROR = 0x75;
		static const byte DENOMINATION_ASSESSING_ERROR = 0x76;
		static const byte PHOTO_PATTERN_ERROR_MARKS_TEARS = 0x77;
		static const byte PHOTO_LEVEL_ERROR_DOUBLES_DIRTY = 0x78;
		static const byte DISABLED_BY_DIP_SWITCH_OR_COMMAND = 0x79;
		static const byte OPERATION_ERROR = 0x7B;
		static const byte BILL_DETECTED_IN_TRANSPORT_ASSEMBLY_AT_WRONG_TIME = 0x7C;
		static const byte LENGTH_ERROR = 0x7D;
		static const byte COLOR_PATTERN_ERROR = 0x7E;
		static const byte SYNC = 0xFC;
		byte BILL_ACCEPTOR_RX;
		byte BILL_ACCEPTOR_TX;
		void (*callbackFunction)(int amount);
		byte billReceived;
		int CASH_TOTAL_RECEIVED_EEPROM_ADDR; // EEPROM address 0 to address 3; 4 bytes
		int BOOT_COUNT_EEPROM_ADDR;
		HardwareSerial &hardwareSerial;
		
		void beginTrans(byte request[], byte requestSize){
			sendRequest(request, requestSize);
			delay(WAIT_FOR_RESPONSE_DELAY); //each serial byte at 9600 bps adds 1 millisecond
			processResponse();	
		}
		void sendRequest(byte request[], byte requestSize){
			byte temp[requestSize+2];
			
			temp[0] = SYNC;
			temp[1] = requestSize+4;
		
			for(int i=0; i < requestSize; i++)
				temp[i+2] = request[i];
			
			int crcval = calcCRC(temp, sizeof(temp)/sizeof(temp[0]), 0);
			//Serial.println(crcval);
			byte crc[] = {crcval & 0xFF, crcval >> 8};
			
			if (PRINT_BYTES){
				Serial.print("H -> A, ");
				for(int i=0; i<sizeof(temp)/sizeof(temp[0]); i++){
					if (temp[i] < 0x16)
						Serial.print("0");
					Serial.print(temp[i], HEX);
					Serial.print(" ");
				}
				
				for(int i=0; i<sizeof(crc)/sizeof(crc[0]); i++){
					if (crc[i] < 0x16)
						Serial.print("0");
					Serial.print(crc[i], HEX);	
					Serial.print(" ");
				}
				Serial.println();
			}
			
			for(int i=0; i<sizeof(temp)/sizeof(temp[0]); i++)
				hardwareSerial.write(temp[i]);
				
			for(int i=0; i<sizeof(crc)/sizeof(crc[0]); i++)
				hardwareSerial.write(crc[i]);
		}
		
		void processResponse(){
			
		//	while(hardwareSerial.available()){
		//		int byteRead = hardwareSerial.read();
		//		if (byteRead < 0x10)
		//			Serial.print('0');
		//		Serial.print(byteRead,HEX);
		//		Serial.print(" ");
		//	}
		//	Serial.println();
		//	return;
			
			byte bytesRead = 0;
			byte responseSize = 0;
		
			while(hardwareSerial.available() == false);
			
			while(hardwareSerial.available()){
				
				if (bytesRead == 0){
					byte first = (byte)hardwareSerial.read();
					//Serial.print("first ");
					//Serial.println(first, HEX);
					if (first != SYNC){
						if (DEBUG) {
							Serial.print("SYNC failed, ");
							Serial.print("byte received ");
							Serial.println(first, HEX);
						}
					}
				}
				else if (bytesRead == 1){
					responseSize = hardwareSerial.read();
					byte response[responseSize];
					response[0] = SYNC;
					response[1] = responseSize;
					//response[2] = hardwareSerial.read();
					//Serial.println(response[2]);
				
					for(int i=2; i<responseSize; i++){
						
						if (Serial.available() == false)
							delay(10);
							
						response[i] = hardwareSerial.read();
						bytesRead++;
					}
					if (PRINT_BYTES){
						Serial.print("H <- A, ");
						for(int i=0; i<responseSize; i++){
							if (response[i] < 0x16)
								Serial.print("0");
							Serial.print(response[i],HEX);
							Serial.print(" ");
						}
						Serial.println();
					}
					
					if (response[2] == ESCROW){
					    if (response[3] == ONE_DOLLAR)
					    	billReceived = 1;
					    else if (response[3] == FIVE_DOLLARS)
					      	billReceived = 5;
					    else if (response[3] == TEN_DOLLARS)
					      	billReceived = 10;
					    else if (response[3] == TWENTY_DOLLARS)
					      	billReceived = 20;
					    else if (response[3] == FIFTY_DOLLARS)
					      	billReceived = 50;
					    else if (response[3] == ONE_HUNDRED_DOLLARS)
					      	billReceived = 100;
					    else
					      if (DEBUG) Serial.println("exception, \"reversed\" bill code received, see pdf");
					      
					    //Serial.print("Bill received: $");
					    //Serial.println(billReceived);
					    
				    	{ byte req[] = {STACK_1}; beginTrans(req,sizeof(req)/sizeof(req[0]));}
					}
					else if (response[2] == VEND_VALID){
						{ byte req[] = {ACK}; sendRequest(req,sizeof(req)/sizeof(req[0]));} //don't listen for a response
						callbackFunction(billReceived);
						//Serial.println("Bill stored, dispense");
						increment(CASH_TOTAL_RECEIVED_EEPROM_ADDR, billReceived);
						billReceived = 0;
					}
					else if (response[2] == INVALID_COMMAND)
						if (DEBUG) Serial.println("reponded with received invalid command");
					else if (response[2] == REJECTING){
						   if (DEBUG) Serial.println("rejecting bill, ");
						   if (response[3] == INSERTION_ERROR_CROOKED);
						     if (DEBUG) Serial.print("Insertion error (Crooked insertion)");
						   else if (response[3] == MAGNETIC_PATTERN_ERROR_CENTER)
						     if (DEBUG) Serial.print("Magnetic pattern error (Center)");
						   else if (response[3] == OTHER_SENSORS_DETECTED_SOMETHING)
						     if (DEBUG) Serial.print("While idle, a sensor other than the entrance sensors detected something.");
						   else if (response[3] == DATA_AMPLITUDE_ERROR)
						     if (DEBUG) Serial.print("Data amplitude error.");
						   else if (response[3] == FEED_ERROR)
						     if (DEBUG) Serial.print("Feed error");
						   else if (response[3] == DENOMINATION_ASSESSING_ERROR)
						     if (DEBUG) Serial.print("Denomination assessing error");
						   else if (response[3] == PHOTO_PATTERN_ERROR_MARKS_TEARS)
						     if (DEBUG) Serial.print("Photo pattern error (Marks, tears etc).");
						   else if (response[3] == PHOTO_LEVEL_ERROR_DOUBLES_DIRTY)
						     if (DEBUG) Serial.print("Photo level error (Sometimes caused by double notes or dirty bills)");
						   else if (response[3] == DISABLED_BY_DIP_SWITCH_OR_COMMAND)
						     if (DEBUG) Serial.print("Bill was disabled by DIP switch or command (Inhibit, direction).");
						   else if (response[3] == OPERATION_ERROR)
						     if (DEBUG) Serial.print("Operation error");
						   else if (response[3] == BILL_DETECTED_IN_TRANSPORT_ASSEMBLY_AT_WRONG_TIME)
						     if (DEBUG) Serial.print("A bill was detected in the transport assembly at the wrong time.");
						   else if (response[3] == LENGTH_ERROR)
						     if (DEBUG) Serial.print("Length error");
						   else if (response[3] == COLOR_PATTERN_ERROR)
						     if (DEBUG) Serial.print("Color pattern error.");  
					}
					
					int crcval = calcCRC(response, sizeof(response)/sizeof(response[0])-2, 0);
					//Serial.println(crcval);
					byte crc[] = {crcval & 0xFF, crcval >> 8};
					
					if (crc[0] != response[responseSize-2] || crc[1] != response[responseSize-1]){
						if (DEBUG) {
							Serial.println("message failed CRC check");
							Serial.print("crc[0] = ");
							Serial.print(crc[0],HEX);
							Serial.print(", crc[1] = ");
							Serial.print(crc[1],HEX);
							Serial.print(", response crc ");
							Serial.print(response[responseSize-2],HEX);
							Serial.print(" ");
							Serial.println(response[responseSize-1],HEX);
						}
					}
						
					if (responseSize != bytesRead+1)
						if (DEBUG) Serial.println("response length did not match stated length");
				}
				bytesRead++;
			}
		
		}
		
		unsigned calcCRC(byte *data, unsigned n, unsigned start) {
		    unsigned I, k, q, c, crcval;
		    crcval=start;
		    for (I=0; I<n; I++) {
		        c=data[I] & 0xFF;
		        q=(crcval^c) & 0x0F;
		        crcval=(crcval>>4)^(q*0x1081);
		        q=(crcval^(c>>4)) & 0x0F;
		        crcval=(crcval>>4)^(q*0x1081);
		    }
		    return crcval;
		}
		
		void EEPROMWritelong(int address, unsigned long value){
		
			byte four = (value & 0xFF);
			byte three = ((value >> 8) & 0xFF);
			byte two = ((value >> 16) & 0xFF);
			byte one = ((value >> 24) & 0xFF);
			
			EEPROM.write(address, four);
			EEPROM.write(address + 1, three);
			EEPROM.write(address + 2, two);
			EEPROM.write(address + 3, one);
		}
		
		unsigned long EEPROMReadlong(unsigned long address){
		    unsigned long four = EEPROM.read(address);
		    unsigned long three = EEPROM.read(address + 1);
		    unsigned long two = EEPROM.read(address + 2);
		    unsigned long one = EEPROM.read(address + 3);
		
		    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
		}
		
		void increment(long address, byte amount){
			EEPROMWritelong(address, EEPROMReadlong(address)+amount);
		}
		
		void resetCounter(int address){
			EEPROMWritelong(address, 0);	
		}

};

#endif