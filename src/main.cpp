// TinyBasic Plus para ESP32 + FabGL (VGA32) - fg1998
// main.cpp: ponto de entrada do PlatformIO. So contem setup() e o loop()
// do Arduino; toda a logica do interpretador esta nos outros arquivos:
//   config.h            - flags de hardware/feature e macros
//   basic_types.h       - tipos e structs do interpretador
//   globals.h/.cpp      - estado global e objetos de hardware (VGA/PS2/SD)
//   basic_tables.h/.cpp - tabelas de keywords/funcoes e mensagens
//   interpreter_core.h/.cpp - funcoes do interpretador (parsing, I/O)
//   basic_loop.h/.cpp   - os comandos do BASIC (basic_run)
//   sd_helpers.h/.cpp   - inicializacao do cartao SD e listagem de arquivos

#include "config.h"
#include "globals.h"
#include "interpreter_core.h"
#include "sd_helpers.h"
#include "basic_loop.h"

/***********************************************************/
void setup()
{

   // Ensures the bootloader runs on the next power-up
    const esp_partition_t* otadata = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, NULL);
    if (otadata) {
        esp_partition_erase_range(otadata, 0, otadata->size);
    }
#ifdef ARDUINO
  Serial.begin(kConsoleBaud); // opens serial port
  while( !Serial ); // for Leonardo

  // Aloca o buffer de programas BASIC em PSRAM (ver globals.cpp/.h) - antes
  // era um array estatico na SRAM interna, o que limitava kRamSize a pouco
  // mais de 100KB antes de estourar o dram0_0_seg no link.
  program = (unsigned char *)ps_malloc(kRamSize);
  if( program == nullptr ) {
    Serial.println("ERRO: falha ao alocar buffer BASIC em PSRAM!");
    while(1) delay(1000);
  }
  
  //Serial.println("Keyboard Test:");
  delay(500);  // avoid garbage into the UART
  Serial.write("\n\nReset\n");
  
  PS2Controller.begin(PS2Preset::KeyboardPort0);

  //VGAController.begin(GPIO_NUM_22,  GPIO_NUM_19,  GPIO_NUM_5, GPIO_NUM_23, GPIO_NUM_15);
  VGAController.begin(GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18, GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_23, GPIO_NUM_15);
  VGAController.setResolution(VGA_640x480_60Hz);
  
  Canvas cv(&VGAController);

  Terminal.begin(&VGAController);
  Terminal.loadFont(&fabgl::FONT_7x13);
  Terminal.setBackgroundColor(Color::Black);
  Terminal.setForegroundColor(Color::BrightGreen);


  Terminal.connectLocally();      // to use Terminal.read(), available(), etc..
  
  Terminal.clear();
  print_info();
  //Terminal.setBackgroundColor(Color::Blue);
  Terminal.setForegroundColor(Color::White);

  Terminal.enableCursor(true);

#ifdef ENABLE_FILEIO
  initSD();
  
#ifdef ENABLE_AUTORUN
  if( SD.exists( kAutorunFilename )) {
    program_end = program_start;
    fp = SD.open( kAutorunFilename );
    inStream = kStreamFile;
    inhibitOutput = true;
    runAfterLoad = true;
  }
#endif /* ENABLE_AUTORUN */

#endif /* ENABLE_FILEIO */

#ifdef ENABLE_EEPROM
#ifdef ENABLE_EAUTORUN
  // read the first byte of the eeprom. if it's a number, assume it's a program we can load
  int val = EEPROM.read(0);
  if( val >= '0' && val <= '9' ) {
    program_end = program_start;
    inStream = kStreamEEProm;
    eepos = 0;
    inhibitOutput = true;
    runAfterLoad = true;
  }
#endif /* ENABLE_EAUTORUN */
#endif /* ENABLE_EEPROM */

#endif /* ARDUINO */
}


void loop()
{
  // basic_run() e o antigo void loop() do .ino original: um interpretador
  // BASIC que roda para sempre (via goto interno) e so retorna quando o
  // comando BYE e executado. Se isso acontecer, o Arduino chama loop()
  // de novo e o interpretador reinicia do zero - mesmo comportamento do
  // arquivo original.
  basic_run();
}
