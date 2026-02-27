// SCLMotor.cpp
// Implementation of the SCL (Serial Command Language) library for the ST10-S.
//
// Protocol summary (PR4 mode, RS-232):
//   Host → Drive : "CMD[param]\r"
//   Drive → Host : one of
//     "XX=value\r"  for read queries   (IP, RS, VE, AC, …)
//     "%\r"         normal ack  – immediate/register-write commands executed
//     "*\r"         exception ack – buffered command placed in motion queue
//     "?\r" / "?N\r" nack – bad command or parameter out of range

#include "SCLMotor.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Maximum time to wait for any single drive response.
static const unsigned long ACK_TIMEOUT_MS = 500UL;

// ── Static helpers ────────────────────────────────────────────────────────────

// Discard any bytes already waiting in the RX buffer.
static void flushRx(SCLConfig* cfg) {
    while (cfg->port->available()) cfg->port->read();
}

// Read characters into buf[] until '\r' or timeout.
// Returns number of characters stored (NOT counting the '\r'), or -1 on timeout.
static int readLine(SCLConfig* cfg, char* buf, uint8_t maxLen) {
    unsigned long start = millis();
    uint8_t idx = 0;
    while (millis() - start < ACK_TIMEOUT_MS) {
        if (cfg->port->available()) {
            char c = (char)cfg->port->read();
            if (c == '\r') {
                buf[idx] = '\0';
                return (int)idx;
            }
            if (idx < maxLen - 1) buf[idx++] = c;
        }
    }
    buf[idx] = '\0';
    return -1; // timed out
}

// ── Initialisation ────────────────────────────────────────────────────────────

void sclBegin(SCLConfig* cfg) {
    cfg->position = 0;
    cfg->port->begin(cfg->baudRate);
    delay(100); // let the UART settle

    // 1. Enable Ack/Nack (PR4).  Use a fixed delay because acks may not be
    //    on yet – we cannot reliably wait for an ack on the very command that
    //    turns acks on.
    flushRx(cfg);
    cfg->port->print("PR4\r");
    delay(50);

    // 2. Switch immediate-command responses to decimal (IFD) so that IP
    //    returns plain integers instead of hex strings.
    flushRx(cfg);
    cfg->port->print("IFD\r");
    delay(50);

    flushRx(cfg);
}

// ── Low-level helpers ─────────────────────────────────────────────────────────

bool sclSend(SCLConfig* cfg, const char* cmd) {
    char buf[32];
    flushRx(cfg);
    cfg->port->print(cmd);
    cfg->port->print('\r');
    int n = readLine(cfg, buf, sizeof(buf));
    if (n < 0)        return false; // timeout
    if (buf[0] == '%') return true; // normal ack
    if (buf[0] == '*') return true; // exception ack (buffered motion command)
    if (buf[0] == '?') return false; // nack
    // Some drives echo data on write (edge case); treat non-empty reply as OK.
    return (n > 0);
}

bool sclQuery(SCLConfig* cfg, const char* cmd, char* resp, uint8_t maxLen) {
    flushRx(cfg);
    cfg->port->print(cmd);
    cfg->port->print('\r');
    int n = readLine(cfg, resp, maxLen);
    return (n > 0);
}

// ── Motor enable / disable ────────────────────────────────────────────────────

bool sclEnable(SCLConfig* cfg) {
    return sclSend(cfg, "ME");
}

bool sclDisable(SCLConfig* cfg) {
    return sclSend(cfg, "MD");
}

// ── Motion parameters ─────────────────────────────────────────────────────────

bool sclSetAccel(SCLConfig* cfg, float rpsps) {
    // AC range: 0.167 – 5461.167 rev/s², resolution 0.167 rev/s²
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "AC%.3f", rpsps);
    return sclSend(cfg, cmd);
}

bool sclSetDecel(SCLConfig* cfg, float rpsps) {
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "DE%.3f", rpsps);
    return sclSend(cfg, cmd);
}

bool sclSetVelocity(SCLConfig* cfg, float rps) {
    // VE range for ST10-S: 0.0042 – 80.0000 rev/s, resolution 0.0042 rev/s
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "VE%.4f", rps);
    return sclSend(cfg, cmd);
}

// ── Move commands ─────────────────────────────────────────────────────────────

bool sclMoveRelative(SCLConfig* cfg, long steps) {
    // FL[steps]: positive = CW, negative = CCW
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "FL%ld", steps);
    return sclSend(cfg, cmd);
}

bool sclMoveAbsolute(SCLConfig* cfg, long steps) {
    // FP[position]: move to absolute step count from SP0 origin
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "FP%ld", steps);
    return sclSend(cfg, cmd);
}

// ── Jogging ───────────────────────────────────────────────────────────────────

bool sclJogStart(SCLConfig* cfg, float rps) {
    // JS accepts negative values for CCW.
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "JS%.4f", rps);
    if (!sclSend(cfg, cmd)) return false;
    return sclSend(cfg, "CJ");
}

bool sclJogStop(SCLConfig* cfg) {
    return sclSend(cfg, "SJ");
}

// ── Stop ──────────────────────────────────────────────────────────────────────

bool sclStop(SCLConfig* cfg) {
    // SKD: decelerate at the DE rate and flush the queue.
    // Preferred for normal stops; motor comes to rest smoothly.
    return sclSend(cfg, "SKD");
}

bool sclEStop(SCLConfig* cfg) {
    // SK (no param): decelerate at the AM (maximum accel) rate and flush queue.
    // Use for emergency stops where the shortest stopping distance is needed.
    return sclSend(cfg, "SK");
}

// ── Position ──────────────────────────────────────────────────────────────────

long sclGetPosition(SCLConfig* cfg) {
    char resp[32];
    if (!sclQuery(cfg, "IP", resp, sizeof(resp))) return cfg->position;
    // Response (decimal format): "IP=10000" or "IP=-10000"
    char* eq = strchr(resp, '=');
    if (!eq) return cfg->position;
    long pos = atol(eq + 1);
    cfg->position = pos;
    return pos;
}

bool sclSetPosition(SCLConfig* cfg, long pos) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "SP%ld", pos);
    bool ok = sclSend(cfg, cmd);
    if (ok) cfg->position = pos;
    return ok;
}

// ── Status polling ────────────────────────────────────────────────────────────

// RS status character codes (from the drive manual):
//   A = Alarm present         D = Disabled
//   E = Drive fault           F = Motor moving
//   H = Homing in progress    J = Jogging
//   M = Motion in progress    P = In position
//   R = Ready                 S = Stopping
//   T = Wait time (WT)        W = Wait input (WI)

bool sclIsMoving(SCLConfig* cfg) {
    char resp[32];
    if (!sclQuery(cfg, "RS", resp, sizeof(resp))) return false;
    // Response: "RS=MR", "RS=PR", "RS=JR", etc.
    char* eq = strchr(resp, '=');
    if (!eq) return false;
    char* s = eq + 1;
    return (strchr(s, 'M') != nullptr ||
            strchr(s, 'J') != nullptr ||
            strchr(s, 'F') != nullptr ||
            strchr(s, 'H') != nullptr ||
            strchr(s, 'S') != nullptr);
}

bool sclWaitForMove(SCLConfig* cfg, unsigned long timeoutMs) {
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        if (!sclIsMoving(cfg)) return true;
        delay(20); // poll at ~50 Hz
    }
    return false; // timed out
}

// ── Alarms ────────────────────────────────────────────────────────────────────

bool sclHasAlarm(SCLConfig* cfg) {
    char resp[32];
    if (!sclQuery(cfg, "RS", resp, sizeof(resp))) return false;
    char* eq = strchr(resp, '=');
    if (!eq) return false;
    return strchr(eq + 1, 'A') != nullptr;
}

bool sclClearAlarm(SCLConfig* cfg) {
    // AR is an IMMEDIATE command; drive responds with '%' ack.
    return sclSend(cfg, "AR");
}
