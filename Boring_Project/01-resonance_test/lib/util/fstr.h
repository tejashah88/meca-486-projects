// fstr.h
// Float-to-C-string helper for AVR Arduino.
// AVR's snprintf does not support %f; use fstr() to format floats inline.
//
// Usage:  snprintf(buf, sizeof(buf), "%s RPS", fstr(val, 2));
//
// NOTE: uses a single static buffer — do NOT call fstr() twice in the same
// expression (e.g. two arguments to the same snprintf). Make two snprintf
// calls instead.

#pragma once

inline const char* fstr(float val, uint8_t decimals = 1) {
  static char _buf[12];
  dtostrf(val, 1, decimals, _buf);
  return _buf;
}
