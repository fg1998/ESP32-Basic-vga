#pragma once
#include "config.h"

// Motor de STRING do BASIC. Trabalha em cima da tabela de simbolos de
// basic_vars.h (variaveis por NOME, ate MAX_VAR_NAME_LEN caracteres,
// sufixo $ = string).
//
// Expressoes de string (concatenacao, funcoes) usam uma PILHA fixa de
// buffers "scratch" (STR_SCRATCH_COUNT buffers de STR_SCRATCH_LEN bytes).
// Cada str_expression()/str_term() empilha um buffer pro seu proprio
// resultado e o desempilha assim que o valor ja foi copiado por quem
// chamou - por isso o limite real e a PROFUNDIDADE de aninhamento da
// expressao (nao o numero total de termos), o que da bastante folga com
// STR_SCRATCH_COUNT modesto. Quem chama str_expression()/str_condition()
// de FORA deste arquivo (basic_loop.cpp, interpreter_core.cpp) precisa
// chamar str_scratch_release() depois de terminar de usar o valor
// devolvido, senao a pilha nunca esvazia.

#define STRVAR_LEN       64   // tamanho de cada variavel de string (63 chars + \0)
#define STR_SCRATCH_LEN  64   // tamanho de cada buffer temporario de expressao
#define STR_SCRATCH_COUNT 8   // profundidade maxima de aninhamento suportada

// Armazenamento das variaveis de string em si (por nome) mora em
// basic_vars.h/.cpp (strvar_ref) - as variaveis A$..Z$ de antes viraram
// nomes de ate MAX_VAR_NAME_LEN caracteres.

// true se o texto em txtpos comeca uma expressao de string: literal entre
// aspas, variavel <LETRA>$, ou uma das funcoes LEFT$/RIGHT$/MID$/STR$/CHR$.
// Nao consome txtpos (pode chamar e depois ainda parsear do mesmo ponto).
bool is_string_expr(void);

// Avalia uma expressao de string completa (com "+" para concatenar) a partir
// de txtpos, consumindo o que for lido. Devolve ponteiro pra um buffer da
// pilha interna - use o valor e depois chame str_scratch_release() (a menos
// que voce esteja dentro do proprio motor de strings, que ja cuida disso
// internamente).
char *str_expression(void);

// Avalia uma condicao "expr_string RELOP expr_string" (usado pelo IF) e
// devolve 1/0 como a expression() numerica faz. Assume que is_string_expr()
// ja confirmou que da pra chamar isso no ponto atual de txtpos. Ja libera
// os buffers que usa internamente - nao precisa chamar str_scratch_release()
// depois desta.
short int str_condition(void);

// Libera (desempilha) o buffer devolvido pela ultima chamada "de fora" a
// str_expression() - chamar depois de terminar de usar o valor devolvido
// (ex: depois do strncpy/printmsgNoNL/strlen que consumiu o resultado).
void str_scratch_release(void);
