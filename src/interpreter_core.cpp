#include "interpreter_core.h"
#include "basic_strings.h"
#include "basic_vars.h"
#include <string.h>
#include <stdlib.h>

// Forward declaration interna: inchar() e usada dentro de getln() mas so
// e definida mais abaixo neste arquivo (mesma ordem do .ino original).
static int inchar(void);

void print_info()
{
  Terminal.write("\e[33mESP32 TinyBasic PC with VGA monitor and PS2keyboard\r\n");
  Terminal.write("\e[32mby Roberto Melzi\e[32m\r\n\n");
  Terminal.write("\e[32mVGA32_V0.15 by fg1998 \e[31m github.com/fg1998/ESP32-Basic-vga \e[32m\r\n\n");
  Terminal.write("\e[37mFabGL - Loopback VT/ANSI Terminal\r\n");
  Terminal.write("\e[37m2019 by Fabrizio Di Vittorio - www.fabgl.com\e[32m\r\n\n");
  Terminal.printf("\e[31mScreen Size        :\e[33m %d x %d\r\n", VGAController.getScreenWidth(), VGAController.getScreenHeight());
  Terminal.printf("\e[32mTerminal Size      :\e[33m %d x %d\r\n", Terminal.getColumns(), Terminal.getRows());
  Terminal.printf("\e[35mFree DMA Memory    :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
  Terminal.printf("\e[36mFree 32 bit Memory :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
  Terminal.printf("\e[34mBASIC Program RAM  :\e[33m %d\r\n\n", kRamSize);
  Terminal.printf("\e[37mTinyBasic Plus v0.15\r\n");
  //Terminal.write("\e[32mFree typing test - press ESC to introduce escape VT/ANSI codes\r\n\n");

}

void ignore_blanks(void)
{
  while(*txtpos == SPACE || *txtpos == TAB)
    txtpos++;
}


/***************************************************************************/
void scantable(const unsigned char *table)
{
  int i = 0;
  table_index = 0;
  while(1)
  {
    // Run out of table entries?
    if(pgm_read_byte( table ) == 0)
      return;

    // Do we match this character?
    if(txtpos[i] == pgm_read_byte( table ))
    {
      i++;
      table++;
    }
    else
    {
      // do we match the last character of keywork (with 0x80 added)? If so, return
      if(txtpos[i]+0x80 == pgm_read_byte( table ))
      {
        // Com nomes de variavel de mais de uma letra, precisa confirmar que
        // isso nao e so um PREFIXO de um identificador maior (ex: a palavra-
        // chave "FOR" nao pode "casar" com uma variavel chamada "FORCE").
        // So importa quando o ultimo caractere da palavra-chave e uma letra
        // (palavras-chave de pontuacao como '?' nao tem essa ambiguidade).
        unsigned char lastMatched = txtpos[i];
        unsigned char next = txtpos[i+1];
        boolean lastIsLetter = (lastMatched >= 'A' && lastMatched <= 'Z');
        boolean nextIsIdentChar = (next >= 'A' && next <= 'Z') || (next >= '0' && next <= '9');
        if( !(lastIsLetter && nextIsIdentChar) )
        {
          txtpos += i+1;  // Advance the pointer to following the keyword
          ignore_blanks();
          return;
        }
        // senao, e prefixo de um identificador maior - trata como mismatch
        // e cai pro mesmo tratamento abaixo (avanca pra proxima palavra)
      }

      // Forward to the end of this keyword
      while((pgm_read_byte( table ) & 0x80) == 0)
        table++;

      // Now move on to the first character of the next word, and reset the position index
      table++;
      table_index++;
      ignore_blanks();
      i = 0;
    }
  }
}

/***************************************************************************/
static void pushb(unsigned char b)
{
  sp--;
  *sp = b;
}

/***************************************************************************/
static unsigned char popb()
{
  unsigned char b;
  b = *sp;
  sp++;
  return b;
}

/***************************************************************************/
void printnum(int num)
{
  int digits = 0;

  if(num < 0)
  {
    num = -num;
    outchar('-');
  }
  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

void printUnum(unsigned int num)
{
  int digits = 0;

  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

/***************************************************************************/
unsigned short testnum(void)
{
  unsigned short num = 0;
  ignore_blanks();

  while(*txtpos>= '0' && *txtpos <= '9' )
  {
    // Trap overflows
    if(num >= 0xFFFF/10)
    {
      num = 0xFFFF;
      break;
    }

    num = num *10 + *txtpos - '0';
    txtpos++;
  }
  return  num;
}

/***************************************************************************/
unsigned char print_quoted_string(void)
{
  int i=0;
  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  // Print the characters
  while(*txtpos != delim)
  {
    outchar(*txtpos);
    txtpos++;
  }
  txtpos++; // Skip over the last delimiter

  return 1;
}


/***************************************************************************/
void printmsgNoNL(const unsigned char *msg)
{
  while( pgm_read_byte( msg ) != 0 ) {
    outchar( pgm_read_byte( msg++ ) );
  };
}

/***************************************************************************/
void printmsg(const unsigned char *msg)
{
  printmsgNoNL(msg);
  line_terminator();
}

/***************************************************************************/
void getln(char prompt)
{
 
  outchar(prompt);
  txtpos = program_end+sizeof(LINENUM);

  while(1)
  {
    char c = inchar();
    switch(c)
    {
    case NL:
      //break;
    case CR:
      line_terminator();
      // Terminate all strings with a NL
      txtpos[0] = NL;
      return;
    case CTRLH:
    case BACKSPACE:
      if(txtpos == program_end)
        break;
      txtpos--;
      //printmsg(backspacemsg);
      Terminal.write("\b\e[K");
      break;
      
    default:
      // We need to leave at least one space to allow us to shuffle the line into order
      if(txtpos == variables_begin-2)
        outchar(BELL);
      else
      {
        txtpos[0] = c;
        txtpos++;
        outchar(c);
      }
    }
  }
}

/***************************************************************************/
unsigned char *findline(void)
{
  unsigned char *line = program_start;
  while(1)
  {
    if(line == program_end)
      return line;

    if(((LINENUM *)line)[0] >= linenum)
      return line;

    // Add the line length onto the current address, to get to the next line;
    line += line[sizeof(LINENUM)];
  }
}

/***************************************************************************/
void toUppercaseBuffer(void)
{
  unsigned char *c = program_end+sizeof(LINENUM);
  unsigned char quote = 0;

  while(*c != NL)
  {
    // Are we in a quoted string?
    if(*c == quote)
      quote = 0;
    else if(*c == '"' || *c == '\'')
      quote = *c;
    else if(quote == 0 && *c >= 'a' && *c <= 'z')
      *c = *c + 'A' - 'a';
    c++;
  }
}

/***************************************************************************/
void printline()
{
  LINENUM line_num;

  line_num = *((LINENUM *)(list_line));
  list_line += sizeof(LINENUM) + sizeof(char);

  // Output the line */
  printnum(line_num);
  outchar(' ');
  while(*list_line != NL)
  {
    outchar(*list_line);
    list_line++;
  }
  list_line++;
  line_terminator();
}

/***************************************************************************/
static short int expr4(void)
{
  // fix provided by Jurg Wullschleger wullschleger@gmail.com
  // fixes whitespace and unary operations
  ignore_blanks();

  if( *txtpos == '-' ) {
    txtpos++;
    return -expr4();
  }
  // end fix

  if(*txtpos == '0')
  {
    txtpos++;
    return 0;
  }

  if(*txtpos >= '1' && *txtpos <= '9')
  {
    short int a = 0;
    do  {
      a = a*10 + *txtpos - '0';
      txtpos++;
    } 
    while(*txtpos >= '0' && *txtpos <= '9');
    return a;
  }

  // Is it a function or variable reference?
  if(txtpos[0] >= 'A' && txtpos[0] <= 'Z')
  {
    short int a;
    char name[MAX_VAR_NAME_LEN + 1];
    boolean isStr;
    scan_identifier(name, &isStr);

    if( isStr )
    {
      // uma variavel de STRING (nome$) usada aqui, em contexto numerico -
      // so e valido dentro de LEN()/VAL()/ASC(), que nao passam por aqui
      // (eles chamam str_expression() direto, nao expr4()). Fora disso, e
      // erro de sintaxe mesmo (nao da pra somar string com numero).
      expression_error = 1;
      return 0;
    }

    ignore_blanks(); // permite espaco entre o nome e "(" tipo "ABS (x)"

    if( *txtpos == '(' )
    {
      int fi = matchFuncName(name);
      if( fi < 0 )
        goto expr4_error; // nome seguido de "(" que nao e funcao conhecida

      unsigned char f = (unsigned char)fi;
      txtpos++;

      // LEN/VAL/ASC recebem uma expressao de STRING como argumento, nao numerica
      if( f == FUNC_LEN || f == FUNC_VAL || f == FUNC_ASC )
      {
        char *s = str_expression();
        if(*txtpos != ')')
          goto expr4_error;
        txtpos++;
        short int r = 0;
        switch(f)
        {
        case FUNC_LEN:
          r = (short int)strlen(s);
          break;
        case FUNC_VAL:
          r = (short int)atoi(s);
          break;
        case FUNC_ASC:
          r = s[0] ? (short int)(unsigned char)s[0] : 0;
          break;
        }
        str_scratch_release();
        return r;
      }

      a = expression();
      if(*txtpos != ')')
        goto expr4_error;
      txtpos++;
      switch(f)
      {
      case FUNC_PEEK:
        return program[a];
        
      case FUNC_ABS:
        if(a < 0) 
          return -a;
        return a;


      case FUNC_AREAD:
        pinMode( a, INPUT );
        return analogRead( a );                        
      case FUNC_DREAD:
        pinMode( a, INPUT );
        return digitalRead( a );


      case FUNC_RND:
#ifdef ARDUINO
        return( random( a ));
#else
        return( rand() % a );
#endif
      }
    }

    // nome sem "(" na sequencia -> variavel numerica comum
    return *numvar_ref(name);
  }

  if(*txtpos == '(')
  {
    short int a;
    txtpos++;
    a = expression();
    if(*txtpos != ')')
      goto expr4_error;

    txtpos++;
    return a;
  }

expr4_error:
  expression_error = 1;
  return 0;

}

/***************************************************************************/
static short int expr3(void)
{
  short int a,b;

  a = expr4();

  ignore_blanks(); // fix for eg:  100 a = a + 1

  while(1)
  {
    if(*txtpos == '*')
    {
      txtpos++;
      b = expr4();
      a *= b;
    }
    else if(*txtpos == '/')
    {
      txtpos++;
      b = expr4();
      if(b != 0)
        a /= b;
      else
        expression_error = 1;
    }
    else
      return a;
  }
}

/***************************************************************************/
static short int expr2(void)
{
  short int a,b;

  if(*txtpos == '-' || *txtpos == '+')
    a = 0;
  else
    a = expr3();

  while(1)
  {
    if(*txtpos == '-')
    {
      txtpos++;
      b = expr3();
      a -= b;
    }
    else if(*txtpos == '+')
    {
      txtpos++;
      b = expr3();
      a += b;
    }
    else
      return a;
  }
}
/***************************************************************************/
short int expression(void)
{
  short int a,b;

  a = expr2();

  // Check if we have an error
  if(expression_error)  return a;

  scantable(relop_tab);
  if(table_index == RELOP_UNKNOWN)
    return a;

  switch(table_index)
  {
  case RELOP_GE:
    b = expr2();
    if(a >= b) return 1;
    break;
  case RELOP_NE:
  case RELOP_NE_BANG:
    b = expr2();
    if(a != b) return 1;
    break;
  case RELOP_GT:
    b = expr2();
    if(a > b) return 1;
    break;
  case RELOP_EQ:
    b = expr2();
    if(a == b) return 1;
    break;
  case RELOP_LE:
    b = expr2();
    if(a <= b) return 1;
    break;
  case RELOP_LT:
    b = expr2();
    if(a < b) return 1;
    break;
  }
  return 0;
}

// returns 1 if the character is valid in a filename
static int isValidFnChar( char c )
{
  if( c >= '0' && c <= '9' ) return 1; // number
  if( c >= 'A' && c <= 'Z' ) return 1; // LETTER
  if( c >= 'a' && c <= 'z' ) return 1; // letter (for completeness)
  if( c == '/' ) return 1;
  if( c == '_' ) return 1;
  if( c == '+' ) return 1;
  if( c == '.' ) return 1;
  if( c == '~' ) return 1;  // Window~1.txt

  return 0;
}

unsigned char * filenameWord(void)
{
  // SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
  unsigned char * ret = txtpos;
  expression_error = 0;

  // make sure there are no quotes or spaces, search for valid characters
  //while(*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE ) txtpos++;
  while( !isValidFnChar( *txtpos )) txtpos++;
  ret = txtpos;

  if( *ret == '\0' ) {
    expression_error = 1;
    return ret;
  }

  // now, find the next nonfnchar
  txtpos++;
  while( isValidFnChar( *txtpos )) txtpos++;
  if( txtpos != ret ) *txtpos = '\0';

  // set the error code if we've got no string
  if( *ret == '\0' ) {
    expression_error = 1;
  }

  return ret;
}


/***************************************************************************/
void line_terminator(void)
{
  outchar(NL);
  outchar(CR);
}


/***********************************************************/
unsigned char breakcheck(void)
{
#ifdef ARDUINO
  if(Serial.available())
    return Serial.read() == CTRLC;
  return 0;
#else
#ifdef __CONIO__
  if(kbhit())
    return getch() == CTRLC;
  else
#endif
    return 0;
#endif
}

/***********************************************************/
static int inchar()
{
  int v;
#ifdef ARDUINO
  
  switch( inStream ) {
  case( kStreamFile ):
#ifdef ENABLE_FILEIO
    v = fp.read();
    if( v == NL ) v=CR; // file translate
    if( !fp.available() ) {
      fp.close();
      goto inchar_loadfinish;
    }
    return v;    
#else
#endif
     break;
  case( kStreamEEProm ):
#ifdef ENABLE_EEPROM
#ifdef ARDUINO
    v = EEPROM.read( eepos++ );
    if( v == '\0' ) {
      goto inchar_loadfinish;
    }
    return v;
#endif
#else
    inStream = kStreamSerial;
    return NL;
#endif
     break;
  case( kStreamSerial ):
  default:
    while(1)
    {
     
     //----------- the following is the key modification -------------------------------------------------------------------------------------------------
     //----------- where the code get the variables from the PS2 keyboard --------------------------------------------------------------------------------
     //----------- and treat them as the ones from the PC keyboard ---------------------------------------------------------------------------------------
 
     /*if (keyboard.available()) {
       // read the next key
       char c = keyboard.read();
       //Serial.print(c);
       return c; 
     }*/
     if (Terminal.available()) {
       // read the next key
       char c = Terminal.read();
       //myWrite(c); 
       //Serial.print(c);
       return c; 
     }
     //------------ end of modification -------------------------------------------------------------------------------------------------------------------
     
     /*if(Serial.available())
     return Serial.read(); */
      if(Serial.available()){
         char c = Serial.read(); 
         //vga.print(c); //-------------------------------------------------- così scrive solo i caratteri della tastiera ----------------------------------------------------------------
         return c; 
      }
    }
  }
  
inchar_loadfinish:
  inStream = kStreamSerial;
  inhibitOutput = false;

  if( runAfterLoad ) {
    runAfterLoad = false;
    triggerRun = true;
  }
  return NL; // trigger a prompt.
  
#else
  // otherwise. desktop!
  int got = getchar();

  // translation for desktop systems
  if( got == LF ) got = CR;

  return got;
#endif
}


/***********************************************************/
void outchar(unsigned char c)
{
  if( inhibitOutput ) return;

#ifdef ARDUINO
  #ifdef ENABLE_FILEIO
    if( outStream == kStreamFile ) {
      // output to a file

      fp.write( c );
    } 
    else
  #endif
  #ifdef ARDUINO
  #ifdef ENABLE_EEPROM
    if( outStream == kStreamEEProm ) {
      EEPROM.write( eepos++, c );
    }
    else 
  #endif /* ENABLE_EEPROM */
  #endif /* ARDUINO */
    Serial.write(c); 
    Terminal.write(c); //------------------------ here to write to the VGA monitor -----------------------------------
    myWrite(c); 
#else
  putchar(c);
#endif
}


void myWrite(char c) {
  if (Terminal.available()) {
    c = Terminal.read();
    switch (c) {
     case 0x7F:       // DEL -> backspace + ESC[K
       Terminal.write("\b\e[K");
       break;
     case 0x0D:       // CR  -> CR + LF
       Terminal.write("\r\n");
       break;
     case 0x03:       // ctrl+c 
       printmsg(breakmsg);
       current_line = 0;
       sp = program+kRamSize;
       printmsg(okmsg);
       break;
     case 0x02:       // ctrl+b 
       Terminal.setForegroundColor(Color::Red);
       //Terminal.printf("\e[35m"); 
       break;
     case 0x1B:       // ESC 
       //Terminal.write("\r\n");
       switch (myScreen){
          case 0: 
             Terminal.setBackgroundColor(Color::Black);
             Terminal.setForegroundColor(Color::Yellow);
             myScreen = 1; 
          break; 
          case 1: 
             Terminal.setBackgroundColor(Color::Black);
             Terminal.setForegroundColor(Color::BrightWhite);
             myScreen = 2; 
          break; 
          case 2: 
             Terminal.setBackgroundColor(Color::Blue);
             Terminal.setForegroundColor(Color::White);
             myScreen = 3; 
          break; 
          case 3: 
             Terminal.setBackgroundColor(Color::Black);
             Terminal.setForegroundColor(Color::BrightGreen);
             myScreen = 0; 
          break; 
       }
       break;
    default:
       Terminal.write(c);
       break;
    }
  }
}
