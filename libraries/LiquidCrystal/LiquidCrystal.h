#ifndef LiquidCrystal_h
#define LiquidCrystal_h

//#include <inttypes.h>
#include <LiquidCrystal/LiquidCrystalBase.h>
#include <wirish/wirish.h>
#include <wirish/Print.h>

class LiquidCrystal : public LiquidCrystalBase {
public:
  LiquidCrystal(uint8 rs, uint8 enable,
                uint8 d0, uint8 d1, uint8 d2, uint8 d3,
                uint8 d4, uint8 d5, uint8 d6, uint8 d7);
  LiquidCrystal(uint8 rs, uint8 rw, uint8 enable,
                uint8 d0, uint8 d1, uint8 d2, uint8 d3,
                uint8 d4, uint8 d5, uint8 d6, uint8 d7);
  LiquidCrystal(uint8 rs, uint8 rw, uint8 enable,
                uint8 d0, uint8 d1, uint8 d2, uint8 d3);
  LiquidCrystal(uint8 rs, uint8 enable,
                uint8 d0, uint8 d1, uint8 d2, uint8 d3);
  void begin(uint8 cols, uint8 rows, uint8 charsize = LCD_5x8DOTS);
protected:
  void send(uint8_t, uint8_t);
private:
  void init(uint8 fourbitmode, uint8 rs, uint8 rw, uint8 enable,
            uint8 d0, uint8 d1, uint8 d2, uint8 d3,
            uint8 d4, uint8 d5, uint8 d6, uint8 d7);
  void write4bits(uint8);
  void write8bits(uint8);
  void pulseEnable();

  uint8 _rs_pin; // LOW: command.  HIGH: character.
  uint8 _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  uint8 _enable_pin; // activated by a HIGH pulse.
  uint8 _data_pins[8];

};

#endif
