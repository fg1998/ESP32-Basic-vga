#include "basic_strings.h"
#include "globals.h"
#include "basic_tables.h"
#include "interpreter_core.h"
#include "basic_vars.h"
#include <string.h>
#include <stdio.h>

static char strScratch[STR_SCRATCH_COUNT][STR_SCRATCH_LEN];
static int scratchTop = 0;

static char *push_scratch(void)
{
  if(scratchTop >= STR_SCRATCH_COUNT)
    scratchTop = STR_SCRATCH_COUNT - 1; // clamp de seguranca (aninhamento absurdo) - nao deveria acontecer em uso normal
  char *b = strScratch[scratchTop++];
  b[0] = 0;
  return b;
}

static void pop_scratch(void)
{
  if(scratchTop > 0)
    scratchTop--;
}

void str_scratch_release(void)
{
  pop_scratch();
}

// compara um prefixo literal (ja em maiusculas, igual o resto do BASIC) sem
// consumir txtpos - so pra "espiar" o que vem a seguir.
static boolean starts_with(const unsigned char *s, const char *kw)
{
  return strncmp((const char *)s, kw, strlen(kw)) == 0;
}

static char *str_term(void)
{
  ignore_blanks();
  char *result = push_scratch();

  // literal entre aspas
  if(*txtpos == '"' || *txtpos == '\'')
  {
    unsigned char delim = *txtpos;
    txtpos++;
    int i = 0;
    while(*txtpos != delim && *txtpos != NL && i < STR_SCRATCH_LEN - 1)
      result[i++] = *txtpos++;
    result[i] = 0;
    // se o literal for mais comprido que o buffer, ainda assim consome o
    // resto dos caracteres ate achar o delimitador - senao o parser fica
    // preso no meio do texto. So o VALOR guardado e truncado, o parsing
    // continua normalmente.
    while(*txtpos != delim && *txtpos != NL)
      txtpos++;
    if(*txtpos == delim)
      txtpos++;
    else
      expression_error = 1; // aspas nunca fechou (foi ate o fim da linha)
    return result;
  }

  // literal entre aspas ja tratado acima; a partir daqui: funcoes fixas
  // (LEFT$/RIGHT$/MID$/STR$/CHR$) sao checadas ANTES da variavel com nome
  // generico, senao "LEFT$(" seria lido como a variavel "LEFT" seguida de
  // um "(" solto.

  // LEFT$(str, n)
  if(starts_with(txtpos, "LEFT$("))
  {
    txtpos += 6;
    char src[STR_SCRATCH_LEN];
    strncpy(src, str_expression(), STR_SCRATCH_LEN - 1);
    src[STR_SCRATCH_LEN - 1] = 0;
    pop_scratch(); // libera o buffer que str_expression() usou pro argumento
    if(*txtpos != ',') { expression_error = 1; return result; }
    txtpos++;
    expression_error = 0;
    short int n = expression();
    if(*txtpos != ')') { expression_error = 1; return result; }
    txtpos++;
    int len = strlen(src);
    if(n < 0) n = 0;
    if(n > len) n = len;
    strncpy(result, src, n);
    result[n] = 0;
    return result;
  }

  // RIGHT$(str, n)
  if(starts_with(txtpos, "RIGHT$("))
  {
    txtpos += 7;
    char src[STR_SCRATCH_LEN];
    strncpy(src, str_expression(), STR_SCRATCH_LEN - 1);
    src[STR_SCRATCH_LEN - 1] = 0;
    pop_scratch();
    if(*txtpos != ',') { expression_error = 1; return result; }
    txtpos++;
    expression_error = 0;
    short int n = expression();
    if(*txtpos != ')') { expression_error = 1; return result; }
    txtpos++;
    int len = strlen(src);
    if(n < 0) n = 0;
    if(n > len) n = len;
    strncpy(result, src + (len - n), n);
    result[n] = 0;
    return result;
  }

  // MID$(str, start [, len])  -- start e 1-based, como no BASIC classico
  if(starts_with(txtpos, "MID$("))
  {
    txtpos += 5;
    char src[STR_SCRATCH_LEN];
    strncpy(src, str_expression(), STR_SCRATCH_LEN - 1);
    src[STR_SCRATCH_LEN - 1] = 0;
    pop_scratch();
    if(*txtpos != ',') { expression_error = 1; return result; }
    txtpos++;
    expression_error = 0;
    short int start = expression();
    short int len = -1;
    ignore_blanks();
    if(*txtpos == ',')
    {
      txtpos++;
      len = expression();
    }
    if(*txtpos != ')') { expression_error = 1; return result; }
    txtpos++;
    int slen = strlen(src);
    if(start < 1) start = 1;
    if(start > slen) { result[0] = 0; return result; }
    int avail = slen - (start - 1);
    int take = (len < 0 || len > avail) ? avail : len;
    strncpy(result, src + (start - 1), take);
    result[take] = 0;
    return result;
  }

  // STR$(n) - numero pra string
  if(starts_with(txtpos, "STR$("))
  {
    txtpos += 5;
    expression_error = 0;
    short int n = expression();
    if(*txtpos != ')') { expression_error = 1; return result; }
    txtpos++;
    snprintf(result, STR_SCRATCH_LEN, "%d", n);
    return result;
  }

  // CHR$(n) - codigo ASCII pra string de 1 char
  if(starts_with(txtpos, "CHR$("))
  {
    txtpos += 5;
    expression_error = 0;
    short int n = expression();
    if(*txtpos != ')') { expression_error = 1; return result; }
    txtpos++;
    result[0] = (char)n;
    result[1] = 0;
    return result;
  }

  // variavel de string com nome (ex: NOME$) - checada DEPOIS das funcoes
  // fixas acima de proposito: "LEFT$(" precisa ser reconhecido como funcao
  // antes de tentarmos ler "LEFT" como nome de variavel.
  if( peek_is_string_var() )
  {
    char name[MAX_VAR_NAME_LEN + 1];
    boolean isStr;
    scan_identifier(name, &isStr); // isStr vai dar true, ja consome o "$" tambem
    strncpy(result, strvar_ref(name), STR_SCRATCH_LEN - 1);
    result[STR_SCRATCH_LEN - 1] = 0;
    return result;
  }

  expression_error = 1;
  result[0] = 0;
  return result;
}

bool is_string_expr(void)
{
  if(*txtpos == '"' || *txtpos == '\'')
    return true;
  if(peek_is_string_var())
    return true;
  if(starts_with(txtpos, "LEFT$(")  || starts_with(txtpos, "RIGHT$(") ||
     starts_with(txtpos, "MID$(")   || starts_with(txtpos, "STR$(")   ||
     starts_with(txtpos, "CHR$("))
    return true;
  return false;
}

char *str_expression(void)
{
  char *result = push_scratch();
  char *part = str_term();
  strncpy(result, part, STR_SCRATCH_LEN - 1);
  result[STR_SCRATCH_LEN - 1] = 0;
  pop_scratch(); // libera o buffer do primeiro termo, ja copiado pra "result"

  ignore_blanks();
  while(*txtpos == '+')
  {
    txtpos++;
    ignore_blanks();
    part = str_term();
    int len = strlen(result);
    strncat(result, part, STR_SCRATCH_LEN - 1 - len);
    pop_scratch(); // libera o buffer desse termo, ja concatenado em "result"
    ignore_blanks();
  }
  return result; // NAO desempilhado aqui - quem chamou precisa liberar depois de usar
}

short int str_condition(void)
{
  char lhs[STR_SCRATCH_LEN];
  strncpy(lhs, str_expression(), STR_SCRATCH_LEN - 1);
  lhs[STR_SCRATCH_LEN - 1] = 0;
  pop_scratch(); // libera o buffer do lado esquerdo, ja copiado pra "lhs" local

  scantable(relop_tab);
  if(table_index == RELOP_UNKNOWN)
  {
    expression_error = 1;
    return 0;
  }
  unsigned char op = table_index;

  char *rhs = str_expression();
  int cmp = strcmp(lhs, rhs);
  pop_scratch(); // libera o buffer do lado direito, ja usado no strcmp

  switch(op)
  {
    case RELOP_EQ: return cmp == 0;
    case RELOP_NE:
    case RELOP_NE_BANG: return cmp != 0;
    case RELOP_GT: return cmp > 0;
    case RELOP_GE: return cmp >= 0;
    case RELOP_LT: return cmp < 0;
    case RELOP_LE: return cmp <= 0;
  }
  return 0;
}
