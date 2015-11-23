#line 1 "GambiPaint.ino"
#include <EEPROM.h>
#include <SPI.h>
#include <Gamebuino.h>
#include <avr/pgmspace.h>

#include "Arduino.h"
void setup();
void loop();
void drawInvader(byte x, byte y);
void invertBitMap();
void clearMenu();
void clearBitMap(bool invert);
void mem_check();
#line 6
Gamebuino gb;
bool btnUp = false;
bool btnDown = false;
bool btnLeft = false;
bool btnRight = false;
bool btnPaint = false;
byte posX = 0;
byte posY = 0;
bool bitMap[24][24];                                 
bool showInfo = true;
bool menuLoop = false;
bool paintMode = true;                                
bool brushBigs = false;                                            
bool brushSmalls = false;                                                            
#define eepAddr 0

          
const byte pencilButton[] PROGMEM = {16,10,
B00111111,B11000000,
B01000000,B00000000,
B10000100,B01000000,
B10000111,B11000000,
B10000100,B01000000,
B10000010,B10000000,
B10100001,B00000000,
B10011110,B00000000,
B01000000,B00000000,
B00111111,B11000000,
};

const byte eraseButton[] PROGMEM = {16,10,
B00111111,B11000000,
B01000000,B00000000,
B10000011,B11000000,
B10000100,B01000000,
B10001000,B10000000,
B10010001,B00000000,
B10111110,B00000000,
B10000000,B00000000,
B01000000,B00000000,
B00111111,B11000000,
};

const byte radioButton[] PROGMEM = {16,10,
B11111111,B00000000,
B00000000,B10000000,
B00111100,B01000000,
B01100110,B01000000,
B01000010,B01000000,
B01000010,B01000000,
B01100110,B01000000,
B00111100,B01000000,
B00000000,B10000000,
B11111111,B00000000,
};

const byte emptyButton[] PROGMEM = {16,10,
B00111111,B11000000,
B01000000,B00000000,
B10000000,B00000000,
B10000000,B00000000,
B10000000,B00000000,
B10000000,B00000000,
B10000000,B00000000,
B10000000,B00000000,
B01000000,B00000000,
B00111111,B11000000,
};

const byte invaderButton[] PROGMEM = {16,10,
B00111111,B11000000,
B01010000,B10000000,
B10001001,B00000000,
B10011111,B10000000,
B10110110,B11000000,
B11011111,B10100000,
B11011111,B10100000,
B10010000,B10000000,
B01001001,B00000000,
B00111111,B11000000,
};

                
#define MENULENGTH 4
const char strLoad[] PROGMEM = "Load from eep";
const char strSave[] PROGMEM = "Save to eep";
const char strPaintOps[] PROGMEM = "Canvas operations";
const char strQuit[] PROGMEM = "Quit GambiPaint";
const char* const menu[MENULENGTH] PROGMEM = {
  strLoad,
  strSave,
  strPaintOps,
  strQuit,
};

                 
#define PAINTMENULENGTH 3
const char strClear[] PROGMEM = "Clear painting";
const char strFill[] PROGMEM = "Fill painting";
const char strInvert[] PROGMEM = "Invert painting";
const char* const paintMenu[PAINTMENULENGTH] PROGMEM = {
  strClear,
  strFill,
  strInvert,
};

                                                       
void setup() {
                                
  clearBitMap(false);
                                    
  gb.begin();
                          
  gb.titleScreen(F("GambiPaint, for painting, ykno"));
  gb.popup(F("Taim 2 Paent!"), 40);

  mem_check();
}

                                                    
void loop() {
                                                                                    
                                                                       
  if (gb.update()) {
                                                  
    gb.display.drawPixel(0, 0);
    gb.display.drawPixel(0, 47);
    gb.display.drawPixel(83, 0);
    gb.display.drawPixel(83, 47);

    if (gb.buttons.pressed(BTN_UP)) {
      btnUp = true;
    } else if (gb.buttons.released(BTN_UP)) {
      btnUp = false;
    }
    if (gb.buttons.pressed(BTN_DOWN)) {
      btnDown = true;
    } else if (gb.buttons.released(BTN_DOWN)) {
      btnDown = false;
    }
    if (gb.buttons.pressed(BTN_LEFT)) {
      btnLeft = true;
    } else if (gb.buttons.released(BTN_LEFT)) {
      btnLeft = false;
    }
    if (gb.buttons.pressed(BTN_RIGHT)) {
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

                               
    if (btnUp)    posY--;
    if (btnDown)  posY++;
    if (btnLeft)  posX--;
    if (btnRight) posX++;

    if (posX > 83)            posX = 0;
    if (posX == 0 && btnLeft) posX = 83;
    if (posY > 47)            posY = 0;
    if (posY == 0 && btnUp)   posY = 47;

                          
    if (gb.buttons.pressed(BTN_A)) {
      if (posX >= 68 && posX <=73) {
        if (posY >= 4 && posY <= 9) {                        
          paintMode = true;
        } else if (posY >= 15 && posY <= 20) {                       
          paintMode = false;
        }
      }
    }

                         
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
    
                             
    gb.display.drawRect(29, 0, 26, 26);
    
                              
    for (byte x = 0; x < 24; x++) {
      for (byte y = 0; y < 24; y++) {
        if (bitMap[x][y]) gb.display.drawPixel(x+30, y+1);
      }
    }

    if (posX > 28 && posX < 54) {
      if (posY < 25 && posY > 0) {
        if (btnPaint) {
          if (!(brushBigs && brushSmalls)) bitMap[posX-30][posY-1] = paintMode                            ;                           
          if ((!brushBigs && brushSmalls) || (brushBigs && !brushSmalls)) {                  
            if (posY < 24) {
              bitMap[(posX-30)][(posY-1)+1] = paintMode;
            }
            if (posX < 53) {
              bitMap[(posX-30+1)][(posY-1)] = paintMode;
              if (posY < 24) {
                bitMap[(posX-30)+1][(posY-1)+1] = paintMode;
              }
            }
          }
          if (brushBigs && !brushSmalls) {
            if (posY < 23) {
              bitMap[(posX-30)][(posY-1)+2] = paintMode;
            }
            if (posX <52) {
              bitMap[(posX-30+2)][(posY-1)] = paintMode;
              if (posY < 23) {
                bitMap[(posX-30)+2][(posY-1)+2] = paintMode;
                bitMap[(posX-30)+1][(posY-1)+2] = paintMode;
                bitMap[(posX-30)+2][(posY-1)+1] = paintMode;
              }
            }
          }
          if (brushBigs && brushSmalls) {
            if (posY < 18 && posY > 0 && posX < 45 && posX > 29) {
                                    
              drawInvader(posX-30, posY-1);
            }
          }
        }
      }
    }

                 
    if (showInfo) {
      gb.display.cursorX = 1;
      gb.display.print(F("Info\r\n"));
      gb.display.print(F("posX:"));
      gb.display.println(posX);
      gb.display.print(F("posY:"));
      gb.display.println(posY);
      gb.display.print(F("\27Menu\r\n"));
    }
    gb.display.drawPixel(posX, posY);                                 

    gb.display.drawPixel(posX-2, posY);
    gb.display.drawPixel(posX-3, posY);

    gb.display.drawPixel(posX+2, posY);
    gb.display.drawPixel(posX+3, posY);

    gb.display.drawPixel(posX, posY-2);
    gb.display.drawPixel(posX, posY-3);

    gb.display.drawPixel(posX, posY+2);
    gb.display.drawPixel(posX, posY+3);

              
            
    gb.display.drawBitmap(57, 2, pencilButton);
    gb.display.drawBitmap(67, 2, radioButton);
           
    gb.display.drawBitmap(57, 13, eraseButton);
    gb.display.drawBitmap(67, 13, radioButton);
           
    gb.display.drawBitmap(1, 27, emptyButton);
    gb.display.drawBitmap(11, 27, radioButton);
    gb.display.drawPixel(4, 33);
             
    gb.display.drawBitmap(1, 38, emptyButton);
    gb.display.drawBitmap(11, 38, radioButton);
    gb.display.fillRect(4, 43, 2, 2);
            
    gb.display.drawBitmap(22, 27, emptyButton);
    gb.display.drawBitmap(32, 27, radioButton);
    gb.display.fillRect(25, 31, 3, 3);
           
    gb.display.drawBitmap(22, 38, invaderButton);
    gb.display.drawBitmap(32, 38, radioButton);
    
    if (paintMode) {
      gb.display.fillRect(70, 6, 2, 2);
    } else {
      gb.display.fillRect(70, 17, 2, 2);
    }
    if (!brushBigs && !brushSmalls) {            
      gb.display.fillRect(14, 31, 2, 2);
    } else if (!brushBigs && brushSmalls) {             
      gb.display.fillRect(14, 42, 2, 2);
    } else if (brushBigs && !brushSmalls) {          
      gb.display.fillRect(35, 31, 2, 2);
    } else if (brushBigs && brushSmalls) {            
      gb.display.fillRect(35, 42, 2, 2);
    }
    
  }

  if (gb.buttons.pressed(BTN_C)) {
    switch (gb.menu(menu, MENULENGTH)) {
      case  0: EEPROM.get(eepAddr, bitMap); break;
      case  1: EEPROM.put(eepAddr, bitMap); break;
      case  2: clearMenu();                 break;
      case  3: gb.changeGame();             break;
      case -1:                              break;
      default:                              break;
    }
  }

  mem_check();

}

void drawInvader(byte x, byte y) {
           
  bitMap[x+2][y] = paintMode;
  bitMap[x+7][y] = paintMode;
           
  bitMap[x+3][y+1] = paintMode;
  bitMap[x+6][y+1] = paintMode;
           
  bitMap[x+2][y+2] = paintMode;
  bitMap[x+3][y+2] = paintMode;
  bitMap[x+4][y+2] = paintMode;
  bitMap[x+5][y+2] = paintMode;
  bitMap[x+6][y+2] = paintMode;
  bitMap[x+7][y+2] = paintMode;
           
  bitMap[x+1][y+3] = paintMode;
  bitMap[x+2][y+3] = paintMode;
  bitMap[x+4][y+3] = paintMode;
  bitMap[x+5][y+3] = paintMode;
  bitMap[x+7][y+3] = paintMode;
  bitMap[x+8][y+3] = paintMode;
           
  bitMap[x][y+4] = paintMode;
  bitMap[x+2][y+4] = paintMode;
  bitMap[x+3][y+4] = paintMode;
  bitMap[x+4][y+4] = paintMode;
  bitMap[x+5][y+4] = paintMode;
  bitMap[x+6][y+4] = paintMode;
  bitMap[x+7][y+4] = paintMode;
  bitMap[x+9][y+4] = paintMode;
           
  bitMap[x][y+5] = paintMode;
  bitMap[x+2][y+5] = paintMode;
  bitMap[x+3][y+5] = paintMode;
  bitMap[x+4][y+5] = paintMode;
  bitMap[x+5][y+5] = paintMode;
  bitMap[x+6][y+5] = paintMode;
  bitMap[x+7][y+5] = paintMode;
  bitMap[x+9][y+5] = paintMode;
           
  bitMap[x+2][y+6] = paintMode;
  bitMap[x+7][y+6] = paintMode;
           
  bitMap[x+3][y+7] = paintMode;
  bitMap[x+6][y+7] = paintMode;
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
    case  2: invertBitMap();               break;
    case -1:                              break;
    default:                              break;
  }
}

void clearBitMap(bool invert) {
   for (byte x = 0; x < 24; x++) {                              
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


