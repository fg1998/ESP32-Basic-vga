#include "sd_helpers.h"
#include "globals.h"
#include "basic_tables.h"
#include "interpreter_core.h"
#include <string.h>

/***********************************************************/
/* SD Card helpers */

char currentDir[MAX_PATH_LEN] = "/";

void resolvePath(const char *name, char *out)
{
  if( name[0] == '/' )
  {
    strncpy(out, name, MAX_PATH_LEN - 1);
    out[MAX_PATH_LEN - 1] = 0;
    return;
  }

  strncpy(out, currentDir, MAX_PATH_LEN - 1);
  out[MAX_PATH_LEN - 1] = 0;
  int len = strlen(out);
  if( len > 0 && out[len - 1] != '/' && len < MAX_PATH_LEN - 1 )
  {
    out[len] = '/';
    out[len + 1] = 0;
    len++;
  }
  strncat(out, name, MAX_PATH_LEN - 1 - len);
}

bool changeDir(const char *name)
{
  char candidate[MAX_PATH_LEN];

  if( strcmp(name, "/") == 0 )
  {
    strcpy(currentDir, "/");
    return true;
  }

  if( strcmp(name, "..") == 0 )
  {
    strncpy(candidate, currentDir, MAX_PATH_LEN - 1);
    candidate[MAX_PATH_LEN - 1] = 0;
    int len = strlen(candidate);
    if( len > 1 && candidate[len - 1] == '/' )
    {
      candidate[len - 1] = 0;
      len--;
    }
    char *lastSlash = strrchr(candidate, '/');
    if( lastSlash == nullptr || lastSlash == candidate )
      strcpy(candidate, "/");
    else
      *lastSlash = 0;
  }
  else
  {
    resolvePath(name, candidate);
  }

  File dir = SD.open(candidate);
  if( !dir || !dir.isDirectory() )
  {
    if(dir) dir.close();
    return false;
  }
  dir.close();

  strncpy(currentDir, candidate, MAX_PATH_LEN - 1);
  currentDir[MAX_PATH_LEN - 1] = 0;
  return true;
}

#ifdef ENABLE_FILEIO
static boolean sd_is_initialized = false;
#endif

#if ARDUINO && ENABLE_FILEIO

int initSD( void )
{
  // if the card is already initialized, we just go with it.
  // there is no support (yet?) for hot-swap of SD Cards. if you need to 
  // swap, pop the card, reset the arduino.)

  if( sd_is_initialized == true ) return kSD_OK;

  //spiSD.begin(14, 16, 17, kSD_CS); ////SCK,MISO,MOSI,SS //HSPI1
  spiSD.begin(14, 2, 12, kSD_CS);  ////SCK,MISO,MOSI,SS //HSPI1
  
  if( !SD.begin( kSD_CS, spiSD )) {
    // failed
    printmsg( sderrormsg );
    return kSD_Fail;
  }
  // success - quietly return 0
  sd_is_initialized = true;

  // and our file redirection flags
  outStream = kStreamSerial;
  inStream = kStreamSerial;
  inhibitOutput = false;

  return kSD_OK;
}
#endif

#if ENABLE_FILEIO
void cmd_Files( void )
{
  Terminal.printf("\r\nDIR: %s\r\n", currentDir);

  File dir = SD.open( currentDir );
  dir.seek(0);

  while( true ) {
    File entry = dir.openNextFile();
    if( !entry ) {
      entry.close();
      break;
    }

    // common header
    printmsgNoNL( indentmsg );
    printmsgNoNL( (const unsigned char *)entry.name() );
    if( entry.isDirectory() ) {
      printmsgNoNL( slashmsg );
    }

    if( entry.isDirectory() ) {
      // directory ending
      for( int i=strlen( entry.name()) ; i<16 ; i++ ) {
        printmsgNoNL( spacemsg );
      }
      printmsgNoNL( dirextmsg );
    }
    else {
      // file ending
      for( int i=strlen( entry.name()) ; i<17 ; i++ ) {
        printmsgNoNL( spacemsg );
      }
      printUnum( entry.size() );
    }
    line_terminator();
    entry.close();
  }
  dir.close();
}
#endif
