#pragma once
#include <Arduino.h>
// TinyBasic Plus para ESP32 + FabGL (VGA32)
// Reorganizado em multiplos arquivos a partir do .ino original de fg1998
// Este arquivo concentra as flags de configuracao/hardware. O historico de
// versoes original foi preservado abaixo, sem alteracoes.

#define kVersion "v0.15"
// v016. 2026-02-23
// New Commands
//  WIFISCAN
//  WIFICONNECT "<SSID>", "<PASSWORD>"
//  WIFISTATUS
//  WIFIDISCONNECT
//  HTTPGET "URL"


// v0.15: 2029-10-23
//      Changes by fg1998 (fg1998@gmail.com)
//      Only for ESP32 with FabGL !!!!!!
//      Modified for latest version of FagGL (0.9.0)
//      Redefined SPI Pins for SD Card work with VGA32_V1.4 from TTGO - SCK = 14,  MISO = 02, MOSI = 12, CS = 13
//      Backspace is now working with Terminal
//      LOAD and SAVE working,  needs to start with a '/' ex: LOAD "/test.bas"  
//      Adjust screen resolution to 640x200 (80x25 text) for low ram use
//      **NEW COMMANDS
//      CLS
//      COLOR <INT>,<INT> -> FORECOLOR, BACKCOLOR
//      POINT <INT>, <INT>, <INT> -> COLOR, X, Y
//      LINE <INT>, <INT>, <INT>, <INT>, <INT>, <INT>  -> COLOR, INIT X, INIT Y, END X, END Y, PEN WIDTH
//      RECTANGLE <INT>, <INT>, <INT>, <INT>, <INT>, <INT>, <INT> -> COLOR, COLOR FILL (-1 USES NO COLOR), INIT X, INIT Y, END X, END Y, PEN WIDTH
//      ELIPSE <INT>, <INT>, <INT>, <INT>, <INT> -> COLOR, X, Y, WIDTH, HEIGHT, PEN WIDTH
//      CURSOR 0/1 -> ENABLE/DISABLE CURSOR
//      AT <INT>,<INT> -> puts cursor on x,y

//      
// v0.14: 2013-11-07
//      Input command always set the variable to 99
//      Modified Input command to accept an expression using getn()
//      Syntax is "input x" where x is any variable
//
// v0.13: 2013-03-04
//      Support for Arduino 1.5 (SPI.h included, additional changes for DUE support)
//
// v0.12: 2013-03-01
//      EEPROM load and save routines added: EFORMAT, ELIST, ELOAD, ESAVE, ECHAIN
//      added EAUTORUN option (chains to EEProm saved program on startup)
//      Bugfixes to build properly on non-arduino systems (PROGMEM #define workaround)
//      cleaned up a bit of the #define options wrt TONE
//
// v0.11: 2013-02-20
//      all display strings and tables moved to PROGMEM to save space
//      removed second serial
//      removed pinMode completely, autoconf is explicit
//      beginnings of EEPROM related functionality (new,load,save,list)
//
// v0.10: 2012-10-15
//      added kAutoConf, which eliminates the "PINMODE" statement.
//      now, DWRITE,DREAD,AWRITE,AREAD automatically set the PINMODE appropriately themselves.
//      should save a few bytes in your programs.
//
// v0.09: 2012-10-12
//      Fixed directory listings.  FILES now always works. (bug in the SD library)
//      ref: http://arduino.cc/forum/index.php/topic,124739.0.html
//      fixed filesize printouts (added printUnum for unsigned numbers)
//      #defineable baud rate for slow connection throttling
//e
// v0.08: 2012-10-02
//      Tone generation through piezo added (TONE, TONEW, NOTONE)
//
// v0.07: 2012-09-30
//      Autorun buildtime configuration feature
//
// v0.06: 2012-09-27
//      Added optional second serial input, used for an external keyboard
//
// v0.05: 2012-09-21
//      CHAIN to load and run a second file
//      RND,RSEED for random stuff
//      Added "!=" for "<>" synonym
//      Added "END" for "STOP" synonym (proper name for the functionality anyway)
//
// v0.04: 2012-09-20
//      DELAY ms   - for delaying
//      PINMODE <pin>, INPUT|IN|I|OUTPUT|OUT|O
//      DWRITE <pin>, HIGH|HI|1|LOW|LO|0
//      AWRITE <pin>, [0..255]
//      fixed "save" appending to existing files instead of overwriting
//  Updated for building desktop command line app (incomplete)
//
// v0.03: 2012-09-19
//  Integrated Jurg Wullschleger whitespace,unary fix
//  Now available through github
//  Project renamed from "Tiny Basic in C" to "TinyBasic Plus"
//     
// v0.02b: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
//  Better FILES listings
//
// v0.02a: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
//  Support for SD Library
//  Added: SAVE, FILES (mostly works), LOAD (mostly works) (redirects IO)
//  Added: MEM, ? (PRINT)
//  Quirk:  "10 LET A=B+C" is ok "10 LET A = B + C" is not.
//  Quirk:  INPUT seems broken?


// hack to let makefiles work with this file unchanged
#ifdef FORCE_DESKTOP 
#undef ARDUINO
#include "desktop.h"
#else
#ifndef ARDUINO
#define ARDUINO 1
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Feature option configuration...

// This enables LOAD, SAVE, FILES commands through the Arduino SD Library
// it adds 9k of usage as well.
#define ENABLE_FILEIO 1


// this turns on "autorun".  if there's FileIO, and a file "autorun.bas",
// then it will load it and run it when starting up
//#define ENABLE_AUTORUN 1
#undef ENABLE_AUTORUN
// and this is the file that gets run
#define kAutorunFilename  "autorun.bas"

// this is the alternate autorun.  Autorun the program in the eeprom.
// it will load whatever is in the EEProm and run it
#define ENABLE_EAUTORUN 1
//#undef ENABLE_EAUTORUN

// this will enable the "TONE", "NOTONE" command using a piezo
// element on the specified pin.  Wire the red/positive/piezo to the kPiezoPin,
// and the black/negative/metal disc to ground.
// it adds 1.5k of usage as well.
//#define ENABLE_TONES 1
#undef ENABLE_TONES
#define kPiezoPin 6 //------------------------------------------ default era 5 -----------------------------------

// we can use the EEProm to store a program during powerdown.  This is 
// 1kbyte on the '328, and 512 bytes on the '168.  Enabling this here will
// allow for this funcitonality to work.  Note that this only works on AVR
// arduino.  Disable it for DUE/other devices.
//#define ENABLE_EEPROM 1
#undef ENABLE_EEPROM

#include "WiFi.h" // include this or you´ll get erro in FabGL
#include <HTTPClient.h>
#include "fabgl.h"

// Sometimes, we connect with a slower device as the console.
// Set your console D0/D1 baud rate here (9600 baud default)
//#define kConsoleBaud 9600
#define kConsoleBaud 115200 

////////////////////////////////////////////////////////////////////////////////
#ifdef ARDUINO
#ifndef RAMEND
// okay, this is a hack for now
// if we're in here, we're a DUE probably (ARM instead of AVR)

//#define RAMEND 4096-1
#define RAMEND 262144-1  //-------------------------- RAM increment for ESP32 (agora em PSRAM via ps_malloc - ver setup() em main.cpp; 256KB, pode subir bem mais) ---------


// turn off EEProm
#undef ENABLE_EEPROM //----------------------------- provo a commentare questo ma non va --------------------------------------- 
#undef ENABLE_TONES  //----------------------------- provo a commentare questo ma non va --------------------------------------- 

#endif

#include <FS.h> 
#include <SD.h>
#include <SPI.h> /* needed as of 1.5 beta */

// Arduino-specific configuration
// set this to the card select for your SD shield
#define kSD_CS 13  // ------------------------ old era 10 -------------------------------------------------------------

#define kSD_Fail  0
#define kSD_OK    1

// set up our RAM buffer size for program and user input
// NOTE: This number will have to change if you include other libraries.
#ifdef ARDUINO
#ifdef ENABLE_FILEIO
#define kRamFileIO (1030) /* approximate */
#else
#define kRamFileIO (0)
#endif
#ifdef ENABLE_TONES
#define kRamTones (40)
#else
#define kRamTones (0)
#endif
#endif /* ARDUINO */
#define kRamSize  (RAMEND - 1160 - kRamFileIO - kRamTones) 

#ifndef ARDUINO
// Not arduino setup
#include <stdio.h>
#include <stdlib.h>
#undef ENABLE_TONES

// size of our program ram
#define kRamSize   4096 /* arbitrary */
//#define kRamSize   16384 /* arbitrary */ //-------------------------------------------------------------- kRamSize ------------------------------------------------------------------------- 

#endif

#ifndef boolean 
#define boolean int
#define true 1
#define false 0
#endif
#endif

#ifndef byte
typedef unsigned char byte;
#endif

// some catches for AVR based text string stuff...
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte( A ) *(A)
#endif

////////////////////////////////////////////////////////////////////////////////
// ASCII Characters
#define CR  '\r'
#define NL  '\n'
#define LF      0x0a
#define TAB '\t'
#define BELL  '\b'
#define SPACE   ' '
#define SQUOTE  '\''
#define DQUOTE  '\"'
#define CTRLC 0x03
#define CTRLH 0x08
#define CTRLS 0x13
#define CTRLX 0x18
#define BACKSPACE 0x7F

#ifdef ARDUINO
#define ECHO_CHARS 1
#else
#define ECHO_CHARS 0
#endif
