/*
  user_config_override.h - HomeVoice's Tasmota build configuration.

  GOES AT:  tasmota/user_config_override.h  in the homevoice branch of the
  fork -- INSIDE the tasmota/ folder (the one full of .ino files). The
  first build ever made here had this file at the repo root, and the
  compiler, which reads only tasmota/user_config_override.h, produced a
  perfectly clean stock binary. The URL bar is the check: it must contain
  /homevoice/tasmota/.

  ONE IMAGE FOR EVERY BOARD. Nothing here is per-board; a driver compiled
  in is merely dormant until a template assigns its pins. The cost of a
  driver nobody uses is flash space and nothing else -- and the cost of a
  driver MISSING is the RC522 story: pins assigned, chip silent, an
  afternoon lost. So this file carries every driver the bench owns.

  Updated 2026-08-24: displays (LCD2004, TM1637 7-segment, MAX7219 dot
  matrix) and the MPU6050 IMU join the RC522. Every block below has a
  matching prove-it grep in .github/workflows/homevoice-build.yml -- a
  green run MEANS all of them are in the binary, not that the compiler
  had a pleasant time.

  Deliberately absent:
    - DS1302 real-time clock: Tasmota has no driver for its 3-wire
      protocol (DS3231 over I2C is the supported RTC, one define away if
      one is ever bought). In this house boards take time from the
      network anyway.
    - H-bridge / joystick / 74HC595 / every analog module: those need no
      define -- their drivers (relays+Interlock, ADC Joystick, 74x595,
      ADC) are already in the standard build, proven by bedroom's own
      short `GPIOs` answer.
*/
#ifndef _USER_CONFIG_OVERRIDE_H_
#define _USER_CONFIG_OVERRIDE_H_

/* ---- RFID card reader (RC522, SPI) -- since 2026-08-18 ------------- */
#define USE_SPI                       /* the bus itself is a compile option */
#define USE_RC522
#define USE_RC522_DATA_FUNCTION       /* read a text block off the card    */
#define USE_RC522_TYPE_INFORMATION    /* report the card kind (MIFARE 1KB) */

/* ---- Displays -- 2026-08-24 ---------------------------------------- */
/* None of these are in precompiled binaries ("included only in
   -displays.bin"). DisplayModel selects at runtime: 1 = character LCD,
   15 = TM1637/TM1638 7-segment, 19 = MAX7219 8x8 matrix. */
#define USE_DISPLAY
#define USE_DISPLAY_LCD               /* HD44780 2004/1602 on the I2C backpack */
#define USE_DISPLAY_TM1637            /* TM1637/TM1638 7-segment digits    */
#define USE_DISPLAY_MAX7219_MATRIX    /* 8x8 dot-matrix chains             */

/* ---- Motion IMU (MPU6050 gyro + accelerometer, I2C 0x68) ----------- */
#define USE_MPU6050

#endif
