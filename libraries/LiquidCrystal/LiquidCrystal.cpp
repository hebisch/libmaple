#include "LiquidCrystal.h"

#include <stdio.h>
#include <string.h>
#include <wirish/WProgram.h>

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

// This library has been modified to be compatible with the LeafLabs Maple;
// very conservative timing is used due to problems with delayMicroseconds()
// that should be fixed in the 0.0.7 release of the libmaple. [bnewbold]

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

LiquidCrystal::LiquidCrystal(uint8_t rs,  uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

void LiquidCrystal::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw,
                         uint8_t enable, uint8_t d0, uint8_t d1,
                         uint8_t d2, uint8_t d3, uint8_t d4,
                         uint8_t d5, uint8_t d6, uint8_t d7)
{
  _rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;
  _displayfunction = fourbitmode?LCD_4BITMODE:LCD_8BITMODE;

  _data_pins[0] = d0;
  _data_pins[1] = d1;
  _data_pins[2] = d2;
  _data_pins[3] = d3;
  _data_pins[4] = d4;
  _data_pins[5] = d5;
  _data_pins[6] = d6;
  _data_pins[7] = d7;
}

void LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  uint8_t fourbitmode = !(_displayfunction&LCD_8BITMODE);
  for (int i = 0; i < 8 - fourbitmode * 4; i++) {
      pinMode(_data_pins[i], OUTPUT);
  }

  pinMode(_rs_pin, OUTPUT);
  // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
  if (_rw_pin != 255) {
    pinMode(_rw_pin, OUTPUT);
  }
  pinMode(_enable_pin, OUTPUT);

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!  according to
  // datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so
  // we'll wait 50
  delay(50);    // Maple mod
  //delayMicroseconds(50000);
  // Now we pull both RS and R/W low to begin commands
  digitalWrite(_rs_pin, LOW);
  digitalWrite(_enable_pin, LOW);
  if (_rw_pin != 255) {
    digitalWrite(_rw_pin, LOW);
  }

  //if needed put the LCD into 4 bit mode
  if (! (_displayfunction & LCD_8BITMODE)) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delay(5);   // Maple mod
    //delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delay(5);   // Maple mod
    //delayMicroseconds(4500); // wait min 4.1ms

    // third go!
    write4bits(0x03);
    delay(1);   // Maple mod
    //delayMicroseconds(150);

    // finally, set to 8-bit interface
    write4bits(0x02);
  }
  LiquidCrystalBase::begin(cols, lines, dotsize);
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LiquidCrystal::send(uint8_t value, uint8_t mode) {
  digitalWrite(_rs_pin, mode);

  // if there is a RW pin indicated, set it low to Write
  if (_rw_pin != 255) {
    digitalWrite(_rw_pin, LOW);
  }

  if (_displayfunction & LCD_8BITMODE) {
    write8bits(value);
  } else {
    write4bits(value>>4);
    write4bits(value);
  }
}

void LiquidCrystal::pulseEnable(void) {
  // _enable_pin should already be LOW (unless someone else messed
  // with it), so don't sit around waiting for long.
  digitalWrite(_enable_pin, LOW);
  delayMicroseconds(2);

  // Enable pulse must be > 450 ns.  Value chosen here according to
  // the following threads:
  // http://forums.leaflabs.com/topic.php?id=640
  // http://forums.leaflabs.com/topic.php?id=512
  togglePin(_enable_pin);
  delayMicroseconds(2);
  togglePin(_enable_pin);

  // Commands needs > 37us to settle.
  delayMicroseconds(50);
}

void LiquidCrystal::write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }

  pulseEnable();
}

void LiquidCrystal::write8bits(uint8_t value) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }

  pulseEnable();
}
