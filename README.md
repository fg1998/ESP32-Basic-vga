# ESP32-Basic-vga

This is a modified version of ESP32 Computer Basic with vga found on instructables https://www.instructables.com/ESP32-Basic-PC-With-VGA-Output/ with some improvements

1 - Works with latest version of FABgl http://www.fabglib.org/classfabgl_1_1_terminal.html
2 - Compatible with TTGO VGA32 Version 1.4 http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1083&FId=t3:50033:3
3 - Add SDCard support (CS=13 CLK=14 MISO=2 MOSI=12). To save or load add '/' as the file name first chr ex load "/foo.bas"
4 - Adjust screen resolution to 640x200 (80x25 text) for low ram use
5 - TEXT AND GRAPHICS CAN BE MIXED TOGHETER
6 - Include file platformio.ini for using with platformIO Visual Studio Code plugin.
See DEMO.BAS for new features

** New commands
  - CLS - Clear screen
  - COLOR FORECOLOR, BACKCOLOR - set screen color 
  - POINT COLOR, X, Y - draw a pixel
  - LINE, INIT X, INIT Y, END X, END Y, PEN WIDTH
  - RECTANGLE COLOR, COLOR FILL (-1 USES NO COLOR), INIT X, INIT Y, END X, END Y, PEN WIDTH
  - ELIPSE COLOR, X, Y, WIDTH, HEIGHT, PEN WIDTH
  - CURSOR 0/1 -> ENABLE/DISABLE CURSOR
  - AT X,Y  -> puts cursor on x,y
  
  
  Notice:
  - When using TTGO VGA32_V.14, dont forget to remove the SD when upload the sketch
  - Serial monitor can be used as keyboard 
  
  DONT FORGET !!!!
  LOAD "/DEMO.BAS"
  
  
