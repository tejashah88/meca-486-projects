// lcd.h
// Thin hardware wrapper around LiquidCrystal.
// Call LCD::init() once in setup() to register the display object.
// All subsequent calls operate on the stored pointer — no MotorConfig dependency.

#pragma once

#include <LiquidCrystal.h>

namespace LCD {

  // Store the display pointer and call lcd->begin(cols, rows).
  // Must be called before any other LCD function.
  void init(LiquidCrystal* lcd, uint8_t cols = 16, uint8_t rows = 2);

  void clear();
  void setCursor(uint8_t col, uint8_t row);
  void print(const char* text);

} // namespace LCD
