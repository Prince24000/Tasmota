/*
  user_config_override.h - HomeVoice's Tasmota build configuration.

  DROP THIS FILE AT:  Tasmota/tasmota/user_config_override.h
  (the repo ships user_config_override_sample.h beside it; this replaces
  the step of copying and editing the sample)

  WHY THIS FILE EXISTS AT ALL: the RC522 RFID driver is not in any
  precompiled Tasmota binary -- Tasmota's own docs say so plainly -- and
  this project ships ONE image to every board. So the image becomes one we
  build ourselves, with the driver on. Everything else about the build
  stays stock: same version the fleet already runs (v15.5.0), so the
  SetOption numbers config.py was verified against keep their meanings.

  NEVER EDIT my_user_config.h -- it is upstream's file, and merges will
  eat your changes. This file is read after it and wins.

  Keep this file IN THE PROJECT REPO next to the firmware it produced.
  A firmware you cannot rebuild is a firmware you cannot fix.
*/

#ifndef _USER_CONFIG_OVERRIDE_H_
#define _USER_CONFIG_OVERRIDE_H_

// ── RC522 RFID reader (SPI) ─────────────────────────────────────────────
// USE_SPI is required: the bus itself is a compile option, and the RC522
// driver rides it. The two RC522 extras cost little flash and buy real
// information: DATA reads a text block off the card (up to 15 chars),
// TYPE reports what kind of card was presented ("MIFARE 1KB") -- both
// end up in the IrReceived-style tag telemetry the hub will listen for.
#define USE_SPI
#define USE_RC522
#define USE_RC522_DATA_FUNCTION
#define USE_RC522_TYPE_INFORMATION

#endif  // _USER_CONFIG_OVERRIDE_H_
