#pragma once
#include "basic_types.h"

// Tabelas de palavras-chave, funcoes internas e mensagens do interpretador.
// Os arrays sao definidos em basic_tables.cpp; aqui ficam so as declaracoes
// extern e os indices/constantes associados (precisam estar visiveis tanto em
// interpreter_core.cpp quanto em basic_loop.cpp).

enum {
  KW_LIST = 0,
  KW_LOAD, KW_NEW, KW_RUN, KW_SAVE,
  KW_NEXT, KW_LET, KW_IF,
  KW_GOTO, KW_GOSUB, KW_RETURN,
  KW_REM,
  KW_FOR,
  KW_INPUT, KW_PRINT,
  KW_POKE,
  KW_STOP, KW_BYE,
  KW_FILES,
  KW_MEM,
  KW_QMARK, KW_QUOTE,
  KW_AWRITE, KW_DWRITE,
  KW_DELAY,
  KW_END,
  KW_RSEED,
  KW_CHAIN,
  KW_CLS,
  KW_COLOR,
  KW_POINT,
  KW_LINE,
  KW_RECTANGLE,
  KW_ELIPSE,
  KW_CURSOR,
  KW_AT,
  KW_INKEY,
  KW_WIFISCAN,
  KW_WIFICONNECT,
  KW_WIFISTATUS,
  KW_WIFIDISCONNECT,
  KW_HTTPGET,
  KW_CD,
  KW_DEFAULT /* always the final one*/
};

extern const unsigned char keywords[] PROGMEM;

// Funcoes numericas com um parametro. Antes eram casadas via scantable()
// contra um PROGMEM prefix-table (igual as palavras-chave de comando); agora
// que variaveis podem ter nome comprido, isso e feito comparando o
// identificador JA ESCANEADO por inteiro (ver expr4() em interpreter_core.cpp
// e matchFuncName() em basic_tables.cpp) - assim "ABSOLUTO" nunca e confundido
// com a funcao "ABS".
#define FUNC_PEEK    0
#define FUNC_ABS     1
#define FUNC_AREAD   2
#define FUNC_DREAD   3
#define FUNC_RND     4
#define FUNC_LEN     5
#define FUNC_VAL     6
#define FUNC_ASC     7

// Devolve o indice FUNC_* de "name" (ja em maiusculas) ou -1 se nao for
// nome de funcao conhecida.
int matchFuncName(const char *name);

extern const unsigned char to_tab[] PROGMEM;
extern const unsigned char step_tab[] PROGMEM;

extern const unsigned char relop_tab[] PROGMEM;
#define RELOP_GE    0
#define RELOP_NE    1
#define RELOP_GT    2
#define RELOP_EQ    3
#define RELOP_LE    4
#define RELOP_LT    5
#define RELOP_NE_BANG   6
#define RELOP_UNKNOWN 7

extern const unsigned char highlow_tab[] PROGMEM;
#define HIGHLOW_HIGH    1
#define HIGHLOW_UNKNOWN 4

// Mensagens
extern const unsigned char okmsg[] PROGMEM;
extern const unsigned char whatmsg[] PROGMEM;
extern const unsigned char howmsg[] PROGMEM;
extern const unsigned char sorrymsg[] PROGMEM;
extern const unsigned char initmsg[] PROGMEM;
extern const unsigned char memorymsg[] PROGMEM;
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
extern const unsigned char eeprommsg[] PROGMEM;
extern const unsigned char eepromamsg[] PROGMEM;
#endif
#endif
extern const unsigned char breakmsg[] PROGMEM;
extern const unsigned char unimplimentedmsg[] PROGMEM;
extern const unsigned char backspacemsg[] PROGMEM;
extern const unsigned char indentmsg[] PROGMEM;
extern const unsigned char sderrormsg[] PROGMEM;
extern const unsigned char sdfilemsg[] PROGMEM;
extern const unsigned char dirextmsg[] PROGMEM;
extern const unsigned char slashmsg[] PROGMEM;
extern const unsigned char spacemsg[] PROGMEM;
