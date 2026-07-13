#include "globals.h"

// Definicoes unicas de todo o estado global (ver globals.h)

fabgl::VGA16Controller VGAController;
fabgl::PS2Controller PS2Controller;
fabgl::Terminal      Terminal;
Canvas cv(&VGAController);
TerminalController tc(&Terminal);
SPIClass spiSD(HSPI);
File fp;
int myScreen;

char eliminateCompileErrors = 1;  // fix to suppress arduino build errors (mantido do original)

// "program" e alocado em PSRAM dentro do setup() (main.cpp), nao aqui -
// precisa do heap ja inicializado antes de chamar ps_malloc().
unsigned char *program = nullptr;
static const char * sentinel = "HELLO"; // nao usado no restante do codigo; mantido do original
unsigned char *txtpos, *list_line, *tmptxtpos;
unsigned char expression_error;
unsigned char *tempsp;

unsigned char *stack_limit;
unsigned char *program_start;
unsigned char *program_end;
unsigned char *stack;
unsigned char *variables_begin;
unsigned char *current_line;
unsigned char *sp;
unsigned char table_index;
LINENUM linenum;

boolean inhibitOutput = false;
boolean runAfterLoad = false;
boolean triggerRun = false;
unsigned char inStream = kStreamSerial;
unsigned char outStream = kStreamSerial;