#pragma once
#include "config.h"
#include "basic_types.h"

// Estado global compartilhado do interpretador e objetos de hardware.
// No .ino original a maioria dessas variaveis era "static" (visivel so
// dentro do arquivo). Como agora estao espalhadas em varios .cpp, viraram
// globais "extern" - definidas uma unica vez em globals.cpp.

// --- Hardware (FabGL / SD) ---
// VGA16Controller (16 cores, framebuffer em PSRAM) em vez do VGAController
// "grande" (ate 64 cores, framebuffer preso na SRAM interna - nao cabe
// inteiro em 640x480, corta a tela na metade). Ja usamos so 0-15 nas cores
// do BASIC (COLOR/POINT/LINE/etc), entao bate certinho.
extern fabgl::VGA16Controller VGAController;
extern fabgl::PS2Controller PS2Controller;
extern fabgl::Terminal      Terminal;
extern Canvas cv;
extern TerminalController tc;
extern SPIClass spiSD;
extern File fp;
extern int myScreen;

// --- Buffer de programa e ponteiros de parsing ---
// "program" era um array estatico (unsigned char program[kRamSize]) que ficava
// na SRAM interna. Agora e um ponteiro, alocado em PSRAM dentro do setup()
// (ver main.cpp), pra nao competir por SRAM com WiFi/FabGL/FreeRTOS.
extern unsigned char *program;
extern unsigned char *txtpos, *list_line, *tmptxtpos;
extern unsigned char expression_error;
extern unsigned char *tempsp;

extern unsigned char *stack_limit;
extern unsigned char *program_start;
extern unsigned char *program_end;
extern unsigned char *stack; // Software stack for things that should go on the CPU stack
extern unsigned char *variables_begin;
extern unsigned char *current_line;
extern unsigned char *sp;
extern unsigned char table_index;
extern LINENUM linenum;

// --- Flags de I/O (load/save/EEPROM/etc) ---
extern boolean inhibitOutput;
extern boolean runAfterLoad;
extern boolean triggerRun;
extern unsigned char inStream;
extern unsigned char outStream;