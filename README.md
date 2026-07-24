# ESP32-Basic-vga

Basic Language for ESP32 with VGA output.

This is a modified version of ESP32 Computer Basic with VGA found on [instructables](https://www.instructables.com/ESP32-Basic-PC-With-VGA-Output/) with some improvements:

- Works with latest version of [FabGL](http://www.fabglib.org/classfabgl_1_1_terminal.html)
- Compatible with [TTGO VGA32 Version 1.4](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1083&FId=t3:50033:3)
- SD Card support (CS=13 CLK=14 MISO=2 MOSI=12). To save or load, add `/` as the first character of the file name (e.g. `LOAD "/foo.bas"`)
- Screen resolution 640x480 with 80x25 text
- Text and graphics can be mixed together
- WiFi commands for network access
- String variables and string functions
- Named variables (not just A-Z) via symbol tables
- Program buffer allocated in PSRAM

Includes `platformio.ini` for use with the PlatformIO Visual Studio Code plugin.

See **DEMO.BAS** for a feature walkthrough.

---

## Command Reference

### Program Control & I/O

| Command | Syntax | Description |
|---|---|---|
| `LIST` | `LIST [start[-end]]` | Prints the current program to the terminal. Optional line range shows only that section. |
| `LOAD` | `LOAD "/name.bas"` | Loads a BASIC program from SD card. Leading `/` targets the SD, otherwise internal source. |
| `NEW` | `NEW` | Erases the current program from memory and clears all variables. |
| `RUN` | `RUN` | Starts execution of the currently loaded program from the first line. |
| `SAVE` | `SAVE "/name.bas"` | Writes the current program to SD card using the same `/` prefix convention as LOAD. |
| `STOP` | `STOP` | Halts program execution and returns to the READY prompt without clearing variables. |
| `END` | `END` | Terminates the running program cleanly at the current line. |
| `BYE` | `BYE` | Exits the interpreter, returning control to the loop caller (triggers reboot/bootloader flow). |
| `FILES` | `FILES` | Lists files and directories in the current SD path, marking directories with `(dir)`. |
| `MEM` | `MEM` | Prints the amount of free program memory in bytes. |
| `CHAIN` | `CHAIN "/name.bas"` | Loads and immediately runs a program from SD, preserving current variables. |
| `CD` | `CD "path"` | Changes the current SD directory used by LOAD, SAVE, FILES, and CHAIN. |

### Flow Control & Variables

| Command | Syntax | Description |
|---|---|---|
| `LET` | `[LET] var = expr` | Assigns a value to a numeric or string variable. The LET keyword itself is optional. |
| `IF` | `IF condition THEN statement` | Executes the trailing statement only when the condition evaluates to non-zero (true). |
| `GOTO` | `GOTO linenum` | Jumps unconditionally to the given line number. |
| `GOSUB` | `GOSUB linenum` | Calls a subroutine at the given line number, pushing the return address on the stack. |
| `RETURN` | `RETURN` | Returns from the most recent GOSUB to the statement following it. |
| `FOR / NEXT` | `FOR var = start TO end [STEP n]` … `NEXT var` | Standard counted loop; STEP can be negative for descending iteration. |
| `REM` | `REM comment text` | Line comment; ignored by the interpreter until end of line. |
| `'` (apostrophe) | `' comment` | Alternate shorthand for REM; the rest of the line is treated as a comment. |
| `DELAY` | `DELAY ms` | Blocks program execution for the given number of milliseconds. |
| `RSEED` | `RSEED n` | Seeds the random generator; same seed reproduces the same RND sequence. |
| `:` (separator) | `stmt1 : stmt2` | Separates multiple statements on the same line, executed left-to-right. |

### Screen & Text

| Command | Syntax | Description |
|---|---|---|
| `CLS` | `CLS` | Clears the screen. Resets cursor to top-left, keeps current colors. |
| `COLOR` | `COLOR fg, bg` | Sets foreground and background color indices (0–15 palette) for subsequent PRINT calls. |
| `AT` | `AT x, y` | Moves the text cursor to column x, row y (0-indexed, 80×25 grid at 640×480). |
| `CURSOR` | `CURSOR 0` \| `CURSOR 1` | Hides (0) or shows (1) the blinking text cursor. |
| `PRINT` | `PRINT expr` or `PRINT "str"` | Outputs value or string at the current cursor position using current COLOR. |
| `?` | `? expr` | Shorthand for PRINT; identical behavior. |
| `INPUT` | `INPUT var` | Prompts the user and reads a value from the keyboard into the given variable. |
| `INKEY` | `INKEY var` | Reads a single keypress without waiting for Enter; stores its ASCII code in var (0 if no key). |

### Graphics

| Command | Syntax | Description |
|---|---|---|
| `POINT` | `POINT color, x, y` | Draws a single pixel at (x,y) with the given palette index color. |
| `LINE` | `LINE color, x1, y1, x2, y2, penWidth` | Draws a line between two points; penWidth in pixels controls thickness. |
| `RECTANGLE` | `RECTANGLE color, fillColor, x1, y1, x2, y2, penWidth` | Draws a rectangle; use fillColor = -1 for outline only, otherwise filled with fillColor. |
| `ELIPSE` | `ELIPSE color, x, y, width, height, penWidth` | Draws an ellipse centered at (x,y) with given axes (note the ELIPSE spelling). |

### GPIO & Low-Level

| Command | Syntax | Description |
|---|---|---|
| `POKE` | `POKE addr, value` | Writes a byte value to the given memory address inside the program buffer. |
| `AWRITE` | `AWRITE pin, value` | Writes an analog (PWM) value to the given ESP32 GPIO pin. |
| `DWRITE` | `DWRITE pin, HIGH\|LOW` | Writes a digital HIGH or LOW to the given ESP32 GPIO pin. |

### Network (WiFi)

| Command | Syntax | Description |
|---|---|---|
| `WIFISCAN` | `WIFISCAN` | Scans and prints nearby SSIDs, RSSI, and encryption info to the screen. |
| `WIFICONNECT` | `WIFICONNECT "ssid", "password"` | Attempts to join the given access point using WPA2/PSK. |
| `WIFISTATUS` | `WIFISTATUS` | Prints current connection state, IP address, and signal info. |
| `WIFIDISCONNECT` | `WIFIDISCONNECT` | Drops the current WiFi association and clears IP configuration. |
| `HTTPGET` | `HTTPGET "url"` | Issues an HTTP GET and prints the response body to the screen. |

---

## Built-in Functions

### Numeric & I/O Functions

| Function | Syntax | Description |
|---|---|---|
| `PEEK` | `PEEK(addr)` | Returns the byte value stored at the given memory address in the program buffer. |
| `ABS` | `ABS(n)` | Returns the absolute value of the numeric expression n. |
| `AREAD` | `AREAD(pin)` | Reads an analog value from the given ESP32 GPIO pin (0..4095 for 12-bit ADC). |
| `DREAD` | `DREAD(pin)` | Reads a digital value from the given ESP32 GPIO pin, returning 0 or 1. |
| `RND` | `RND(n)` | Returns a pseudo-random integer in the range 0..n-1. |
| `LEN` | `LEN(str$)` | Returns the length of a string expression in characters. |
| `VAL` | `VAL(str$)` | Parses a string containing a number and returns its numeric value. |
| `ASC` | `ASC(str$)` | Returns the ASCII code of the first character of the given string. |

### String Functions

| Function | Syntax | Description |
|---|---|---|
| `LEFT$` | `LEFT$(str$, n)` | Returns the leftmost n characters of a string; clamps to the string's length. |
| `RIGHT$` | `RIGHT$(str$, n)` | Returns the rightmost n characters of a string; clamps to the string's length. |
| `MID$` | `MID$(str$, start [, len])` | Returns a substring starting at position start (1-based); omit len to read to end. |
| `STR$` | `STR$(n)` | Converts a numeric expression to its decimal string representation. |
| `CHR$` | `CHR$(n)` | Returns a 1-character string whose ASCII code is n. |

---

## Relational Operators (for use in `IF`)

`=`, `<>`, `!=`, `>`, `>=`, `<`, `<=`

Works with both numeric and string comparisons (strings compared lexicographically).

---

## Notes

- When using TTGO VGA32 v1.4, remove the SD card before uploading the sketch
- Serial monitor can be used as keyboard
- String variables end with `$` (e.g. `NOME$ = "FERNANDO"`)
- Variable names accept letters and digits, uppercase only

**DON'T FORGET!**
`LOAD "/DEMO.BAS"`
