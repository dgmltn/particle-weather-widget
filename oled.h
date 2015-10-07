#ifndef OLED_H
#define OLED_H

#include "SparkFunMicroOLED/SparkFunMicroOLED.h"

//////////////////////////////////
// MicroOLED Object Declaration //
//////////////////////////////////
#define PIN_RESET D7  // Connect RST to pin 7 (req. for SPI and I2C)
#define PIN_DC    D6  // Connect DC to pin 6 (required for SPI)
#define PIN_CS    A2 // Connect CS to pin A2 (required for SPI)
MicroOLED oled(MODE_SPI, PIN_RESET, PIN_DC, PIN_CS);

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 48

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

void drawXbm(int x, int y, int width, int height, const unsigned char *xbm);
void setCursor(int x, int y);
void printTitle(int x, String title, int font);
void writeNoWrap(char c);
void printNoWrap(String s);
void printNoWrap(int d);

#endif
