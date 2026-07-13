#include "basic_tables.h"
#include <string.h>

// Definicoes dos arrays de tabela (removido o "static" original, ja que
// agora sao compartilhados entre varios arquivos .cpp).

const unsigned char keywords[] PROGMEM = {
  'L','I','S','T'+0x80,
  'L','O','A','D'+0x80,
  'N','E','W'+0x80,
  'R','U','N'+0x80,
  'S','A','V','E'+0x80,
  'N','E','X','T'+0x80,
  'L','E','T'+0x80,
  'I','F'+0x80,
  'G','O','T','O'+0x80,
  'G','O','S','U','B'+0x80,
  'R','E','T','U','R','N'+0x80,
  'R','E','M'+0x80,
  'F','O','R'+0x80,
  'I','N','P','U','T'+0x80,
  'P','R','I','N','T'+0x80,
  'P','O','K','E'+0x80,
  'S','T','O','P'+0x80,
  'B','Y','E'+0x80,
  'F','I','L','E','S'+0x80,
  'M','E','M'+0x80,
  '?'+ 0x80,
  '\''+ 0x80,
  'A','W','R','I','T','E'+0x80,
  'D','W','R','I','T','E'+0x80,
  'D','E','L','A','Y'+0x80,
  'E','N','D'+0x80,
  'R','S','E','E','D'+0x80,
  'C','H','A','I','N'+0x80,
  //NEW COMMANDS BY fg1998@gmail.com, only for ESP32
  'C','L','S'+0x80,
  'C','O','L','O','R'+0x80,
  'P','O','I','N','T'+0x80,
  'L','I','N','E'+0x80,
  'R','E','C','T','A','N','G','L','E'+0x80,
  'E','L','I','P','S','E'+0x80,
  'C','U','R','S','O','R'+0x80,
  'A','T'+0x80,
  'I','N','K','E','Y'+0x80,
  'W','I','F','I','S','C','A','N'+0x80,
  'W','I','F','I','C','O','N','N','E','C','T'+0x80,
  'W','I','F','I','S','T','A','T','U','S'+0x80,
  'W','I','F','I','D','I','S','C','O','N','N','E','C','T'+0x80,
  'H','T','T','P','G','E','T'+0x80,
  'C','D'+0x80,
  0
};


static const char * const funcNames[] = {
  "PEEK", "ABS", "AREAD", "DREAD", "RND", "LEN", "VAL", "ASC"
};
#define NUM_FUNC_NAMES (sizeof(funcNames) / sizeof(funcNames[0]))

int matchFuncName(const char *name)
{
  for(unsigned int i = 0; i < NUM_FUNC_NAMES; i++)
    if(strcmp(name, funcNames[i]) == 0)
      return (int)i;
  return -1;
}

const unsigned char to_tab[] PROGMEM = {
  'T','O'+0x80,
  0
};

const unsigned char step_tab[] PROGMEM = {
  'S','T','E','P'+0x80,
  0
};

const unsigned char relop_tab[] PROGMEM = {
  '>','='+0x80,
  '<','>'+0x80,
  '>'+0x80,
  '='+0x80,
  '<','='+0x80,
  '<'+0x80,
  '!','='+0x80,
  0
};

const unsigned char highlow_tab[] PROGMEM = { 
  'H','I','G','H'+0x80,
  'H','I'+0x80,
  'L','O','W'+0x80,
  'L','O'+0x80,
  0
};

const unsigned char okmsg[]            PROGMEM = "OK";
const unsigned char whatmsg[]          PROGMEM = "What? ";
const unsigned char howmsg[]           PROGMEM = "How?";
const unsigned char sorrymsg[]         PROGMEM = "Sorry!";
const unsigned char initmsg[]          PROGMEM = "TinyBasic Plus " kVersion;
const unsigned char memorymsg[]        PROGMEM = "bytes free :";
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
const unsigned char eeprommsg[]        PROGMEM = " EEProm bytes total.";
const unsigned char eepromamsg[]       PROGMEM = " EEProm bytes available.";
#endif
#endif
const unsigned char breakmsg[]         PROGMEM = "break!";
const unsigned char unimplimentedmsg[] PROGMEM = "Unimplemented";
const unsigned char backspacemsg[]     PROGMEM = "\b\e[K";
const unsigned char indentmsg[]        PROGMEM = "    ";
const unsigned char sderrormsg[]       PROGMEM = "SD card error.";
const unsigned char sdfilemsg[]        PROGMEM = "SD file error.";
const unsigned char dirextmsg[]        PROGMEM = "(dir)";
const unsigned char slashmsg[]         PROGMEM = "/";
const unsigned char spacemsg[]         PROGMEM = " ";
