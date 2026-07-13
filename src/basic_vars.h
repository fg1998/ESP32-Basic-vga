#pragma once
#include "config.h"

// Tabela de simbolos do BASIC - substitui o esquema antigo de 26 variaveis
// fixas (A..Z, uma letra so) por variaveis com NOME (ate MAX_VAR_NAME_LEN
// caracteres), tanto numericas quanto de string (sufixo $).
//
// Guardunder duas tabelas separadas (uma pra numero, uma pra string) porque
// a linguagem ja distinguia os dois "espacos de nome" por causa do sufixo $
// (A e A$ sempre foram coisas diferentes) - continua assim, so que agora com
// nome comprido nos dois.
//
// Cada variavel, uma vez criada, fica num slot FIXO (nunca move de lugar) -
// isso importa porque o FOR guarda o PONTEIRO da variavel do laco direto no
// stack frame (ver forloop/next em basic_loop.cpp).

#define MAX_VAR_NAME_LEN 10   // maximo de caracteres do nome (sem contar o $)
#define MAX_NUM_VARS     128  // quantas variaveis numericas distintas cabem
#define MAX_STR_VARS     64   // quantas variaveis de string distintas cabem

// Le um identificador comecando em txtpos (letra seguida de letras/digitos),
// consumindo txtpos. Copia (truncado em MAX_VAR_NAME_LEN chars) pra "out"
// (que precisa ter pelo menos MAX_VAR_NAME_LEN+1 bytes) - SEMPRE consome o
// identificador inteiro mesmo se for mais comprido que isso, pra nao deixar
// o parser preso no meio do nome. Se logo depois do nome vier um '$', tambem
// consome o '$' e marca *isStr = true. Devolve o tamanho real do nome (0 se
// txtpos nao comecava com uma letra - nesse caso nada foi consumido).
int scan_identifier(char *out, boolean *isStr);

// Espia (sem consumir txtpos) se o que vem a seguir e uma referencia a
// variavel de string: um identificador (qualquer tamanho) seguido de '$'.
boolean peek_is_string_var(void);

// Acha (ou cria, se ainda nao existir) a variavel numerica de nome "name" e
// devolve um ponteiro estavel pro valor dela. Em caso de tabela cheia,
// marca expression_error e devolve um ponteiro de emergencia (nao trava).
short int *numvar_ref(const char *name);

// Igual numvar_ref(), mas pra variavel de string - devolve ponteiro pro
// buffer de STRVAR_LEN bytes dela (ver basic_strings.h).
char *strvar_ref(const char *name);
