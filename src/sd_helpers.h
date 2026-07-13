#pragma once
#include "config.h"

// Funcoes de apoio ao cartao SD (equivalentes ao trecho "SD Card helpers"
// do .ino original), agora com nocao de "diretorio atual" pra CD/FILES/LOAD/SAVE.

#define MAX_PATH_LEN 64

// Caminho absoluto do diretorio atual (comeca em "/"). LOAD/SAVE resolvem
// nomes relativos contra isso; FILES lista esse diretorio.
extern char currentDir[MAX_PATH_LEN];

// Resolve "name" pra um caminho absoluto em "out" (out precisa ter pelo
// menos MAX_PATH_LEN bytes). Se "name" ja comecar com '/', usa como esta
// (absoluto); senao, concatena com currentDir.
void resolvePath(const char *name, char *out);

// Tenta mudar currentDir pra "name" (aceita "/", "..", nome relativo ou
// caminho absoluto). Retorna true se o diretorio existe e a troca foi feita.
bool changeDir(const char *name);

#if ARDUINO && ENABLE_FILEIO
int initSD(void);
#endif

#if ENABLE_FILEIO
void cmd_Files(void);
#endif
