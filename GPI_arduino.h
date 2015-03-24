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


// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _GPI_arduino_H_
#define _GPI_arduino_H_
#include "Arduino.h"
//add your includes for the project GPI_arduino here

#include <avr/eeprom.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "gpi.pb.h"
#include <SimpleTimer.h>


//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

struct settings_t
{
	int firstCutoff, secondCutoff, thirdCutoff, fourthCutoff, fifthCutoff, sixthCutoff;
} settings;



//add your function definitions for the project GPI_arduino here
void configureLEDPins();
void configureGPSInput();
void configureButtons();
void testDisplay();
void displayDigit(byte);
void dancingNeutral();
int getGPSVoltage();
void saveSettings();
void learnGearVoltages();
byte determineGear(int);
void saveGear(byte, byte, int *);
void buttonInterupt();
void sendCurrentGear();
void sendGearCutOffs();
void sendGPIData();
void receiveGPIData();

//Do not add code below this line
#endif /* _GPI_arduino_H_ */
