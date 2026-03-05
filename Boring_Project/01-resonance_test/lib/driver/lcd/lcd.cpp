// lcd.cpp
// Thin LiquidCrystal hardware wrapper.
// The LiquidCrystal object is owned by the sketch; lcd.cpp stores only a pointer.

#include "lcd.h"

namespace {
  static LiquidCrystal* _lcd = nullptr;
}

namespace LCD {

void init(LiquidCrystal* lcd, uint8_t cols, uint8_t rows) {
  _lcd = lcd;
  if (_lcd) _lcd->begin(cols, rows);
}

void clear() {
  if (_lcd) _lcd->clear();
}

void setCursor(uint8_t col, uint8_t row) {
  if (_lcd) _lcd->setCursor(col, row);
}

void print(const char* text) {
  if (_lcd) _lcd->print(text);
}

} // namespace LCD
