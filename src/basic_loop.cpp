#include "config.h"
#include "basic_types.h"
#include "globals.h"
#include "basic_tables.h"
#include "interpreter_core.h"
#include "sd_helpers.h"
#include "basic_loop.h"
#include "basic_strings.h"
#include "basic_vars.h"
#include <string.h>

// Este arquivo concentra os COMANDOS do BASIC: e o antigo void loop() do
// .ino original, renomeado para basic_run(). Mantido como uma unica funcao
// de proposito porque o interpretador usa "goto" para saltar entre os
// rotulos de cada comando (input, forloop, gosub, print, cls, color, point,
// line, rectangle, elipse, wifi_*, httpget, cursor, at, etc.) - separar cada
// comando em uma funcao própria exigiria reescrever essa logica de controle,
// o que nao foi pedido. So a organizacao em arquivos mudou.

void basic_run()
{
  Serial.println("&with &V&G&A&x&& video output"); 
  //Serial.println("Version beta 1.0"); 
  unsigned char *start;
  unsigned char *newEnd;
  unsigned char linelen;
  boolean isDigital;
  boolean alsoWait = false;
  int val;
  short int *nextVarPtr; // usado por next:/gosub_return (era 1 char, "for_var")

  program_start = program;
  program_end = program_start;
  sp = program+kRamSize;  // Needed for printnum
  stack_limit = program+kRamSize-STACK_SIZE;
  variables_begin = stack_limit - 27*VAR_SIZE;


#ifdef ARDUINO
#ifdef ENABLE_EEPROM
  // eprom size
  printnum( E2END+1 );
  printmsg( eeprommsg );
#endif /* ENABLE_EEPROM */
#endif /* ARDUINO */

warmstart:
  // this signifies that it is running in 'direct' mode.
  current_line = 0;
  sp = program+kRamSize;
  printmsg(okmsg);

prompt:
  if( triggerRun ){
    triggerRun = false;
    current_line = program_start;
    goto execline;
  }

  getln( '>' );
  toUppercaseBuffer();

  txtpos = program_end+sizeof(unsigned short);

  // Find the end of the freshly entered line
  while(*txtpos != NL)
    txtpos++;

  // Move it to the end of program_memory
  {
    unsigned char *dest;
    dest = variables_begin-1;
    while(1)
    {
      *dest = *txtpos;
      if(txtpos == program_end+sizeof(unsigned short))
        break;
      dest--;
      txtpos--;
    }
    txtpos = dest;
  }

  // Now see if we have a line number
  linenum = testnum();
  ignore_blanks();
  if(linenum == 0)
    goto direct;

  if(linenum == 0xFFFF)
    goto qhow;

  // Find the length of what is left, including the (yet-to-be-populated) line header
  linelen = 0;
  while(txtpos[linelen] != NL)
    linelen++;
  linelen++; // Include the NL in the line length
  linelen += sizeof(unsigned short)+sizeof(char); // Add space for the line number and line length

  // Now we have the number, add the line header.
  txtpos -= 3;
  *((unsigned short *)txtpos) = linenum;
  txtpos[sizeof(LINENUM)] = linelen;


  // Merge it into the rest of the program
  start = findline();

  // If a line with that number exists, then remove it
  if(start != program_end && *((LINENUM *)start) == linenum)
  {
    unsigned char *dest, *from;
    unsigned tomove;

    from = start + start[sizeof(LINENUM)];
    dest = start;

    tomove = program_end - from;
    while( tomove > 0)
    {
      *dest = *from;
      from++;
      dest++;
      tomove--;
    } 
    program_end = dest;
  }

  if(txtpos[sizeof(LINENUM)+sizeof(char)] == NL) // If the line has no txt, it was just a delete
    goto prompt;



  // Make room for the new line, either all in one hit or lots of little shuffles
  while(linelen > 0)
  { 
    unsigned int tomove;
    unsigned char *from,*dest;
    unsigned int space_to_make;

    space_to_make = txtpos - program_end;

    if(space_to_make > linelen)
      space_to_make = linelen;
    newEnd = program_end+space_to_make;
    tomove = program_end - start;


    // Source and destination - as these areas may overlap we need to move bottom up
    from = program_end;
    dest = newEnd;
    while(tomove > 0)
    {
      from--;
      dest--;
      *dest = *from;
      tomove--;
    }

    // Copy over the bytes into the new space
    for(tomove = 0; tomove < space_to_make; tomove++)
    {
      *start = *txtpos;
      txtpos++;
      start++;
      linelen--;
    }
    program_end = newEnd;
  }
  goto prompt;

unimplemented:
  printmsg(unimplimentedmsg);
  goto prompt;

qhow: 
  printmsg(howmsg);
  goto prompt;

qwhat:  
  printmsgNoNL(whatmsg);
  if(current_line != NULL)
  {
    unsigned char tmp = *txtpos;
    if(*txtpos != NL)
      *txtpos = '^';
    list_line = current_line;
    printline();
    *txtpos = tmp;
  }
  line_terminator();
  goto prompt;

qsorry: 
  printmsg(sorrymsg);
  goto warmstart;

run_next_statement:
  while(*txtpos == ':')
    txtpos++;
  ignore_blanks();
  if(*txtpos == NL)
    goto execnextline;
  goto interperateAtTxtpos;

direct: 
  txtpos = program_end+sizeof(LINENUM);
  if(*txtpos == NL)
    goto prompt;

interperateAtTxtpos:
  if(breakcheck())
  {
    printmsg(breakmsg);
    goto warmstart;
  }

  scantable(keywords);

  switch(table_index)
  {
  case KW_DELAY:
    {
#ifdef ARDUINO
      expression_error = 0;
      val = expression();
      delay( val );
      goto execnextline;
#else
      goto unimplemented;
#endif
    }

  case KW_FILES:
    goto files;
  case KW_LIST:
    goto list;
  case KW_CHAIN:
    goto chain;
  case KW_LOAD:
    goto load;
  case KW_MEM:
    goto mem;
  case KW_NEW:
    if(txtpos[0] != NL)
      goto qwhat;
    program_end = program_start;
    Terminal.setForegroundColor(fabgl::Color::BrightWhite);
    Terminal.setBackgroundColor(fabgl::Color::Black);
    
    Terminal.clear();
    print_info();
    goto prompt;
  case KW_RUN:
    current_line = program_start;
    goto execline;
  case KW_SAVE:
    goto save;
  case KW_NEXT:
    goto next;
  case KW_LET:
    goto assignment;
  case KW_IF:
    short int val;
    expression_error = 0;
    if( is_string_expr() )
      val = str_condition();
    else
      val = expression();
    if(expression_error || *txtpos == NL)
      goto qhow;
    if(val != 0)
      goto interperateAtTxtpos;
    goto execnextline;

  case KW_GOTO:
    expression_error = 0;
    linenum = expression();
    if(expression_error || *txtpos != NL)
      goto qhow;
    current_line = findline();
    goto execline;

  case KW_GOSUB:
    goto gosub;
  case KW_RETURN:
    goto gosub_return; 
  case KW_REM:
  case KW_QUOTE:
    goto execnextline;  // Ignore line completely
  case KW_FOR:
    goto forloop; 
  case KW_INPUT:
    goto input; 
  case KW_PRINT:
  case KW_QMARK:
    goto print;
  case KW_POKE:
    goto poke;
  case KW_END:
  case KW_STOP:
    // This is the easy way to end - set the current line to the end of program attempt to run it
    if(txtpos[0] != NL)
      goto qwhat;
    current_line = program_end;
    goto execline;
  case KW_BYE:
    // Leave the basic interperater
    return;

  case KW_AWRITE:  // AWRITE <pin>, HIGH|LOW
    isDigital = false;
    goto awrite;
  case KW_DWRITE:  // DWRITE <pin>, HIGH|LOW
    isDigital = true;
    goto dwrite;

  case KW_RSEED:
    goto rseed;
  case KW_CLS:
    goto cls;
  case KW_COLOR:
    goto color;
  case KW_POINT:
    goto point;
  case KW_LINE:
    goto line;
  case  KW_RECTANGLE:
    goto rectangle;
  case KW_ELIPSE:
    goto elipse;
  case KW_CURSOR:
    goto cursor;
  case KW_AT:
    goto at;
  case KW_WIFISCAN:
      goto wifi_scan;
  case KW_WIFICONNECT:
      goto wifi_connect;
  case KW_WIFISTATUS:
      goto wifi_status;
  case KW_WIFIDISCONNECT:
      goto wifi_disconnect;
  case KW_HTTPGET:
      goto httpget;
  case KW_CD:
      goto cd_cmd;
  case KW_DEFAULT:
    goto assignment;
  default:
    break;
  }

execnextline:
  if(current_line == NULL)    // Processing direct commands?
    goto prompt;
  current_line +=  current_line[sizeof(LINENUM)];

execline:
  if(current_line == program_end) // Out of lines to run
    goto warmstart;
  txtpos = current_line+sizeof(LINENUM)+sizeof(char);
  goto interperateAtTxtpos;

#ifdef ARDUINO
#ifdef ENABLE_EEPROM
elist:
  {
    int i;
    for( i = 0 ; i < (E2END +1) ; i++ )
    {
      val = EEPROM.read( i );

      if( val == '\0' ) {
        goto execnextline;
      }

      if( ((val < ' ') || (val  > '~')) && (val != NL) && (val != CR))  {
        outchar( '?' );
      } 
      else {
        outchar( val );
      }
    }
  }
  goto execnextline;

eformat:
  {
    for( int i = 0 ; i < E2END ; i++ )
    {
      if( (i & 0x03f) == 0x20 ) outchar( '.' );
      EEPROM.write( i, 0 );
    }
    outchar( LF );
  }
  goto execnextline;

esave:
  {
    outStream = kStreamEEProm;
    eepos = 0;

    // copied from "List"
    list_line = findline();
    while(list_line != program_end) {
      printline();
    }
    outchar('\0');

    // go back to standard output, close the file
    outStream = kStreamSerial;
    
    goto warmstart;
  }
  
  
echain:
  runAfterLoad = true;

eload:
  // clear the program
  program_end = program_start;

  // load from a file into memory
  eepos = 0;
  inStream = kStreamEEProm;
  inhibitOutput = true;
  goto warmstart;
#endif /* ENABLE_EEPROM */
#endif

input:
  {
    char varName[MAX_VAR_NAME_LEN + 1];
    int value;
    boolean isStringInput = false;

    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    scan_identifier(varName, &isStringInput);
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;

    if( isStringInput )
    {
      tmptxtpos = txtpos;
      getln( '?' );
      // string: le a linha digitada sem passar por toUppercaseBuffer, pra
      // preservar maiusculas/minusculas do que o usuario digitou
      {
        unsigned char *p = program_end+sizeof(unsigned short);
        char *dst = strvar_ref(varName);
        int i = 0;
        while( *p != NL && i < STRVAR_LEN-1 )
          dst[i++] = *p++;
        dst[i] = 0;
      }
      txtpos = tmptxtpos;
      goto run_next_statement;
    }

inputagain:
    tmptxtpos = txtpos;
    getln( '?' );
    toUppercaseBuffer();
    txtpos = program_end+sizeof(unsigned short);
    ignore_blanks();
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto inputagain;
    *numvar_ref(varName) = value;
    txtpos = tmptxtpos;

    goto run_next_statement;
  }

forloop:
  {
    char varName[MAX_VAR_NAME_LEN + 1];
    boolean isStr;
    short int initial, step, terminal;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    scan_identifier(varName, &isStr);
    if(isStr)
      goto qwhat; // FOR nao funciona com variavel de string
    ignore_blanks();
    if(*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    expression_error = 0;
    initial = expression();
    if(expression_error)
      goto qwhat;

    scantable(to_tab);
    if(table_index != 0)
      goto qwhat;

    terminal = expression();
    if(expression_error)
      goto qwhat;

    scantable(step_tab);
    if(table_index == 0)
    {
      step = expression();
      if(expression_error)
        goto qwhat;
    }
    else
      step = 1;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;


    if(!expression_error && *txtpos == NL)
    {
      struct stack_for_frame *f;
      if(sp + sizeof(struct stack_for_frame) < stack_limit)
        goto qsorry;

      sp -= sizeof(struct stack_for_frame);
      f = (struct stack_for_frame *)sp;
      short int *varptr = numvar_ref(varName);
      *varptr = initial;
      f->frame_type = STACK_FOR_FLAG;
      f->var_ptr = varptr;
      f->terminal = terminal;
      f->step     = step;
      f->txtpos   = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  goto qhow;

gosub:
  expression_error = 0;
  linenum = expression();
  if(!expression_error && *txtpos == NL)
  {
    struct stack_gosub_frame *f;
    if(sp + sizeof(struct stack_gosub_frame) < stack_limit)
      goto qsorry;

    sp -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)sp;
    f->frame_type = STACK_GOSUB_FLAG;
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = findline();
    goto execline;
  }
  goto qhow;

next:
  // Fnd the variable name
  ignore_blanks();
  if(*txtpos < 'A' || *txtpos > 'Z')
    goto qhow;
  {
    char nextName[MAX_VAR_NAME_LEN + 1];
    boolean nextIsStr;
    scan_identifier(nextName, &nextIsStr);
    if(nextIsStr)
      goto qwhat; // NEXT nao funciona com variavel de string
    nextVarPtr = numvar_ref(nextName);
  }
  ignore_blanks();
  if(*txtpos != ':' && *txtpos != NL)
    goto qwhat;

gosub_return:
  // Now walk up the stack frames and find the frame we want, if present
  tempsp = sp;
  while(tempsp < program+kRamSize-1)
  {
    switch(tempsp[0])
    {
    case STACK_GOSUB_FLAG:
      if(table_index == KW_RETURN)
      {
        struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
        current_line  = f->current_line;
        txtpos      = f->txtpos;
        sp += sizeof(struct stack_gosub_frame);
        goto run_next_statement;
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_gosub_frame);
      break;
    case STACK_FOR_FLAG:
      // Flag, Var, Final, Step
      if(table_index == KW_NEXT)
      {
        struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
        // Is the the variable we are looking for? (compara o PONTEIRO da
        // variavel, ja que agora o nome pode ter mais de uma letra - era
        // "txtpos[-1] == f->for_var" quando so existiam variaveis A..Z)
        if(nextVarPtr == f->var_ptr)
        {
          short int *varaddr = f->var_ptr;
          *varaddr = *varaddr + f->step;
          // Use a different test depending on the sign of the step increment
          if((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal))
          {
            // We have to loop so don't pop the stack
            txtpos = f->txtpos;
            current_line = f->current_line;
            goto run_next_statement;
          }
          // We've run to the end of the loop. drop out of the loop, popping the stack
          sp = tempsp + sizeof(struct stack_for_frame);
          goto run_next_statement;
        }
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_for_frame);
      break;
    default:
      //printf("Stack is stuffed!\n");
      goto warmstart;
    }
  }
  // Didn't find the variable we've been looking for
  goto qhow;

assignment:
  {
    //Variavel só pode começar com A..Z
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qhow;

    char varName[MAX_VAR_NAME_LEN + 1];
    boolean isStr;
    scan_identifier(varName, &isStr);

    if(isStr)
    {
      // atribuicao de STRING: NOME$ = <expressao de string>
      ignore_blanks();
      if(*txtpos != '=')
        goto qwhat;
      txtpos++;
      ignore_blanks();
      expression_error = 0;
      char *s = str_expression();
      if(expression_error)
        goto qwhat;
      if(*txtpos != NL && *txtpos != ':')
        goto qwhat;
      char *dst = strvar_ref(varName);
      strncpy(dst, s, STRVAR_LEN - 1);
      dst[STRVAR_LEN - 1] = 0;
      str_scratch_release();
      goto run_next_statement;
    }

    short int value;
    short int *var;

    var = numvar_ref(varName);

    ignore_blanks();

    if (*txtpos != '=')
      goto qwhat;

    txtpos++;
    ignore_blanks();
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    // Check that we are at the end of the statement
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
    *var = value;
  }
  goto run_next_statement;
poke:
  {
    short int value;
    unsigned char *address;

    // Work out where to put it
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    address = (unsigned char *)value;

    // check for a comma
    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    // Now get the value to assign
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    //printf("Poke %p value %i\n",address, (unsigned char)value);
    // Check that we are at the end of the statement
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
  }
  goto run_next_statement;

list:
  {
    LINENUM startLine, endLine;
    boolean hasEnd = false;

    startLine = testnum(); // Retuns 0 if no line found (lista desde o inicio)
    ignore_blanks();
    if(*txtpos == '-')
    {
      // LIST <inicio>-<fim>  ou  LIST <inicio>-  ou  LIST -<fim>
      txtpos++;
      hasEnd = true;
      endLine = testnum(); // 0 = sem limite superior (lista ate o fim)
    }

    // Should be EOL
    if(txtpos[0] != NL)
      goto qwhat;

    // Find the line
    linenum = startLine;
    list_line = findline();
    while(list_line != program_end)
    {
      if(hasEnd && endLine != 0)
      {
        LINENUM curLine = *((LINENUM *)list_line);
        if(curLine > endLine)
          break;
      }
      printline();
    }
  }
  goto warmstart;

print:
  // If we have an empty list then just put out a NL
  if(*txtpos == ':' )
  {
    line_terminator();
    txtpos++;
    goto run_next_statement;
  }
  if(*txtpos == NL)
  {
    goto execnextline;
  }

  while(1)
  {
    ignore_blanks();
    if(is_string_expr())
    {
      // cobre literal entre aspas, variavel X$ e as funcoes LEFT$/RIGHT$/MID$/
      // STR$/CHR$ - e ja da conta de concatenar com "+" (ex: "X: " + A$).
      // Limitado ao tamanho do buffer scratch (STR_SCRATCH_LEN); pra strings
      // literais mais compridas que isso, ver basic_strings.h.
      expression_error = 0;
      char *s = str_expression();
      if(expression_error)
        goto qwhat;
      printmsgNoNL((const unsigned char *)s);
      str_scratch_release();
    }
    else
    {
      short int e;
      expression_error = 0;
      e = expression();
      if(expression_error)
        goto qwhat;
      printnum(e);
    }

    // At this point we have three options, a comma or a new line
    if(*txtpos == ',')
      txtpos++; // Skip the comma and move onto the next
    else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':'))
    {
      txtpos++; // This has to be the end of the print - no newline
      break;
    }
    else if(*txtpos == NL || *txtpos == ':')
    {
      line_terminator();  // The end of the print statement
      break;
    }
    else
      goto qwhat; 
  }
  goto run_next_statement;

mem:
  // tamanho total do buffer de programa (era memoria livre: variables_begin-program_end)
  printnum(kRamSize);
  printmsg(memorymsg);
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
  {
    // eprom size
    printnum( E2END+1 );
    printmsg( eeprommsg );
    
    // figure out the memory usage;
    val = ' ';
    int i;   
    for( i=0 ; (i<(E2END+1)) && (val != '\0') ; i++ ) {
      val = EEPROM.read( i );    
    }
    printnum( (E2END +1) - (i-1) );
    
    printmsg( eepromamsg );
  }
#endif /* ENABLE_EEPROM */
#endif /* ARDUINO */
  goto run_next_statement;


  /*************************************************/

#ifdef ARDUINO
awrite: // AWRITE <pin>,val
dwrite:
  {
    short int pinNo;
    short int value;
    unsigned char *txtposBak;

    // Get the pin number
    expression_error = 0;
    pinNo = expression();
    if(expression_error)
      goto qwhat;

    // check for a comma
    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();


    txtposBak = txtpos; 
    scantable(highlow_tab);
    if(table_index != HIGHLOW_UNKNOWN)
    {
      if( table_index <= HIGHLOW_HIGH ) {
        value = 1;
      } 
      else {
        value = 0;
      }
    } 
    else {

      // and the value (numerical)
      expression_error = 0;
      value = expression();
      if(expression_error)
        goto qwhat;
    }
    pinMode( pinNo, OUTPUT );
    if( isDigital ) {
      digitalWrite( pinNo, value );
    } 
    else {
      //analogWrite( pinNo, value );
    }
  }
  goto run_next_statement;
#else
pinmode: // PINMODE <pin>, I/O
awrite: // AWRITE <pin>,val
dwrite:
  goto unimplemented;
#endif

  /*************************************************/
cd_cmd:
  // CD <diretorio> - muda o diretorio atual do SD (usado por FILES/LOAD/SAVE)
  // aceita "/" (raiz), ".." (sobe um nivel) ou nome relativo/absoluto
#ifdef ENABLE_FILEIO
  {
    unsigned char *dirname;
    expression_error = 0;
    dirname = filenameWord();
    if(expression_error)
      goto qwhat;
    if( !changeDir((const char *)dirname) )
      Terminal.write("\r\nDiretorio nao encontrado.\r\n");
    else
      Terminal.printf("\r\nOK: %s\r\n", currentDir);
  }
  goto warmstart;
#else
  goto unimplemented;
#endif

files:
  // display a listing of files on the device.
  // version 1: no support for subdirectories

#ifdef ENABLE_FILEIO
    cmd_Files();
  goto warmstart;
#else
  goto unimplemented;
#endif // ENABLE_FILEIO


chain:
  runAfterLoad = true;

load:
  // clear the program
  program_end = program_start;

  // load from a file into memory
#ifdef ENABLE_FILEIO
  {
    unsigned char *filename;

    // Work out the filename
    expression_error = 0;
    filename = filenameWord();
    if(expression_error)
      goto qwhat;

    char loadPath[MAX_PATH_LEN];
    resolvePath((const char *)filename, loadPath);

#ifdef ARDUINO
    // Arduino specific
    if( !SD.exists( loadPath ))
    {
      printmsg( sdfilemsg );
    } 
    else {

      fp = SD.open( loadPath );
      inStream = kStreamFile;
      inhibitOutput = true;
    }
#else // ARDUINO
    // Desktop specific
#endif // ARDUINO
    // this will kickstart a series of events to read in from the file.

  }
  goto warmstart;
#else // ENABLE_FILEIO
  goto unimplemented;
#endif // ENABLE_FILEIO






save:
  {

    unsigned char *filename;

    // Work out the filename
    expression_error = 0;
    filename = filenameWord();
    if(expression_error)
      goto qwhat;

    char savePath[MAX_PATH_LEN];
    resolvePath((const char *)filename, savePath);

#ifdef ARDUINO
    // remove the old file if it exists
    if( SD.exists( savePath )) {
      SD.remove( savePath );
    }

    // open the file, switch over to file output
    fp = SD.open( savePath, FILE_WRITE );
        if(!fp){
      Serial.println("Erro na abertura do arquivo");
    }
    outStream = kStreamFile;

    // copied from "List"
    list_line = findline();
    while(list_line != program_end)
      printline();

    // go back to standard output, close the file
    outStream = kStreamSerial;

    fp.close();
#else // ARDUINO
    // desktop
#endif // ARDUINO
    goto warmstart;
  }








rseed:
  {
    short int value;

    //Get the pin number
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;


  randomSeed( value );

    goto run_next_statement;
  }

cls:
{
  Terminal.clear();
  //tc.clear();
  //tc.setCursorPos(0,0);
  goto run_next_statement;
} 


color:
  {
    short int foreColor;
    short int backColor;

    //Get foreColor
    expression_error = 0;
    foreColor = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the backColor
    expression_error = 0;
    backColor = expression();
    if(expression_error)
      goto qwhat;

    switch (foreColor)
    {
    case 0:
      Terminal.setForegroundColor(fabgl::Color::Red);
      break;
    case 1:
      Terminal.setForegroundColor(fabgl::Color::Green);
      break;
    case 2:
      Terminal.setForegroundColor(fabgl::Color::Yellow);
      break;
    case 3:
      Terminal.setForegroundColor(fabgl::Color::Blue);
      break;
    case 4:
      Terminal.setForegroundColor(fabgl::Color::Magenta);
      break;
    case 5:
      Terminal.setForegroundColor(fabgl::Color::Cyan);
      break;
    case 6:
      Terminal.setForegroundColor(fabgl::Color::White);
      break;
    case 7:
      Terminal.setForegroundColor(fabgl::Color::BrightBlack);
      break;
    case 8:
      Terminal.setForegroundColor(fabgl::Color::BrightRed);
      break;
    case 9:
      Terminal.setForegroundColor(fabgl::Color::BrightGreen);
      break;
    case 10:
      Terminal.setForegroundColor(fabgl::Color::BrightYellow);
      break;
    case 11:
      Terminal.setForegroundColor(fabgl::Color::BrightBlue);
      break;
    case 12:
      Terminal.setForegroundColor(fabgl::Color::BrightMagenta);
      break;
    case 13:
      Terminal.setForegroundColor(fabgl::Color::BrightCyan);
      break;
    case 14:
      Terminal.setForegroundColor(fabgl::Color::BrightWhite);
      break;
    case 15:
      Terminal.setForegroundColor(fabgl::Color::Black);
      break;
    default:
      Terminal.setForegroundColor(fabgl::Color::BrightBlack);
      break;

    }
    
    switch (backColor)
    {
    case 0:
      Terminal.setBackgroundColor(fabgl::Color::Red);
      break;
    case 1:
      Terminal.setBackgroundColor(fabgl::Color::Green);
      break;
    case 2:
      Terminal.setBackgroundColor(fabgl::Color::Yellow);
      break;
    case 3:
      Terminal.setBackgroundColor(fabgl::Color::Blue);
      break;
    case 4:
      Terminal.setBackgroundColor(fabgl::Color::Magenta);
      break;
    case 5:
      Terminal.setBackgroundColor(fabgl::Color::Cyan);
      break;
    case 6:
      Terminal.setBackgroundColor(fabgl::Color::White);
      break;
    case 7:
      Terminal.setBackgroundColor(fabgl::Color::BrightBlack);
      break;
    case 8:
      Terminal.setBackgroundColor(fabgl::Color::BrightRed);
      break;
    case 9:
      Terminal.setBackgroundColor(fabgl::Color::BrightGreen);
      break;
    case 10:
      Terminal.setBackgroundColor(fabgl::Color::BrightYellow);
      break;
    case 11:
      Terminal.setBackgroundColor(fabgl::Color::BrightBlue);
      break;
    case 12:
      Terminal.setBackgroundColor(fabgl::Color::BrightMagenta);
      break;
    case 13:
      Terminal.setBackgroundColor(fabgl::Color::BrightCyan);
      break;
    case 14:
      Terminal.setBackgroundColor(fabgl::Color::BrightWhite);
      break;
    case 15:
      Terminal.setBackgroundColor(fabgl::Color::Black);
      break;
    default:
      Terminal.setBackgroundColor(fabgl::Color::Black);
      break;
    }

    goto run_next_statement;
  }

point: {
  short int color;
  short int x;
  short int y;

    //Get color
    expression_error = 0;
    color = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get X
    expression_error = 0;
    x = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y
    expression_error = 0;
    y = expression();
    if(expression_error)
      goto qwhat;

    cv.setPenColor((fabgl::Color)color);

    cv.setPixel(x,y);

  goto run_next_statement;
}

line: {
  short int color;
  short int startX;
  short int startY;
  short int endX;
  short int endY;
  short int penWidth;
    //Get color
    expression_error = 0;
    color = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get startX
    expression_error = 0;
    startX = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get startY
    expression_error = 0;
    startY = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get endX
    expression_error = 0;
    endX = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();
    
    //Get endY
    expression_error = 0;
    endY = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get pen width
    expression_error = 0;
    penWidth = expression();
    if(expression_error)
      goto qwhat;

    cv.setPenColor((fabgl::Color)color);
    cv.setPenWidth(penWidth);
    cv.drawLine(startX, startY, endX, endY);


  goto run_next_statement;
}

rectangle: {
  short int color;
  short int fillColor;
  short int startX;
  short int startY;
  short int endX;
  short int endY;
  short int penWidth;

    //Get color
    expression_error = 0;
    color = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get fillColor
    expression_error = 0;
    fillColor = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get startX
    expression_error = 0;
    startX = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get startY
    expression_error = 0;
    startY = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get endX
    expression_error = 0;
    endX = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();


    //Get endY
    expression_error = 0;
    endY = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get pen width
    expression_error = 0;
    penWidth = expression();
    if(expression_error)
      goto qwhat;

    cv.setPenColor((fabgl::Color)color);
    cv.setPenWidth(penWidth);
    cv.drawRectangle(startX, startY, endX, endY);


    cv.setPenWidth(1);
    //Fills the retangle
    if(fillColor > -1) {
      
      cv.setPenColor((fabgl::Color)fillColor);
      for(short int y = startY+ 1; y < endY; y++ ){
        cv.drawLine(startX +1 , y, endX-1, y);
      }
      
    }


  goto run_next_statement;
}

elipse: {
  short int color;
  short int x;
  short int y;
  short int width;
  short int height;
  short int penWidth;

    //Get color
    expression_error = 0;
    color = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get x
    expression_error = 0;
    x = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y
    expression_error = 0;
    y = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get width
    expression_error = 0;
    width = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();


    //Get height
    expression_error = 0;
    height = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();



    //Get penWidth
    expression_error = 0;
    penWidth = expression();
    if(expression_error)
      goto qwhat;



    cv.setPenColor((fabgl::Color)color);
    cv.setPenWidth(penWidth);
    
    cv.drawEllipse(x, y, width, height);


  goto run_next_statement;
}


wifi_scan: {
  Terminal.write("Scanning WiFi networks...\r\n");
  
  // Em vez de forçar o mode(WIFI_STA) toda vez, apenas garantimos 
  // que o rádio está ligado. Se já estiver em STA, ele ignora.
  if (WiFi.getMode() == WIFI_OFF) {
      WiFi.mode(WIFI_STA);
      delay(100);
  }

  // O parâmetro 'false' no scanNetworks torna o scan síncrono (bloqueante),
  // o que é mais seguro para o interpretador Basic.
  int n = WiFi.scanNetworks(false, false, false, 300); 
  
  if (n < 0) {
    Terminal.write("Scan failed. Try again.\r\n");
  } else if (n == 0) {
    Terminal.write("No networks found.\r\n");
  } else {
    Terminal.printf("%d networks found:\r\n", n);
    for (int i = 0; i < n; ++i) {
      Terminal.printf("%d: %s (%d dBm) [CH: %d]\r\n", 
                      i + 1, 
                      WiFi.SSID(i).c_str(), 
                      WiFi.RSSI(i), 
                      WiFi.channel(i));
      delay(5); // Pequeno fôlego para o Terminal VGA
    }
  }
  
  // Limpa os resultados da memória para evitar o erro de deinit posterior
  WiFi.scanDelete(); 
  
  goto run_next_statement;
}



wifi_connect: {
  char ssid[32];
  char pass[64];
  int i = 0;

  // 1. Capturar o SSID (entre aspas)
  ignore_blanks();
  if (*txtpos != '"' && *txtpos != '\'') goto qwhat;
  unsigned char delim = *txtpos++;
  while (*txtpos != delim && i < 31) ssid[i++] = *txtpos++;
  ssid[i] = '\0';
  if (*txtpos++ != delim) goto qwhat;

  // 2. Pular a vírgula
  ignore_blanks();
  if (*txtpos != ',') goto qwhat;
  txtpos++;
  ignore_blanks();

  // 3. Capturar a Senha (entre aspas)
  if (*txtpos != '"' && *txtpos != '\'') goto qwhat;
  delim = *txtpos++;
  i = 0;
  while (*txtpos != delim && i < 63) pass[i++] = *txtpos++;
  pass[i] = '\0';
  if (*txtpos++ != delim) goto qwhat;

  // 4. Iniciar Conexão
  Terminal.printf("\r\nConnecting to %s...", ssid);
  WiFi.begin(ssid, pass);

  // Tentativa de conexão (timeout de 10 segundos)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Terminal.write(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Terminal.printf("\r\nConnected! IP: %s\r\n", WiFi.localIP().toString().c_str());
  } else {
    Terminal.write("\r\nFailed to connect.\r\n");
  }

  goto run_next_statement;
}

wifi_status: {
  if (WiFi.status() == WL_CONNECTED) {
    Terminal.write("\r\nStatus: CONNECTED");
    Terminal.printf("\r\nSSID  : %s", WiFi.SSID().c_str());
    Terminal.printf("\r\nIP    : %s", WiFi.localIP().toString().c_str());
    Terminal.printf("\r\nRSSI  : %d dBm\r\n", WiFi.RSSI());
  } else {
    Terminal.write("\r\nStatus: DISCONNECTED\r\n");
    // Mostra o motivo técnico se não estiver conectado
    Terminal.printf("Reason Code: %d\r\n", WiFi.status());
  }
  goto run_next_statement;
}


wifi_disconnect: {
  Terminal.write("\r\nDisconnecting...");
  
  // WiFi.disconnect(apagar_credenciais, desligar_radio)
  WiFi.disconnect(true, true); 
  
  // Garante que o rádio está em modo OFF para economizar energia
  WiFi.mode(WIFI_OFF);
  
  Terminal.write(" WiFi OFF.\r\n");
  goto run_next_statement;
}



httpget: {
  char url[128];
  int i = 0;

  // 1. Capturar a URL (entre aspas)
  ignore_blanks();
  if (*txtpos != '"' && *txtpos != '\'') goto qwhat;
  unsigned char delim = *txtpos++;
  while (*txtpos != delim && i < 127) url[i++] = *txtpos++;
  url[i] = '\0';
  if (*txtpos++ != delim) goto qwhat;

  // 2. Verificar conexão antes de tentar
  if (WiFi.status() != WL_CONNECTED) {
    Terminal.write("\r\nError: Not connected to WiFi.\r\n");
    goto run_next_statement;
  }

  // 3. Executar o GET
  HTTPClient http;
  Terminal.printf("\r\nRequesting: %s\r\n", url);
  
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) { // Sucesso
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Terminal.printf("Response (Code %d):\r\n", httpCode);
      Terminal.write("----------------------------\r\n");
      Terminal.write(payload.c_str());
      Terminal.write("\r\n----------------------------\r\n");
    } else {
      Terminal.printf("HTTP Warning: %d\r\n", httpCode);
    }
  } else {
    Terminal.printf("HTTP Error: %s\r\n", http.errorToString(httpCode).c_str());
  }

  http.end(); // Fecha a conexão
  goto run_next_statement;
}



cursor: {
  short int enable;
    //Get enable
    expression_error = 0;
    enable = expression();
    if(expression_error)
      goto qwhat;
      
    Terminal.enableCursor(enable);

  goto run_next_statement;
}

at: {
    short int x;
    short int y;
      //Get X
    expression_error = 0;
    x = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();


    //Get height
    expression_error = 0;
    y = expression();
    if(expression_error)
      goto qwhat;

    tc.setCursorPos(x,y);
    goto run_next_statement;
}


#ifdef ENABLE_TONES
tonestop:
  noTone( kPiezoPin );
  goto run_next_statement;

tonegen:
  {
    // TONE freq, duration
    // if either are 0, tones turned off
    short int freq;
    short int duration;

    //Get the frequency
    expression_error = 0;
    freq = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();


    //Get the duration
    expression_error = 0;
    duration = expression();
    if(expression_error)
      goto qwhat;

    if( freq == 0 || duration == 0 )
      goto tonestop;

    tone( kPiezoPin, freq, duration );
    if( alsoWait ) {
      delay( duration );
      alsoWait = false;
    }
    goto run_next_statement;
  }
#endif /* ENABLE_TONES */
}