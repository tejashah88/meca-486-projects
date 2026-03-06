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

  // Clear all characters and return the cursor to (0, 0).
  void clear();

  // Move the cursor to column col, row row (both 0-indexed).
  void setCursor(uint8_t col, uint8_t row);

  // Write a null-terminated string at the current cursor position.
  void print(const char* text);

} // namespace LCD
