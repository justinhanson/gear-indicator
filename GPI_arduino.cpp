//	Gear Position Indicator
//    Copyright (C) 2015  justin@justinhanson.com
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.



// Do not remove the include below
#include "GPI_arduino.h"
#define _DEBUG						//uncomment for debug
#define END_MSG_CHAR '*'

/*
 * SV650 Manual from voltages
 * 1st gear - 1.36V
 * 2nd gear - 1.77V
 * 3rd gear - 2.49V
 * 4th gear - 3.23V
 * 5th gear - 4.1V
 * 6th gear - 4.55V
 * Neutral  - 5V
 *
 *
 *	LED PIN OUT
 *
 *    1: E				10: G
 *    2: D				9: F
 *    3: Common Anode	8: Common Anode
 *    4: C				7: A
 *    5: DP				6: B
 *
 *			A
 *		F		B
 *			G
 *		E		C
 *			D
 *					DP
 */

// pin out for 7 segment LED
const byte A = 10;  // TOP
const byte F = 3;  // TOP_LEFT
const byte B = 4;  // TOP_RIGHT
const byte G = 5;  // MIDDLE
const byte D = 6;  // BOTTOM
const byte E = 7;  // BOTTOM_LEFT
const byte C = 8;  // BOTTOM_RIGHT
const byte DP = 9; // DP

// helpers to easily switch between common anode and cathode LEDs
// led are output for low will be on for COMMON ANODE
//looks like my LEDs are COMMON CATHODE
const byte ON = HIGH;
const byte OFF = LOW;

const byte ACTIVITY = 13;
const byte GPS_PIN = A0;  // pin for reading analog voltage
const byte BUTTON_PIN = 2;

/// timer settings for displaying the digit
// timer1Counter = 59286; // preload timer 65536-16MHz/25610Hz i.e 100mS
// timer1Counter = 63036; // preload timer 65536-16MHz/256/25Hz i.e 40mS
// timer1Counter = 64285; // preload timer 65536-16MHz/256/50Hz i.e 20mS
// timer1Counter = 64910; // preload timer 65536-16MHz/256/100Hz i.e 10mS
const unsigned int timer1Counter = 59286;

const int firstSecondThreshold = 300;


// globals
volatile boolean buttonPressed = false;
volatile boolean inLearningMode = false;
volatile boolean isAboveThreshold;
char gear;

// Bluetooth vars
SimpleTimer SendCurrentGearTimer;
GPIData gpiData;



// the setup routine runs once when you press reset:
void setup() {
	Serial.begin(115200);
	SendCurrentGearTimer.setInterval(500, sendCurrentGear);

	configureLEDPins();
	configureGPSInput();
	configureButtons();

	//read in saved settings
	eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));

	testDisplay();
	//if eeprom is empty - go into learning mode
	if ( isnan(settings.firstCutoff) || settings.firstCutoff == 0)
	{
		#ifdef _DEBUG
		Serial.println("Eeprom not initialized");
		#endif
		learnGearVoltages();
	}
	else
	{
		#ifdef _DEBUG
		//dump out the settings
		Serial.println("Gear cut offs:");
		Serial.print("First: ");
		Serial.println(settings.firstCutoff);

		Serial.print("Second: ");
		Serial.println(settings.secondCutoff);

		Serial.print("Third: ");
		Serial.println(settings.thirdCutoff);

		Serial.print("Fourth: ");
		Serial.println(settings.fourthCutoff);

		Serial.print("Fifth: ");
		Serial.println(settings.fifthCutoff);

		Serial.print("Sixth: ");
		Serial.println(settings.sixthCutoff);
		#endif
	}

	noInterrupts();  // disable all interrupts

	TCCR1A = 0;
	TCCR1B = 0;
	// set up interrupt on overflow..
	TCNT1 = timer1Counter;   // preload timer
	TCCR1B |= (1 << CS12);    // 256 prescaler
	TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
	interrupts();
}

ISR(TIMER1_OVF_vect)        // interrupt service routine read all serial Ports
{
	TCNT1 = timer1Counter;  // reload  timer  to go off in another 100 Milli Seconds.
	if (! inLearningMode )
	{
		displayDigit(gear);
	}
}

// the loop routine runs over and over again forever:
void loop()
{
	static byte sampleCount;
	static int gpsVoltage;
	static int minGPSVoltage = 1000;
	static byte buttonState;
	static unsigned long startPress;
	static boolean ignoreButton;

	SendCurrentGearTimer.run();

	gpsVoltage = getGPSVoltage();

	if (gpsVoltage > firstSecondThreshold)
	{
		isAboveThreshold = true;
	}

	if (gpsVoltage < minGPSVoltage)
	{
		minGPSVoltage = gpsVoltage;
	}

	//only every 6th time through the loop will we update the gear
	if (sampleCount % 6 == 0)
	{
		gear = determineGear(minGPSVoltage);
		minGPSVoltage = 1000;  // reset  voltage to greater than max volt=500
	}
	sampleCount++;

	// Goes into learning mode
	buttonState = digitalRead(BUTTON_PIN);
	if (buttonState  == LOW)
	{
		startPress = 0; // If not pressed, reset counter
		ignoreButton = false;
	}
	else
	{
		if ( ! ignoreButton )
		{
			if (startPress == 0) // check if continued press
			{
				startPress = millis();  // if not, set timer
			}
			else if (millis() - startPress >= 2000)
			{
				ignoreButton = true;  // if user keeps holding down the button
				learnGearVoltages();
			}
		}
	}

	if (Serial.available())
	{
		receiveGPIData();
	}

}

void testDisplay()
{
	delay(1000);

	displayDigit(1);
	delay(250);
	displayDigit(2);
	delay(250);
	displayDigit(3);
	delay(250);
	displayDigit(4);
	delay(250);
	displayDigit(5);
	delay(250);
	displayDigit(6);
	delay(250);
	displayDigit(0);
}

// configure the segment leds to  output
void configureLEDPins()
{
	pinMode(A, OUTPUT);
	pinMode(B, OUTPUT);
	pinMode(C, OUTPUT);
	pinMode(D, OUTPUT);
	pinMode(E, OUTPUT);
	pinMode(F, OUTPUT);
	pinMode(G, OUTPUT);
	pinMode(DP, OUTPUT);

	//test led for arduino - blinks led to show it working
	pinMode(ACTIVITY, OUTPUT);
}

void configureGPSInput()
{
	pinMode(GPS_PIN, INPUT);
}

void configureButtons()
{
	pinMode(BUTTON_PIN, INPUT);
}

void allOff()
{
	digitalWrite(A, OFF);
	digitalWrite(B, OFF);
	digitalWrite(C, OFF);
	digitalWrite(D, OFF);
	digitalWrite(E, OFF);
	digitalWrite(F, OFF);
	digitalWrite(G, OFF);
}

void displayDigit(byte digit)
{
	allOff();

	// special cases
	// 0 is neutral
	// dancing zeros by default
	if (digit == 0)
	{
		dancingNeutral();
		return;
	}
	else if (digit == 8) //show 'L'
	{
		digitalWrite(F, ON);
		digitalWrite(E, ON);
		digitalWrite(D, ON);
		return;
	}
	else if (digit == 9) //show 'H'
	{
		digitalWrite(F, ON);
		digitalWrite(E, ON);
		digitalWrite(G, ON);
		digitalWrite(B, ON);
		digitalWrite(C, ON);
		return;
	}

	//Conditions for displaying segment a
	if(digit!=1 && digit != 4)
		digitalWrite(A, ON);

	//Conditions for displaying segment b
	if(digit != 5 && digit != 6)
		digitalWrite(B,ON);

	//Conditions for displaying segment c
	if(digit !=2)
		digitalWrite(C,ON);

	//Conditions for displaying segment d
	if(digit != 1 && digit !=4 && digit !=7)
		digitalWrite(D,ON);

	//Conditions for displaying segment e
	if(digit == 2 || digit ==6 || digit == 8 || digit==0)
		digitalWrite(E,ON);

	//Conditions for displaying segment f
	if(digit != 1 && digit !=2 && digit!=3 && digit !=7)
		digitalWrite(F,ON);
	if (digit!=0 && digit!=1 && digit !=7)
		digitalWrite(G,ON);

}

void dancingNeutral()
{
	static byte neutralCount;
	switch (neutralCount % 6)
	{
	case 0:
		digitalWrite(A,ON);
		break;
	case 1:
		digitalWrite(B,ON);
		break;
	case 2:
		digitalWrite(C,ON);
		break;
	case 3:
		digitalWrite(D,ON);
		break;
	case 4:
		digitalWrite(E,ON);
		break;
	case 5:
		digitalWrite(F,ON);
		break;
	}
	neutralCount++;
}

// switch to int to reduce float overhead, precision should be fine
int getGPSVoltage()
{
	int smallest, rawADC;
	smallest = analogRead(GPS_PIN);

	for (int i=0; i<15; i++)
	{
		rawADC = analogRead(GPS_PIN);
		if (rawADC < smallest)
		{
			smallest = rawADC;
		}
	}
	return smallest * (5.0 / 1023.0) * 100;
}

// Interrupt service routine for interrupt 0
void buttonInterupt()
{
	static unsigned long buttonTime = 0;
	static unsigned long lastButtonTime = 0;

	buttonTime = millis();
	//check to see if increment() was called in the last 250 milliseconds
	if (buttonTime - lastButtonTime > 250)
	{
		buttonPressed = true;
		lastButtonTime = buttonTime;
	}
}

void saveSettings()
{
	eeprom_update_block((const void*)&settings, (void*)0, sizeof(settings));
}

void learnGearVoltages()
{
	inLearningMode = true;
	int low, high;

	//turn on DP to signify learning mode
	digitalWrite(DP,ON);

	//make sure user has taken hand off of button before continuing
	allOff();
	digitalWrite(G, ON);
	byte buttonState = digitalRead(BUTTON_PIN);
	while (buttonState != LOW)
	{
		buttonState = digitalRead(BUTTON_PIN);
	}

	attachInterrupt(0, buttonInterupt, RISING);  //0 is pin 2

	// 8 = low, 9 = high
	saveGear(1, 9, &high);
	saveGear(2, 8, &low);
	settings.firstCutoff = (high + low)/2;

	saveGear(2, 9, &high);
	saveGear(3, 8, &low);
	settings.secondCutoff = (high + low)/2;

	saveGear(3, 9, &high);
	saveGear(4, 8, &low);
	settings.thirdCutoff = (high + low)/2;

	saveGear(4, 9, &high);
	saveGear(5, 8, &low);
	settings.fourthCutoff = (high + low)/2;

	saveGear(5, 9, &high);
	saveGear(6, 8, &low);
	settings.fifthCutoff = (high + low)/2;

	saveGear(6, 9, &high);
	settings.sixthCutoff = (high + 500)/2;

	digitalWrite(DP,OFF);

	//lastly save the new gear settings
	saveSettings();

	detachInterrupt(0);
	inLearningMode = false;
}

void saveGear(byte gear, byte hiLow, int *setting)
{
	//clear button pressed flag
	buttonPressed = false;
	int voltage = 0;
	allOff();

	while ( not buttonPressed )
	{
		displayDigit(gear);
		delay(500);
		displayDigit(hiLow);
		delay(500);
		voltage = getGPSVoltage();
		Serial.println (voltage);
	}

	// flash the gear to indicate saved state
	displayDigit(gear);
	delay(100);
	allOff();
	delay(100);

	displayDigit(gear);
	delay(100);
	allOff();
	delay(100);

	displayDigit(gear);

	*setting = voltage;
	delay(1000);
}

/*
 *  low and high readings for each gear to determine
 *  the cut-off points for determining gear
 */
byte determineGear(int gpsVoltage)
{
	if ( gpsVoltage <= settings.firstCutoff )
	{
		isAboveThreshold = false;
		return 1;
	}
	else if ( gpsVoltage <= settings.secondCutoff && isAboveThreshold == true)
	{
		return 2;
	}
	else if ( gpsVoltage <= settings.thirdCutoff )
	{
		return 3;
	}
	else if ( gpsVoltage <= settings.fourthCutoff )
	{
		return 4;
	}
	else if ( gpsVoltage <= settings.fifthCutoff )
	{
		return 5;
	}
	else if ( gpsVoltage <=  settings.sixthCutoff )
	{
		return 6;
	}

	return 0; //neutral
}





////////////////////////////////  NanoPB methods ////////////////////////////

void sendCurrentGear()
{
	gpiData.type = GPIData_MessageType_GEAR;
	gpiData.has_cutOffMessage = false;
	gpiData.has_requestMessage = false;
	gpiData.has_gearMessage = true;

	gpiData.gearMessage.has_gear = true;
	gpiData.gearMessage.gear = gear;

	sendGPIData();
}

void sendGearCutOffs()
{
	gpiData.type = GPIData_MessageType_CUTOFF;
	gpiData.has_cutOffMessage = true;
	gpiData.has_gearMessage = false;
	gpiData.has_requestMessage = false;

	gpiData.cutOffMessage.has_firstCutoff = true;
	gpiData.cutOffMessage.firstCutoff = settings.firstCutoff;

	gpiData.cutOffMessage.has_secondCutoff = true;
	gpiData.cutOffMessage.secondCutoff = settings.secondCutoff;

	gpiData.cutOffMessage.has_thirdCutoff = true;
	gpiData.cutOffMessage.thirdCutoff = settings.thirdCutoff;

	gpiData.cutOffMessage.has_fourthCutoff = true;
	gpiData.cutOffMessage.fourthCutoff = settings.fourthCutoff;

	gpiData.cutOffMessage.has_fifthCutoff = true;
	gpiData.cutOffMessage.fifthCutoff = settings.fifthCutoff;

	gpiData.cutOffMessage.has_sixthCutoff = true;
	gpiData.cutOffMessage.sixthCutoff = settings.sixthCutoff;

	sendGPIData();
}

void sendGPIData()
{
	uint8_t gearMessageBuffer[62];
	pb_ostream_t protobufOutputStream = pb_ostream_from_buffer(gearMessageBuffer, sizeof(gearMessageBuffer));
	bool status = pb_encode(&protobufOutputStream, GPIData_fields, &gpiData);

	/* Check for errors... */
	if (!status)
	{
		#ifdef _DEBUG
		Serial.println ("error");
		Serial.println(PB_GET_ERROR(&protobufOutputStream));
		#endif
		return;
	}


	for(size_t i=0;i<protobufOutputStream.bytes_written;i++)
	{
		Serial.write(gearMessageBuffer[i]);
	}
	Serial.write(END_MSG_CHAR);
	Serial.flush();
}


void receiveGPIData()
{
	GPIData gpiDataReceived;
	unsigned int length = 0;
	uint8_t fromAndroidBuffer[62];
	uint8_t next = ' ';

	while (Serial.available())  {
		next = Serial.read();
		fromAndroidBuffer[length++] = next;
		delay(50);
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t stream_in = pb_istream_from_buffer(fromAndroidBuffer, length);

	/* Now we are ready to decode the message. */
	bool status = pb_decode(&stream_in, GPIData_fields, &gpiDataReceived);

	/* Check for errors... */
	if (!status)
	{
		#ifdef _DEBUG
		Serial.println ("error from android");
		Serial.println(PB_GET_ERROR(&stream_in));
		#endif
	}
	else{
		if (gpiDataReceived.type == GPIData_MessageType_REQUEST)
		{
			if ( gpiDataReceived.requestMessage.type == RequestMessage_RequestType_GET_CUTOFFS )
			{
				sendGearCutOffs();
			}
		}
		else if (gpiDataReceived.type == GPIData_MessageType_CUTOFF)
		{
			if (gpiDataReceived.cutOffMessage.has_firstCutoff)
			{
				settings.firstCutoff = gpiDataReceived.cutOffMessage.firstCutoff;
			}
			if (gpiDataReceived.cutOffMessage.has_secondCutoff)
			{
				settings.secondCutoff = gpiDataReceived.cutOffMessage.secondCutoff;
			}
			if (gpiDataReceived.cutOffMessage.has_thirdCutoff)
			{
				settings.thirdCutoff = gpiDataReceived.cutOffMessage.thirdCutoff;
			}
			if (gpiDataReceived.cutOffMessage.has_fourthCutoff)
			{
				settings.fourthCutoff = gpiDataReceived.cutOffMessage.fourthCutoff;
			}
			if (gpiDataReceived.cutOffMessage.has_fifthCutoff)
			{
				settings.fifthCutoff = gpiDataReceived.cutOffMessage.fifthCutoff;
			}
			if (gpiDataReceived.cutOffMessage.has_sixthCutoff)
			{
				settings.sixthCutoff = gpiDataReceived.cutOffMessage.sixthCutoff;
			}
			saveSettings();
			sendGearCutOffs();
		}

	}
}
