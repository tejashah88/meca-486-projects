/*
 * 16x2 LCD Demo
 * Uses the built-in LiquidCrystal library (no install needed).
 *
 * ── Wiring ────────────────────────────────────────────────────────────────────
 *
 *  LCD Pin │ Name │ Connect to
 *  ────────┼──────┼─────────────────────────────────────────────
 *     1    │ VSS  │ GND
 *     2    │ VDD  │ 5V
 *     3    │ V0   │ Potentiometer wiper (contrast, 10kΩ pot between 5V & GND)
 *     4    │ RS   │ Arduino pin 7
 *     5    │ RW   │ GND  (always write)
 *     6    │ EN   │ Arduino pin 8
 *     7-10 │ D0-3 │ Not connected (4-bit mode)
 *    11    │ D4   │ Arduino pin 4
 *    12    │ D5   │ Arduino pin 5
 *    13    │ D6   │ Arduino pin 6
 *    14    │ D7   │ Arduino pin 11
 *    15    │ A    │ 5V through 220Ω resistor  (backlight +)
 *    16    │ K    │ GND                        (backlight -)
 *
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include <LiquidCrystal.h>

// LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(7, 8, 4, 5, 6, 11);

// Custom character: a simple smiley face
byte smiley[8] = {
  0b00000,
  0b01010,
  0b01010,
  0b00000,
  0b10001,
  0b01110,
  0b00000,
  0b00000,
};

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, smiley);

  // ── Scene 1: static greeting ──────────────────────────────────────────────
  lcd.setCursor(0, 0);
  lcd.print("  Hello, World! ");
  lcd.setCursor(0, 1);
  lcd.print("   MECA  486    ");
  delay(2000);

  // ── Scene 2: smiley + custom char demo ───────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Custom char:");
  lcd.setCursor(13, 0);
  lcd.write(byte(0));   // smiley
  delay(2000);
}

void loop() {
  // ── Scene 3: scrolling marquee ────────────────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Scrolling...  ");

  String msg = "  16x2 LCD on Arduino!  ";
  for (int i = 0; i < (int)msg.length(); i++) {
    lcd.setCursor(0, 1);
    lcd.print(msg.substring(i, i + 16).c_str());
    delay(300);
  }
  delay(500);

  // ── Scene 4: live counter ─────────────────────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Counter:");
  for (int count = 0; count <= 20; count++) {
    lcd.setCursor(9, 0);
    lcd.print(count);
    lcd.print("  "); // clear trailing digits
    delay(200);
  }
  delay(1000);
}
