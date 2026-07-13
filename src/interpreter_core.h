#pragma once
#include "config.h"
#include "basic_types.h"
#include "globals.h"
#include "basic_tables.h"

// Funcoes "de motor" do interpretador: leitura/escrita de caracteres,
// parsing de expressoes, manipulacao do buffer de programa em memoria.
// Implementadas em interpreter_core.cpp. Correspondem as funcoes auxiliares
// do .ino original (a maioria era "static", removido aqui pois agora sao
// chamadas tambem a partir de basic_loop.cpp e sd_helpers.cpp).

void print_info();
void ignore_blanks(void);
void scantable(const unsigned char *table);
void printnum(int num);
void printUnum(unsigned int num);
unsigned short testnum(void);
unsigned char print_quoted_string(void);
void printmsgNoNL(const unsigned char *msg);
void printmsg(const unsigned char *msg);
void getln(char prompt);
unsigned char *findline(void);
void toUppercaseBuffer(void);
void printline();
short int expression(void);
unsigned char *filenameWord(void);
void line_terminator(void);
unsigned char breakcheck(void);
void outchar(unsigned char c);
void myWrite(char c);
