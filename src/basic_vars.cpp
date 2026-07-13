#include "basic_vars.h"
#include "globals.h"
#include "basic_strings.h"
#include <string.h>

struct NumVarEntry {
  char name[MAX_VAR_NAME_LEN + 1];
  short int value;
};

struct StrVarEntry {
  char name[MAX_VAR_NAME_LEN + 1];
  char value[STRVAR_LEN];
};

static NumVarEntry numVars[MAX_NUM_VARS];
static int numVarCount = 0;

static StrVarEntry strVars[MAX_STR_VARS];
static int strVarCount = 0;

int scan_identifier(char *out, boolean *isStr)
{
  *isStr = false;

  if( !(*txtpos >= 'A' && *txtpos <= 'Z') )
  {
    out[0] = 0;
    return 0;
  }

  int i = 0;
  while( (*txtpos >= 'A' && *txtpos <= 'Z') || (*txtpos >= '0' && *txtpos <= '9') )
  {
    if( i < MAX_VAR_NAME_LEN )
      out[i++] = *txtpos;
    txtpos++;
  }
  out[i] = 0;

  if( *txtpos == '$' )
  {
    *isStr = true;
    txtpos++;
  }

  return i;
}

boolean peek_is_string_var(void)
{
  if( !(*txtpos >= 'A' && *txtpos <= 'Z') )
    return false;

  unsigned char *p = txtpos;
  while( (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') )
    p++;

  return (*p == '$');
}

short int *numvar_ref(const char *name)
{
  for(int i = 0; i < numVarCount; i++)
    if(strcmp(numVars[i].name, name) == 0)
      return &numVars[i].value;

  if(numVarCount >= MAX_NUM_VARS)
  {
    // tabela cheia - nao ha como criar mais variaveis numericas. Devolve um
    // ponteiro de emergencia (estatico) pra nao arriscar null-deref; o valor
    // ali e essencialmente descartavel.
    expression_error = 1;
    static short int overflowSlot;
    overflowSlot = 0;
    return &overflowSlot;
  }

  strncpy(numVars[numVarCount].name, name, MAX_VAR_NAME_LEN);
  numVars[numVarCount].name[MAX_VAR_NAME_LEN] = 0;
  numVars[numVarCount].value = 0;
  return &numVars[numVarCount++].value;
}

char *strvar_ref(const char *name)
{
  for(int i = 0; i < strVarCount; i++)
    if(strcmp(strVars[i].name, name) == 0)
      return strVars[i].value;

  if(strVarCount >= MAX_STR_VARS)
  {
    expression_error = 1;
    static char overflowSlot[STRVAR_LEN];
    overflowSlot[0] = 0;
    return overflowSlot;
  }

  strncpy(strVars[strVarCount].name, name, MAX_VAR_NAME_LEN);
  strVars[strVarCount].name[MAX_VAR_NAME_LEN] = 0;
  strVars[strVarCount].value[0] = 0;
  return strVars[strVarCount++].value;
}
