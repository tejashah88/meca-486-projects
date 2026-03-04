// Motor Music — Z-Axis Stepper Motor
// Step: D25, Dir: D24

#define STEP_PIN 25
#define DIR_PIN  24

// ── Note frequencies (Hz) — C3 to C7 ─────────────────────────────────────
#define REST  0
#define C3   131
#define CS3  139
#define D3   147
#define DS3  156
#define E3   165
#define F3   175
#define FS3  185
#define G3   196
#define GS3  208
#define A3   220
#define AS3  233
#define B3   247
#define C4   262
#define CS4  277
#define D4   294
#define DS4  311
#define E4   330
#define F4   349
#define FS4  370
#define G4   392
#define GS4  415
#define A4   440
#define AS4  466
#define B4   494
#define C5   523
#define CS5  554
#define D5   587
#define DS5  622
#define E5   659
#define F5   698
#define FS5  740
#define G5   784
#define GS5  831
#define A5   880
#define AS5  932
#define B5   988
#define C6  1047
#define CS6 1109
#define D6  1175
#define DS6 1245
#define E6  1319
#define F6  1397
#define FS6 1480
#define G6  1568
#define GS6 1661
#define A6  1760
#define AS6 1865
#define B6  1976
#define C7  2093

// ── Tempo ─────────────────────────────────────────────────────────────────
#define BPM   84
#define S16   (60000 / BPM / 4)
#define S8    (60000 / BPM / 2)
#define S8D   (S8 + S16)           // dotted eighth
#define S4    (60000 / BPM)
#define S4D   (S4 + S8)            // dotted quarter
#define S2    (60000 * 2 / BPM)
#define S1    (60000 * 4 / BPM)

#define GAP   20  // ms silence between notes

// ── Melody ────────────────────────────────────────────────────────────────
struct Note { int freq; int dur; };

#include "Bohemian-Rhapsody-1_notes.h"
#include "Rasputin_notes.h"
#include "R.Astley.Never gonna give you up K_notes.h"
// Stay With Me (Sam Smith)
const Note stayWithMe[] PROGMEM = {
  // ── Verse 1 ────────────────────────────────────────────────────────────
  // "Guess it's true, I'm not good at a one-night stand"
  {A4,S8},{G4,S8},{E4,S4},{REST,S8},
  {E4,S8},{D4,S8},{E4,S8},{G4,S8},{A4,S4},
  {G4,S8},{E4,S4},{REST,S4},

  // "But I still need love 'cause I'm just a man"
  {E4,S8},{D4,S8},{E4,S8},{G4,S8},
  {A4,S8},{G4,S8},{E4,S8},{D4,S8},{C4,S2},

  // "These nights never seem to go to plan"
  {A4,S8},{G4,S8},{E4,S4},{REST,S8},
  {E4,S8},{D4,S8},{E4,S8},{G4,S8},{A4,S4},
  {G4,S8},{E4,S4},{REST,S4},

  // "I don't want you to leave, will you hold my hand?"
  {E4,S8},{D4,S8},{E4,S8},{G4,S8},
  {A4,S8},{G4,S8},{E4,S8},{D4,S8},{C4,S2},

  // ── Pre-chorus ─────────────────────────────────────────────────────────
  // "Oh, won't you stay with me?"
  {E4,S4D},{E4,S8},{G4,S8},{A4,S8},
  {G4,S8},{E4,S8},{D4,S4},{REST,S4},

  // "'Cause you're all I need"
  {E4,S8},{G4,S8},{A4,S4},
  {G4,S8},{E4,S8},{D4,S4},{REST,S4},

  // "This ain't love, it's clear to see"
  {G4,S8},{A4,S8},{C5,S8},{B4,S8},
  {A4,S8},{G4,S8},{E4,S4},{REST,S4},

  // "But darling, stay with me"
  {G4,S8},{A4,S8},{G4,S8},
  {E4,S8},{D4,S8},{C4,S2},

  // ── Verse 2 ────────────────────────────────────────────────────────────
  // "Why am I so emotional?"
  {A4,S8},{G4,S8},{E4,S4},{REST,S8},
  {E4,S8},{D4,S8},{E4,S8},{G4,S8},{A4,S4},
  {G4,S8},{E4,S4},{REST,S4},

  // "No, it's not a good look, gain some self-control"
  {E4,S8},{D4,S8},{E4,S8},{G4,S8},
  {A4,S8},{G4,S8},{E4,S8},{D4,S8},{C4,S2},

  // "And deep down I know this never works"
  {A4,S8},{G4,S8},{E4,S4},{REST,S8},
  {E4,S8},{D4,S8},{E4,S8},{G4,S8},{A4,S4},
  {G4,S8},{E4,S4},{REST,S4},

  // "But you can lay with me so it doesn't hurt"
  {E4,S8},{D4,S8},{E4,S8},{G4,S8},
  {A4,S8},{G4,S8},{E4,S8},{D4,S8},{C4,S2},

  // ── Chorus (x2) ────────────────────────────────────────────────────────
  // "Oh, won't you stay with me?"
  {E4,S4D},{E4,S8},{G4,S8},{A4,S8},
  {G4,S8},{E4,S8},{D4,S4},{REST,S4},

  // "'Cause you're all I need"
  {E4,S8},{G4,S8},{A4,S4},
  {G4,S8},{E4,S8},{D4,S4},{REST,S4},

  // "This ain't love, it's clear to see"
  {G4,S8},{A4,S8},{C5,S8},{B4,S8},
  {A4,S8},{G4,S8},{E4,S4},{REST,S4},

  // "But darling, stay with me"
  {G4,S8},{A4,S8},{G4,S8},
  {E4,S8},{D4,S8},{C4,S2},

  // repeat chorus
  {E4,S4D},{E4,S8},{G4,S8},{A4,S8},
  {G4,S8},{E4,S8},{D4,S4},{REST,S4},

  {E4,S8},{G4,S8},{A4,S4},
  {G4,S8},{E4,S8},{D4,S4},{REST,S4},

  {G4,S8},{A4,S8},{C5,S8},{B4,S8},
  {A4,S8},{G4,S8},{E4,S4},{REST,S4},

  {G4,S8},{A4,S8},{G4,S8},
  {E4,S8},{D4,S8},{C4,S1},

  {REST,S8},
};

const int LEN_STAY_WITH_ME = sizeof(stayWithMe) / sizeof(stayWithMe[0]);

// Active song selector (add more songs later)
enum SongId {
  SONG_STAY_WITH_ME    = 0,
  SONG_BOHEMIAN_RHAPSODY,
  SONG_RASPUTIN,
  SONG_NEVER_GONNA_GIVE_YOU_UP,
};

const SongId ACTIVE_SONG = SONG_NEVER_GONNA_GIVE_YOU_UP;

// ─────────────────────────────────────────────────────────────────────────

void playNote(int freqHz, int durationMs) {
  int playMs = durationMs - GAP;
  if (playMs < 1) playMs = 1;

  if (freqHz == REST) {
    delay(durationMs);
    return;
  }

  unsigned long halfPeriod = 500000UL / freqHz;
  unsigned long start = millis();
  while ((long)(millis() - start) < playMs) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(halfPeriod);
  }
  delay(GAP);
}

void playSong(const Note* song, int length) {
  for (int i = 0; i < length; i++) {
    Note n;
    memcpy_P(&n, &song[i], sizeof(Note));
    playNote(n.freq, n.dur);
  }
}

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN,  OUTPUT);
  digitalWrite(DIR_PIN,  LOW);
  digitalWrite(STEP_PIN, LOW);
}

void loop() {
  switch (ACTIVE_SONG) {
    case SONG_BOHEMIAN_RHAPSODY:
      playSong(bohemianRhapsody1, LEN_BOHEMIANRHAPSODY1);
      break;
    case SONG_STAY_WITH_ME:
      playSong(stayWithMe, LEN_STAY_WITH_ME);
      break;
    case SONG_RASPUTIN:
      playSong(rasputin, LEN_RASPUTIN);
      break;
    case SONG_NEVER_GONNA_GIVE_YOU_UP:
      playSong(neverGonnaGiveYouUp, LEN_neverGonnaGiveYouUp);
      break;
    default:
      break;
  }
  delay(1500);
}
