#include <EEPROM.h>
#include <SPI.h>
#include <Gamebuino.h>
#include <avr/pgmspace.h>

Gamebuino gb;
bool btnUp = false;
bool btnDown = false;
bool btnLeft = false;
bool btnRight = false;
bool btnPaint = false;
byte posX = 0;
byte posY = 0;
byte cursorIcon = 0;
bool cursorStep = false;
bool bitMap[24][24];////width, then height. X then Y.
bool showInfo = true;
bool menuLoop = false;
bool paintMode = true;////true = paint, false = erase.
bool brushBigs = false;////2 bits representing the brush size.//MSB
bool brushSmalls = false;//////B00 = small, B01 = small2, B10 = med, B11 = large//LSB
#define eepAddr 0

///bitmaps
const byte pencilButton[] PROGMEM = {16, 10,
                                     B00111111, B11000000,
                                     B01000000, B00000000,
                                     B10000100, B01000000,
                                     B10000111, B11000000,
                                     B10000100, B01000000,
                                     B10000010, B10000000,
                                     B10100001, B00000000,
                                     B10011110, B00000000,
                                     B01000000, B00000000,
                                     B00111111, B11000000,
                                    };

const byte eraseButton[] PROGMEM = {16, 10,
                                    B00111111, B11000000,
                                    B01000000, B00000000,
                                    B10000011, B11000000,
                                    B10000100, B01000000,
                                    B10001000, B10000000,
                                    B10010001, B00000000,
                                    B10111110, B00000000,
                                    B10000000, B00000000,
                                    B01000000, B00000000,
                                    B00111111, B11000000,
                                   };

const byte radioButton[] PROGMEM = {16, 10,
                                    B11111111, B00000000,
                                    B00000000, B10000000,
                                    B00111100, B01000000,
                                    B01100110, B01000000,
                                    B01000010, B01000000,
                                    B01000010, B01000000,
                                    B01100110, B01000000,
                                    B00111100, B01000000,
                                    B00000000, B10000000,
                                    B11111111, B00000000,
                                   };

const byte emptyButton[] PROGMEM = {16, 10,
                                    B00111111, B11000000,
                                    B01000000, B00000000,
                                    B10000000, B00000000,
                                    B10000000, B00000000,
                                    B10000000, B00000000,
                                    B10000000, B00000000,
                                    B10000000, B00000000,
                                    B10000000, B00000000,
                                    B01000000, B00000000,
                                    B00111111, B11000000,
                                   };

const byte invaderButton[] PROGMEM = {16, 10,
                                      B00111111, B11000000,
                                      B01010000, B10000000,
                                      B10001001, B00000000,
                                      B10011111, B10000000,
                                      B10110110, B11000000,
                                      B11011111, B10100000,
                                      B11011111, B10100000,
                                      B10010000, B10000000,
                                      B01001001, B00000000,
                                      B00111111, B11000000,
                                     };

///menu stufffff
#define MENULENGTH 5
const char strLoad[] PROGMEM = "Load from eep";
const char strSave[] PROGMEM = "Save to eep";
const char strPaintOps[] PROGMEM = "Canvas operations";
const char strCursorOps[] PROGMEM = "Cursor operations";
const char strQuit[] PROGMEM = "Quit GambiPaint";
const char* const menu[MENULENGTH] PROGMEM = {
  strLoad,
  strSave,
  strPaintOps,
  strCursorOps,
  strQuit
};

//clearMenu stuff
#define PAINTMENULENGTH 3
const char strClear[] PROGMEM = "Clear painting";
const char strFill[] PROGMEM = "Fill painting";
const char strInvert[] PROGMEM = "Invert painting";
const char* const paintMenu[PAINTMENULENGTH] PROGMEM = {
  strClear,
  strFill,
  strInvert
};

//cursor change menu stuff
#define CURSORMENULENGTH 6
const char strStep[] PROGMEM = "Toggle single step";
const char strCrossh[] PROGMEM = "Cross-hair cursor";
const char strPoint[] PROGMEM = "Point Cursor";
const char strChev[] PROGMEM = "Chevron Cursor";
const char strMouse[] PROGMEM = "Mouse Pointer";
const char strMouseUpside[] PROGMEM = "Upside Mouse Pointer";
const char* const cursorChoser[CURSORMENULENGTH] PROGMEM = {
  strStep,
  strCrossh,
  strPoint,
  strChev,
  strMouse,
  strMouseUpside
};

// the setup routine runs once when Gamebuino starts up
void setup() {
  ///initializing bitmap to zero
  clearBitMap(false);
  // initialize the Gamebuino object
  gb.begin();
  //display the main menu:
  gb.titleScreen(F("for painting, ykno..."));
  gb.popup(F("Tym 2 Paent!"), 40);

  mem_check();
}

// the loop routine runs over and over again forever
void loop() {
  //updates the gamebuino (the display, the sound, the auto backlight... everything)
  //returns true when it's time to render a new frame (20 times/second)
  if (gb.update()) {
    //just so I can see the corners in simbuino...
    gb.display.drawPixel(0, 0);
    gb.display.drawPixel(0, 47);
    gb.display.drawPixel(83, 0);
    gb.display.drawPixel(83, 47);

    if (gb.buttons.pressed(BTN_UP)) {
      if (cursorStep) posY -= 1;
      btnUp = true;
    } else if (gb.buttons.released(BTN_UP)) {
      btnUp = false;
    }
    if (gb.buttons.pressed(BTN_DOWN)) {
      if (cursorStep) posY += 1;
      btnDown = true;
    } else if (gb.buttons.released(BTN_DOWN)) {
      btnDown = false;
    }
    if (gb.buttons.pressed(BTN_LEFT)) {
      if (cursorStep) posX -= 1;
      btnLeft = true;
    } else if (gb.buttons.released(BTN_LEFT)) {
      btnLeft = false;
    }
    if (gb.buttons.pressed(BTN_RIGHT)) {
      if (cursorStep) posX += 1;
      btnRight = true;
    } else if (gb.buttons.released(BTN_RIGHT)) {
      btnRight = false;
    }
    if (gb.buttons.pressed(BTN_A)) {
      btnPaint = true;
    } else if (gb.buttons.released(BTN_A)) {
      btnPaint = false;
    }

    if (gb.buttons.pressed(BTN_B)) showInfo = !showInfo;

    ///all sorts of input stuff
    if (!cursorStep) {
      if (btnUp)    posY--;
      if (btnDown)  posY++;
      if (btnLeft)  posX--;
      if (btnRight) posX++;
    }
    
    if (posX > 83)            posX = 0;
    if (posX == 0 && btnLeft) posX = 83;
    if (posY > 47)            posY = 0;
    if (posY == 0 && btnUp)   posY = 47;

    ///brush button clicks
    if (gb.buttons.pressed(BTN_A)) {
      if (posX >= 68 && posX <= 73) {
        if (posY >= 4 && posY <= 9) {/////if clicked on paint
          paintMode = true;
        } else if (posY >= 15 && posY <= 20) {////if clicked on erase
          paintMode = false;
        }
      }
    }

    ///size button clicks
    if (gb.buttons.pressed(BTN_A)) {
      if (posX >= 12 && posX <= 17) {
        if (posY >= 29 && posY <= 34) {
          brushBigs   = false;
          brushSmalls = false;
        } else if (posY >= 40 && posY <= 45) {
          brushBigs   = false;
          brushSmalls = true;
        }
      } else if (posX >= 33 && posX <= 38) {
        if (posY >= 29 && posY <= 34) {
          brushBigs   = true;
          brushSmalls = false;
        } else if (posY >= 40 && posY <= 45) {
          brushBigs   = true;
          brushSmalls = true;
        }
      }
    }

    ///draw the canvas border
    gb.display.drawRect(29, 0, 26, 26);

    ///draw the current bitMap
    for (byte x = 0; x < 24; x++) {
      for (byte y = 0; y < 24; y++) {
        if (bitMap[x][y]) gb.display.drawPixel(x + 30, y + 1);
      }
    }

    if (posX > 28 && posX < 54) {
      if (posY < 25 && posY > 0) {
        if (btnPaint) {
          if (!(brushBigs && brushSmalls)) bitMap[posX - 30][posY - 1] = paintMode/*!bitMap[posX-30][posY-1]*/; //<- this for invert paint.
          if ((!brushBigs && brushSmalls) || (brushBigs && !brushSmalls)) {///if small2 brush
            if (posY < 24) {
              bitMap[(posX - 30)][(posY - 1) + 1] = paintMode;
            }
            if (posX < 53) {
              bitMap[(posX - 30 + 1)][(posY - 1)] = paintMode;
              if (posY < 24) {
                bitMap[(posX - 30) + 1][(posY - 1) + 1] = paintMode;
              }
            }
          }
          if (brushBigs && !brushSmalls) {
            if (posY < 23) {
              bitMap[(posX - 30)][(posY - 1) + 2] = paintMode;
            }
            if (posX < 52) {
              bitMap[(posX - 30 + 2)][(posY - 1)] = paintMode;
              if (posY < 23) {
                bitMap[(posX - 30) + 2][(posY - 1) + 2] = paintMode;
                bitMap[(posX - 30) + 1][(posY - 1) + 2] = paintMode;
                bitMap[(posX - 30) + 2][(posY - 1) + 1] = paintMode;
              }
            }
          }
          if (brushBigs && brushSmalls) {
            if (posY < 18 && posY > 0 && posX < 45 && posX > 29) {
              ///draw the invader...
              drawInvader(posX - 30, posY - 1);
            }
          }
        }
      }
    }

    ///info stuff
    if (showInfo) {
      gb.display.cursorX = 1;
      gb.display.print(F("Info\r\n"));
      gb.display.print(F("posX:"));
      gb.display.println(posX);
      gb.display.print(F("posY:"));
      gb.display.println(posY);
      gb.display.print(F("\27Menu\r\n"));
    }

    //////////////draw the cursor....
    gb.display.drawPixel(posX, posY);
    if (cursorIcon == 0) {//cross-hair
      gb.display.drawPixel(posX - 2, posY);
      gb.display.drawPixel(posX - 3, posY);

      gb.display.drawPixel(posX + 2, posY);
      gb.display.drawPixel(posX + 3, posY);

      gb.display.drawPixel(posX, posY - 2);
      gb.display.drawPixel(posX, posY - 3);

      gb.display.drawPixel(posX, posY + 2);
      gb.display.drawPixel(posX, posY + 3);
    } else if (cursorIcon == 1) {//single point
      //do nothing, single point is already drawn
    } else if (cursorIcon == 2) {//4 chevrons
      gb.display.drawPixel(posX - 2, posY - 2);//top left
      gb.display.drawPixel(posX - 3, posY - 2);
      gb.display.drawPixel(posX - 2, posY - 3);

      gb.display.drawPixel(posX + 2, posY + 2);//bottom right
      gb.display.drawPixel(posX + 3, posY + 2);
      gb.display.drawPixel(posX + 2, posY + 3);

      gb.display.drawPixel(posX - 2, posY + 2);//bottom left
      gb.display.drawPixel(posX - 3, posY + 2);
      gb.display.drawPixel(posX - 2, posY + 3);

      gb.display.drawPixel(posX + 2, posY - 2);//bottom right
      gb.display.drawPixel(posX + 3, posY - 2);
      gb.display.drawPixel(posX + 2, posY - 3);
    } else if (cursorIcon == 3) {//mouse pointer
      gb.display.drawFastVLine(posX, posY + 1, 6);
      gb.display.drawPixel(posX + 1, posY + 1);
      gb.display.drawPixel(posX + 1, posY + 5);
      gb.display.drawPixel(posX + 2, posY + 2);
      gb.display.drawPixel(posX + 2, posY + 5);
      gb.display.drawPixel(posX + 3, posY + 3);
      gb.display.drawPixel(posX + 3, posY + 4);
      gb.display.drawPixel(posX + 3, posY + 6);
      gb.display.drawPixel(posX + 4, posY + 4);
    } else if (cursorIcon = 4) {//upside-down mouse pointer
      gb.display.drawFastVLine(posX, posY - 6, 6);
      gb.display.drawPixel(posX - 1, posY - 1);
      gb.display.drawPixel(posX - 1, posY - 5);
      gb.display.drawPixel(posX - 2, posY - 2);
      gb.display.drawPixel(posX - 2, posY - 5);
      gb.display.drawPixel(posX - 3, posY - 3);
      gb.display.drawPixel(posX - 3, posY - 4);
      gb.display.drawPixel(posX - 3, posY - 6);
      gb.display.drawPixel(posX - 4, posY - 4);
    }


    ///buttons
    //pencil
    gb.display.drawBitmap(57, 2, pencilButton);
    gb.display.drawBitmap(67, 2, radioButton);
    //erase
    gb.display.drawBitmap(57, 13, eraseButton);
    gb.display.drawBitmap(67, 13, radioButton);
    //small
    gb.display.drawBitmap(1, 27, emptyButton);
    gb.display.drawBitmap(11, 27, radioButton);
    gb.display.drawPixel(4, 33);
    //small 2
    gb.display.drawBitmap(1, 38, emptyButton);
    gb.display.drawBitmap(11, 38, radioButton);
    gb.display.fillRect(4, 43, 2, 2);
    //medium
    gb.display.drawBitmap(22, 27, emptyButton);
    gb.display.drawBitmap(32, 27, radioButton);
    gb.display.fillRect(25, 31, 3, 3);
    //large
    gb.display.drawBitmap(22, 38, invaderButton);
    gb.display.drawBitmap(32, 38, radioButton);

    if (paintMode) {
      gb.display.fillRect(70, 6, 2, 2);
    } else {
      gb.display.fillRect(70, 17, 2, 2);
    }
    if (!brushBigs && !brushSmalls) {//draw small
      gb.display.fillRect(14, 31, 2, 2);
    } else if (!brushBigs && brushSmalls) {//draw small2
      gb.display.fillRect(14, 42, 2, 2);
    } else if (brushBigs && !brushSmalls) {//draw med
      gb.display.fillRect(35, 31, 2, 2);
    } else if (brushBigs && brushSmalls) {//draw large
      gb.display.fillRect(35, 42, 2, 2);
    }

  }

  if (gb.buttons.pressed(BTN_C)) {
    switch (gb.menu(menu, MENULENGTH)) {
      case  0: EEPROM.get(eepAddr, bitMap); break;
      case  1: EEPROM.put(eepAddr, bitMap); break;
      case  2: clearMenu();                 break;
      case  3: cursorMenu();                break;
      case  4: gb.changeGame();             break;
      case -1:                              break;
      default:                              break;
    }
  }

  mem_check();

}

void drawInvader(byte x, byte y) {
  //top row
  bitMap[x + 2][y] = paintMode;
  bitMap[x + 7][y] = paintMode;
  //2nd row
  bitMap[x + 3][y + 1] = paintMode;
  bitMap[x + 6][y + 1] = paintMode;
  //3rd row
  bitMap[x + 2][y + 2] = paintMode;
  bitMap[x + 3][y + 2] = paintMode;
  bitMap[x + 4][y + 2] = paintMode;
  bitMap[x + 5][y + 2] = paintMode;
  bitMap[x + 6][y + 2] = paintMode;
  bitMap[x + 7][y + 2] = paintMode;
  //4th row
  bitMap[x + 1][y + 3] = paintMode;
  bitMap[x + 2][y + 3] = paintMode;
  bitMap[x + 4][y + 3] = paintMode;
  bitMap[x + 5][y + 3] = paintMode;
  bitMap[x + 7][y + 3] = paintMode;
  bitMap[x + 8][y + 3] = paintMode;
  //5th row
  bitMap[x][y + 4] = paintMode;
  bitMap[x + 2][y + 4] = paintMode;
  bitMap[x + 3][y + 4] = paintMode;
  bitMap[x + 4][y + 4] = paintMode;
  bitMap[x + 5][y + 4] = paintMode;
  bitMap[x + 6][y + 4] = paintMode;
  bitMap[x + 7][y + 4] = paintMode;
  bitMap[x + 9][y + 4] = paintMode;
  //6th row
  bitMap[x][y + 5] = paintMode;
  bitMap[x + 2][y + 5] = paintMode;
  bitMap[x + 3][y + 5] = paintMode;
  bitMap[x + 4][y + 5] = paintMode;
  bitMap[x + 5][y + 5] = paintMode;
  bitMap[x + 6][y + 5] = paintMode;
  bitMap[x + 7][y + 5] = paintMode;
  bitMap[x + 9][y + 5] = paintMode;
  //7th row
  bitMap[x + 2][y + 6] = paintMode;
  bitMap[x + 7][y + 6] = paintMode;
  //8th row
  bitMap[x + 3][y + 7] = paintMode;
  bitMap[x + 6][y + 7] = paintMode;
}

void invertBitMap() {
  for (byte x = 0; x < 24; x++) {
    for (byte y = 0; y < 24; y++) {
      bitMap[x][y] = !bitMap[x][y];
    }
  }
}

void clearMenu() {
  switch (gb.menu(paintMenu, PAINTMENULENGTH)) {
    case  0: clearBitMap(false);          break;
    case  1: clearBitMap(true);           break;
    case  2: invertBitMap();              break;
    case -1:                              break;
    default:                              break;
  }
}

void cursorMenu() {
  switch (gb.menu(cursorChoser, CURSORMENULENGTH)) {
    case  0: cursorStep = !cursorStep;    break;
    case  1: cursorIcon = 0;              break;
    case  2: cursorIcon = 1;              break;
    case  3: cursorIcon = 2;              break;
    case  4: cursorIcon = 3;              break;
    case  5: cursorIcon = 4;              break;
    case -1:                              break;
    default:                              break;
  }
}

void clearBitMap(bool invert) {
  for (byte x = 0; x < 24; x++) {///initializing bitmap to zero
    for (byte y = 0; y < 24; y++) {
      bitMap[x][y] = invert;
    }
  }
}

void mem_check()
{
  void * heap_ptr = malloc(0);
  if ((!heap_ptr) || ((int)heap_ptr + 100 >= SP))
  {
    Serial.println("Out of memory!");
    while (1);
  }
  free(heap_ptr);
}

