// SCLMotor.h
// Serial Command Language (SCL) library for Applied Motion ST10-S stepper driver.
// Communicates over any Arduino HardwareSerial port at 9600 baud (default).
//
// Quick-start:
//   SCLConfig axis;
//   axis.port     = &Serial1;
//   axis.baudRate = 9600;
//   sclBegin(&axis);            // start serial, enable Ack/Nack, decimal format
//   sclEnable(&axis);           // ME – energise motor
//   sclSetAccel(&axis, 50.0f);  // AC50
//   sclSetDecel(&axis, 50.0f);  // DE50
//   sclSetVelocity(&axis, 5.0f);// VE5
//   sclMoveRelative(&axis, 2000);// FL2000
//   sclWaitForMove(&axis, 5000); // block until done (5 s timeout)
//
// Protocol notes (Applied Motion SCL, PR4 mode):
//   - Commands are ASCII strings terminated with '\r' (no '\n').
//   - Ack/Nack enabled (PR4):
//       '%' = normal ack  (immediate or register-write commands)
//       '*' = exception ack (command placed in motion queue)
//       '?' = nack (followed by optional error-code digit)
//   - Read queries return "XX=value\r" as the sole response (no extra ack).
//   - Decimal format (IFD) is set by sclBegin so position values are plain integers.

#ifndef SCL_MOTOR_H
#define SCL_MOTOR_H

#include <Arduino.h>

// ── Configuration struct ──────────────────────────────────────────────────────
struct SCLConfig {
    HardwareSerial* port;       // hardware serial port  (Serial1, Serial2, …)
    uint32_t        baudRate;   // baud rate matching driver setting (default 9600)
    long            position;   // last position read by sclGetPosition() [steps]
};

// ── Initialisation ────────────────────────────────────────────────────────────
// Opens the serial port, enables Ack/Nack (PR4), and sets decimal response
// format (IFD).  Call once in setup() before any other SCL function.
void sclBegin(SCLConfig* cfg);

// ── Motor enable / disable ────────────────────────────────────────────────────
bool sclEnable(SCLConfig* cfg);   // ME – energise motor
bool sclDisable(SCLConfig* cfg);  // MD – de-energise motor

// ── Motion parameters ─────────────────────────────────────────────────────────
// All buffered – take effect for the next move command.
bool sclSetAccel(SCLConfig* cfg, float rpsps);    // AC – accel  [rev/s²]
bool sclSetDecel(SCLConfig* cfg, float rpsps);    // DE – decel  [rev/s²]
bool sclSetVelocity(SCLConfig* cfg, float rps);   // VE – cruise [rev/s]

// ── Move commands ─────────────────────────────────────────────────────────────
// Both use the last AC / DE / VE values.
bool sclMoveRelative(SCLConfig* cfg, long steps); // FL – relative move [steps]
bool sclMoveAbsolute(SCLConfig* cfg, long steps); // FP – absolute move [steps]

// ── Jogging ───────────────────────────────────────────────────────────────────
// sclJogStart sets JS then sends CJ (Commence Jogging).
// sclJogStop  sends SJ.  Direction is sign of rps (positive = CW, negative = CCW).
bool sclJogStart(SCLConfig* cfg, float rps);
bool sclJogStop(SCLConfig* cfg);

// ── Stop ──────────────────────────────────────────────────────────────────────
// sclStop  – SKD: decelerate using DE rate then flush queue (controlled stop).
// sclEStop – SK:  decelerate using AM (max-accel) rate then flush queue (fast stop).
bool sclStop(SCLConfig* cfg);
bool sclEStop(SCLConfig* cfg);

// ── Position ──────────────────────────────────────────────────────────────────
// sclGetPosition queries the drive and caches result in cfg->position.
// sclSetPosition sends SP to redefine the origin (also caches locally).
long sclGetPosition(SCLConfig* cfg);
bool sclSetPosition(SCLConfig* cfg, long pos);

// ── Status polling ────────────────────────────────────────────────────────────
// sclIsMoving  returns true if drive status contains M, J, F, H, or S.
// sclWaitForMove blocks until drive is idle or timeout (ms) expires.
//   Returns true = move finished, false = timed out.
bool sclIsMoving(SCLConfig* cfg);
bool sclWaitForMove(SCLConfig* cfg, unsigned long timeoutMs);

// ── Alarms ────────────────────────────────────────────────────────────────────
bool sclHasAlarm(SCLConfig* cfg);   // true if 'A' in RS status
bool sclClearAlarm(SCLConfig* cfg); // AR – reset alarm

// ── Low-level helpers ─────────────────────────────────────────────────────────
// sclSend  – write command, wait for ack (* / %) or nack (?).
//   Returns true on success, false on nack or timeout.
bool sclSend(SCLConfig* cfg, const char* cmd);

// sclQuery – write command, read "XX=value\r" response into resp[].
//   Returns true if a non-empty response arrived before timeout.
bool sclQuery(SCLConfig* cfg, const char* cmd, char* resp, uint8_t maxLen);

#endif // SCL_MOTOR_H
