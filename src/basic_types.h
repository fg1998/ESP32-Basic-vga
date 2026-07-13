#pragma once
#include "config.h"

// Tipos e estruturas basicas do interpretador (equivalentes ao .ino original)

typedef short unsigned LINENUM;

// these will select, at runtime, where IO happens through for load/save
enum {
  kStreamSerial = 0,
  kStreamEEProm,
  kStreamFile
};

struct stack_for_frame {
  char frame_type;
  short int *var_ptr;  // ponteiro estavel pra variavel do FOR (era 1 char, "for_var", quando so existiam A..Z)
  short int terminal;
  short int step;
  unsigned char *current_line;
  unsigned char *txtpos;
};

struct stack_gosub_frame {
  char frame_type;
  unsigned char *current_line;
  unsigned char *txtpos;
};

#define STACK_SIZE (sizeof(struct stack_for_frame)*5)
#define VAR_SIZE sizeof(short int) // Size of variables in bytes
#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
