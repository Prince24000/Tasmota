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

/* ==== THE I2C SWEEP -- 2026-08-25 ===================================
   Every catalogue entry added in the sweep has its driver named here,
   and every define here has a string-proof grep in homevoice-build.yml
   (the driver's own tele-SENSOR name, searched in the BUILT BINARY --
   the check_firmware.py method promoted to CI). A green run MEANS all
   of these are in the image.

   #ifndef-guarded because several are already on in Tasmota's defaults;
   redefining one would only make the compiler grumble about a fact.  */

/* -- temperature / humidity / pressure ------------------------------- */
#ifndef USE_SHT3X
#define USE_SHT3X                 /* SHT30/31/35 @ 0x44-0x45             */
#endif
#ifndef USE_AHT2x
#define USE_AHT2x                 /* AHT20/21 @ 0x38                     */
#endif
#ifndef USE_HTU
#define USE_HTU                   /* HTU21 / SI7021 (GY-21) @ 0x40       */
#endif
#ifndef USE_AM2320
#define USE_AM2320                /* AM2320 @ 0x5C                       */
#endif
#ifndef USE_BMP
#define USE_BMP                   /* BMP085/180/280 + BME280 @ 0x76-77   */
#endif
#ifndef USE_BME68X
#define USE_BME68X                /* BME680/688 @ 0x76-77                */
#endif
#ifndef USE_MLX90614
#define USE_MLX90614              /* IR thermometer @ 0x5A               */
#endif

/* -- soil ------------------------------------------------------------- */
#ifndef USE_SEESAW_SOIL
#define USE_SEESAW_SOIL           /* Adafruit capacitive @ 0x36          */
#endif
#ifndef USE_CHIRP
#define USE_CHIRP                 /* Chirp! @ 0x20                       */
#endif

/* -- distance / motion ------------------------------------------------ */
#ifndef USE_VL53L0X
#define USE_VL53L0X               /* laser ToF 2 m @ 0x29                */
#endif
#ifndef USE_VL53L1X
#define USE_VL53L1X               /* laser ToF 4 m @ 0x29 (fit ONE)      */
#endif
#ifndef USE_QMC5883L
#define USE_QMC5883L              /* GY-271 clone compass @ 0x0D         */
#endif
#ifndef USE_HMC5883L
#define USE_HMC5883L              /* genuine Honeywell compass @ 0x1E    */
#endif

/* -- electrical ------------------------------------------------------- */
#ifndef USE_ADS1115
#define USE_ADS1115               /* 4-ch 16-bit ADC @ 0x48-0x4B         */
#endif
#ifndef USE_INA226
#define USE_INA226                /* DC V/A @ 0x40+ -- jumper it off     */
#endif                            /*   0x40; the HTU21 lives there       */

/* -- weather ---------------------------------------------------------- */
#ifndef USE_AS3935
#define USE_AS3935                /* lightning detector @ 0x03           */
#endif

/* -- air quality ------------------------------------------------------ */
#ifndef USE_SGP30
#define USE_SGP30                 /* TVOC / eCO2 @ 0x58                  */
#endif
/* NOT USE_CCS811. Tasmota keeps TWO drivers for this chip in the same
   Xsns slot 31 -- legacy (USE_CCS811) and V2 (USE_CCS811_V2, multiple
   addresses) -- and tasmota32's defaults already carry the V2. Defining
   the legacy name compiled BOTH into slot 31 and the build died on
   redefinition (caught by CI, 2026-08-25, on the runner instead of on
   a board -- the proof steps working as intended). The guard below is
   belt only: V2 is default-on, and must never fall out.              */
#ifndef USE_CCS811_V2
#define USE_CCS811_V2             /* eCO2 / TVOC @ 0x5A-0x5B (WAKE->GND) */
#endif
#ifndef USE_SCD30
#define USE_SCD30                 /* true CO2 @ 0x61                     */
#endif
#ifndef USE_SCD40
#define USE_SCD40                 /* true CO2 @ 0x62                     */
#endif
#ifndef USE_SEN5X
#define USE_SEN5X                 /* PM + VOC @ 0x69 (keep MPU AD0 low)  */
#endif

/* -- light and UV ----------------------------------------------------- */
#ifndef USE_VEML7700
#define USE_VEML7700              /* lux, wide range @ 0x10              */
#endif
#ifndef USE_VEML6070
#define USE_VEML6070              /* UV index @ 0x38+0x39                */
#endif
#ifndef USE_SI1145
#define USE_SI1145                /* visible+IR+UV @ 0x60                */
#endif
#ifndef USE_MAX44009
#define USE_MAX44009              /* GY-49 lux @ 0x4A                    */
#endif
#ifndef USE_BH1750
#define USE_BH1750                /* GY-302 lux @ 0x23 -- default-on,    */
#endif                            /*   pinned so no future default set   */
                                  /*   can quietly drop it               */

/* -- NO OLED define, and the reason is a CI catch (2026-08-25):
   USE_DISPLAY_SSD1306 was defined here, the build went green, and the
   string-proof found NO ssd1306 in the binary -- no driver, no library
   in the dependency graph. In this Tasmota the define is a name with
   nothing behind it: OLEDs are driven by the UNIVERSAL display driver
   from a descriptor file loaded onto the board, which is an integration
   design job (get display.ini to the board's filesystem), not a define.
   Parked with the harvest list; the catalogue offers no OLED entry
   until a board has proven one.                                       */

/* ==== HARVEST-PENDING GPIO DRIVERS -- compiled NOW, catalogued at the
   next bench session. These take GPIO pins, so their catalogue entries
   need component codes READ OFF A BOARD's short `GPIOs` answer (the
   house method -- codes are never guessed). Building the drivers in
   today means ONE paste from the first board flashed with this image
   harvests every code at once: HX711, PMS5003, MH-Z19, SDS011, plus
   the TM1637 and MAX7219 codes still owed from 2026-08-24.          */
#ifndef USE_HX711
#define USE_HX711                 /* load cell -- a scale under anything */
#endif
#ifndef USE_PMS5003
#define USE_PMS5003               /* particulate matter, UART            */
#endif
#ifndef USE_MHZ19
#define USE_MHZ19                 /* CO2, UART (the classic MH-Z19)      */
#endif
#ifndef USE_NOVA_SDS
#define USE_NOVA_SDS              /* SDS011 particulate, UART            */
#endif

/* ==== DELIBERATELY NOT SWEPT -- each for a reason worth keeping ======
   USE_PCF8574   The I/O-expander driver claims 0x20-0x27 -- the SAME
                 chip and addresses the LCD backpack uses. Enabling it
                 would have two drivers fighting over the bedroom LCD.
   USE_DS3231    The RTC lives at 0x68, which is the MPU6050's address.
                 Boards take time from the network here anyway.
   USE_APDS9960  Its own define disables SHT and VEML6070 (address
                 fights); gesture lost the trade against UV + the SHT
                 family.
   USE_TSL2561 / USE_TSL2591   0x39 and 0x29 crowd the VL53L0X and
                 others; four light sensors are already swept.
   USE_MCP230xx / USE_PCA9685  Real expanders, but each needs its own
                 per-pin configuration flow -- an honest entry is a
                 design job, not a define. Parked with the custom door.
*/

#endif
