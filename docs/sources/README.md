# Sources

The manuals, data sheets and libraries the host and the stick are built on, each at the version the code relies on.

- nrf_to_nrf by TMRh20, the radio library of both sketches: https://github.com/TMRh20/nrf_to_nrf at tag v1.7.0, commit d1810ab1d6fb9ef8cedf6487d54fcacf0337ee65 (https://github.com/TMRh20/nrf_to_nrf/tree/d1810ab1d6fb9ef8cedf6487d54fcacf0337ee65). CI installs it as `nrf_to_nrf@1.7.0` from the Arduino Library Manager, which builds its release 1.7.0 from that tag.
- Adafruit nRF52 Arduino core (`adafruit:nrf52`), the board package both sketches build with: https://github.com/adafruit/Adafruit_nRF52_Arduino at release tag 1.7.0, commit ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab (https://github.com/adafruit/Adafruit_nRF52_Arduino/tree/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab; release page https://github.com/adafruit/Adafruit_nRF52_Arduino/releases/tag/1.7.0). CI installs it as `adafruit:nrf52@1.7.0` from Adafruit's board index. Its build step in `platform.txt` runs adafruit-nrfutil, which CI installs with `pip install adafruit-nrfutil` and no version; CI's run of commit 49f064d87c30ab834dcd6793569f9a6c06b49a8e (https://github.com/camshaft/answer-sticks/actions/runs/42420394770) installed adafruit-nrfutil 0.5.3.post16. The files read at that commit:
  - `platform.txt`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/platform.txt
  - `variants/pca10056/variant.h`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/variants/pca10056/variant.h
  - `variants/pca10056/variant.cpp`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/variants/pca10056/variant.cpp
  - `cores/nRF5/main.cpp`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/main.cpp
  - `cores/nRF5/wiring.c`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/wiring.c
  - `cores/nRF5/delay.c`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/delay.c
  - `cores/nRF5/wiring_digital.c`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/wiring_digital.c
  - `cores/nRF5/wiring_analog_nRF52.c`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/wiring_analog_nRF52.c
  - `cores/nRF5/WInterrupts.c`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/WInterrupts.c
  - `cores/nRF5/Tone.cpp`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/Tone.cpp
  - `cores/nRF5/freertos/config/FreeRTOSConfig.h`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/freertos/config/FreeRTOSConfig.h
  - `cores/nRF5/freertos/portable/CMSIS/nrf52/port_cmsis_systick.c`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/cores/nRF5/freertos/portable/CMSIS/nrf52/port_cmsis_systick.c
  - `libraries/Wire/Wire.h`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/libraries/Wire/Wire.h
  - `libraries/Wire/Wire_nRF52.cpp`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/libraries/Wire/Wire_nRF52.cpp
  - `libraries/Bluefruit52Lib/src/bluefruit.cpp`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/libraries/Bluefruit52Lib/src/bluefruit.cpp
  - `libraries/InternalFileSytem/src/InternalFileSystem.cpp` (the core spells the directory `InternalFileSytem`): https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/libraries/InternalFileSytem/src/InternalFileSystem.cpp
  - `libraries/InternalFileSytem/src/flash/flash_nrf5x.c`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/libraries/InternalFileSytem/src/flash/flash_nrf5x.c
  - `libraries/InternalFileSytem/src/flash/flash_nrf5x.h`: https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/ba4ad90d7e90c2ad5d4c98c10539f1f77fafa6ab/libraries/InternalFileSytem/src/flash/flash_nrf5x.h

## Host LCD: the 20x4 module and its controllers

None of these three data sheets lets itself be copied, so each is listed by its title, its publisher, its URL, its revision, and the size and SHA-256 of the file read, so that a later reader can check that the file at the URL is still the one the design cites.

### Newhaven Display NHD-0420H1Z-FL-GBW-33V3

- Title: NHD-0420H1Z-FL-GBW-33V3, Character Liquid Crystal Display Module
- Publisher: Newhaven Display International, Inc.
- URL: https://newhavendisplay.com/content/specs/NHD-0420H1Z-FL-GBW-33V3.pdf
- Revision: 2, dated 7/20/16 in the data sheet's revision history; 12 pages
- Read on 2026-11-07: 448760 bytes, Last-Modified Tue, 08 Feb 2022 19:52:27 +0000
- SHA-256: `e61718d6fc33226052691711599065889744175f92379f2a3bdc1a62cbf347a5`
- Not copied here: its mechanical drawing says it "shall not be copied, reproduced, and/or disclosed in any format without permission".
- What it is: the 4x20 LCD module the host uses, rated for a 3.3 V supply, with the ST7066U controller and a 16-pin header at 2.54 mm pitch.
- Where to buy: Newhaven Display, https://newhavendisplay.com/4x20-character-lcd-stn-gray-display-h1z-with-yellow-green-backlight-3v/ (US$17.10, in stock on 2026-11-07). Its outline is 79 mm x 36 mm x 13 mm.

### Sitronix ST7066U

- Title: ST7066U, Dot Matrix LCD Controller/Driver
- Publisher: Sitronix; the file is Newhaven Display's copy, which the module's data sheet links to
- URL: https://newhavendisplay.com/content/app_notes/ST7066U.pdf
- Revision: V2.2, 2006/05/11; 42 pages
- Read on 2026-11-07: 618877 bytes, Last-Modified Thu, 10 Feb 2022 15:01:22 +0000
- SHA-256: `3038a0579b41ca8567c1e9e254ebbe4ef7b6313df2dfd6e26fe98cf3c858311d`
- Not copied here: the data sheet doesn't give a license to copy it.
- What it is: the controller the module uses, compatible with the HD44780.

### Hitachi HD44780U (LCD-II)

- Title: HD44780U (LCD-II) (Dot Matrix Liquid Crystal Display Controller/Driver)
- Publisher: Hitachi, Ltd.; the file is SparkFun Electronics' copy
- URL: https://cdn.sparkfun.com/assets/9/5/f/7/b/HD44780.pdf (https://www.sparkfun.com/datasheets/LCD/HD44780.pdf redirects there)
- Revision: ADE-207-272(Z), '99.9, Rev. 0.0; 60 pages
- Read on 2026-11-07: 329795 bytes, Last-Modified Wed, 04 Dec 2024 20:18:20 GMT
- SHA-256: `4324d67ec14fc8e7ba8cf7db7256749c9b80b94b3f8bedfa614a0d2839c7dea9`
- Not copied here: its Cautions, item 6, say "No one is permitted to reproduce or duplicate, in any form, the whole or part of this document without written approval from Hitachi."
- What it is: the controller whose instruction set the ST7066U and the hd44780 library follow.

## Host LCD: the I2C backpack and its library

The host drives the LCD through a PCF8574 I2C backpack, with the hd44780 library's `hd44780_I2Cexp` class. None of the five documents below lets itself be copied or states a license, so each is listed by its title, its publisher, its URL, its revision, and the size and SHA-256 of the file read. The library is listed by its tag and commit.

### Texas Instruments PCF8574

- Title: PCF8574 Remote 8-Bit I/O Expander for I2C Bus
- Publisher: Texas Instruments
- URL: https://www.ti.com/lit/ds/symlink/pcf8574.pdf
- Revision: SCPS068K, July 2001, revised September 2024; 39 pages
- Read on 2026-11-07: 2613289 bytes, Last-Modified Thu, 05 Nov 2026 06:34:36 GMT
- SHA-256: `f62ec6e9e3c749de3d5b41509ea00250382b3e666513b04868f5638eaeea4e72`
- Not copied here: its notice says "Other reproduction and display of these resources is prohibited".
- What it is: the I/O expander on the backpack, with its supply range (section 5.3), its I2C clock limit (5.6) and its addresses (7.3.3).

### Texas Instruments PCF8574A

- Title: PCF8574A Remote 8-Bit I/O Expander for I2C Bus
- Publisher: Texas Instruments
- URL: https://www.ti.com/lit/ds/symlink/pcf8574a.pdf
- Revision: SCPS069H, July 2001, revised September 2024; 36 pages
- Read on 2026-11-07: 2546661 bytes, Last-Modified Thu, 05 Nov 2026 06:33:27 GMT
- SHA-256: `d21abac204a76945c122b5a89c089a7f32d248724c0393c5a218f96c999fdb16`
- Not copied here: SCPS068K's notice, which covers TI's resources, prohibits other reproduction of them.
- What it is: the A part, which some backpacks carry in place of the PCF8574, with its supply range (page 4) and its I2C clock limit (page 5).

### NXP PCF8574; PCF8574A

- Title: PCF8574; PCF8574A Remote 8-bit I/O expander for I2C-bus with interrupt
- Publisher: NXP Semiconductors
- URL: https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf
- Revision: Product data sheet Rev. 5, 27 May 2013; 33 pages
- Read on 2026-11-07: 913144 bytes, from the Internet Archive's capture of that URL of 2024-12-29, https://web.archive.org/web/20241229203815id_/https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf, whose original Last-Modified is Wed, 19 Apr 2023 21:55:32 GMT. NXP's URL answered HTTP 404 to a script's read from a GitHub Actions runner.
- SHA-256: `15873fa13e8b9e3baeb924cc5ce845eabf0e1d2671a441feaaa3c4bb56e77013`
- Not copied here: it says "All rights reserved".
- What it is: the data sheet of both parts, with their address maps (Tables 4 and 5), their supply range (Table 9) and their I2C clock limit (Table 10).

### PCF8574 I2C LCD1602 adapter board schematic

- Title: PCF8574 I2C LCD1602 Adapter board, I2C LCD adapter schematic
- Publisher: "Hanako" (the PDF's author field says "Yuki"), posted on mikrocontroller.net as attachment 521754
- URL: https://www.mikrocontroller.net/attachment/521754/PCF8574_I2C_LCD_Cir_En.pdf
- Revision: dated 2021.3.16 in its title block; 1 page, created 2021-03-31
- Read on 2026-11-07: 721344 bytes, from the Internet Archive's capture of that URL of 2024-11-02, https://web.archive.org/web/20241102105358id_/https://www.mikrocontroller.net/attachment/521754/PCF8574_I2C_LCD_Cir_En.pdf. The URL answered HTTP 403 to a script's read.
- SHA-256: `64fb10abd5b00f69a1d73480d83eab2b2a15ca8f862da8c0e45bde4d3a481d85`
- Not copied here: it doesn't state a license.
- What it is: a third party's drawing of the common backpack, since no maker of the backpack publishes one. Most of it is an image; its text names the PCF8574's pins, the contrast adjustment, the LED jumper and the S8050 backlight transistor, and marks two parts "May be 10K" ohms.

### Handsontec I2C Serial Interface 1602 LCD Module

- Title: I2C Serial Interface 1602 LCD Module, User Guide
- Publisher: Handsontec
- URL: https://www.handsontec.com/dataspecs/module/I2C_1602_LCD.pdf
- Revision: SKU DSP-1182; the guide doesn't state a revision; 8 pages, created 2018-08-05
- Read on 2026-11-07: 900672 bytes, Last-Modified Mon, 06 Aug 2018 00:00:49 GMT
- SHA-256: `8045603417887006e39c2240a261ffbaeb175d3f7189359dd253f6c41ca4cda8`
- Not copied here: it doesn't state a license.
- What it is: a seller's guide to the same backpack, with its addresses and supply (page 1), its address pads (page 2) and a pin map for the LiquidCrystal_I2C library (page 4).

### hd44780 by Bill Perry

- Title: hd44780, library.properties version 1.3.2
- Publisher: Bill Perry (duinoWitchery)
- URL: https://github.com/duinoWitchery/hd44780
- Revision: tag 1.3.2 (tag object 8980050e48ab50f222aea8dfe00c68b83002f078), commit 27fd9d3809a92e744eef60b13dfcf4096c91cbe8, https://github.com/duinoWitchery/hd44780/tree/27fd9d3809a92e744eef60b13dfcf4096c91cbe8
- Read on 2026-11-07: CI installs it as `hd44780@1.3.2` from the Arduino Library Manager (`.github/workflows/ci.yml` line 41), whose index lists https://downloads.arduino.cc/libraries/github.com/duinoWitchery/hd44780-1.3.2.zip, 464103 bytes, SHA-256 `c1408b8129a225fa4e2f3e2e3bef709c1f98ca53dadedfcc9b071f083e504f89`. The installed `hd44780ioClass/hd44780_I2Cexp.h` is the file at that commit: 41602 bytes, SHA-256 `2573418c85ca871a4006511c355f2c16388c8fc85afaf110eb8183cd3647372f`.
- Not copied here: CI installs it. It is licensed GPL-3.0.
- What it is: the host's LCD library. Its `hd44780_I2Cexp` class finds the backpack's address and pin map itself.

## Host buzzer: a passive piezo

The host plays its wrong-answer tone on this buzzer. Its data sheet doesn't let itself be copied, so it is listed by its title, its publisher, its URL, its revision, and the size and SHA-256 of the file read.

### TDK PS1240P02BT

- Title: Piezoelectronic buzzers, Without oscillator circuit, Pin terminal type, PS series (PS12, PS14, PS17)
- Publisher: TDK Corporation; the file is Adafruit Industries' copy, which its product 160 links to
- URL: https://cdn-shop.adafruit.com/product-files/160/160_ds.pdf (TDK's own copy, https://product.tdk.com/system/files/dam/doc/product/sw_piezo/sw_piezo/piezo-buzzer/catalog/piezoelectronic_buzzer_ps_en.pdf, answered HTTP 403 on 2026-11-07)
- Revision: 20220222 (piezoelectronic_buzzer_ps_en, February 2022); 11 pages
- Read on 2026-11-07: 431437 bytes
- SHA-256: `1cc2789af2cb03705e09b8b6e49710c4003f4e5f6fbeb7f0873b8a25c686e60e`
- Not copied here: the catalog doesn't give a license to copy it.
- What it is: a piezo buzzer without an oscillator circuit, driven by an outside square wave, 12.2 mm across with its two pins 5 mm apart.
- Where to buy: Adafruit, https://www.adafruit.com/product/160 (US$1.50, in stock on 2026-11-07), under the name "Piezo Buzzer - PS1240". Its listed height of 6.53 mm is the PS1240P02BT's 6.5 mm, not the 3.5 mm of the PS1240P02CT3.

## Chip: Nordic Semiconductor nRF52840

The host and every stick run on the nRF52840. Its product specification doesn't let itself be copied either, so it is listed the same way as the LCD data sheets.

### nRF52840 Product Specification

- Title: nRF52840 Product Specification v1.11
- Publisher: Nordic Semiconductor ASA
- URL: https://docs-be.nordicsemi.com/bundle/ps_nrf52840/attach/nRF52840_PS_v1.11.pdf?_LANG=enus, the PDF that Nordic's documentation site attaches to the nRF52840 product specification
- Revision: v1.11, October 2024, document 4413_417 v1.11 (the footer of each page), the newest entry of its revision history (page 12); 980 pages, created 2024-10-01
- Read on 2026-11-07: 18583008 bytes, from the Internet Archive's capture of that URL of 2025-08-25, https://web.archive.org/web/20250825153625id_/https://docs-be.nordicsemi.com/bundle/ps_nrf52840/attach/nRF52840_PS_v1.11.pdf?_LANG=enus. Nordic's servers answered a script's read with a browser check (docs.nordicsemi.com) or a redirect to their web reader (docs-be.nordicsemi.com). The Archive lists each of its captures of the URL, from 2025-02-07 to 2025-10-10, with the same content digest, SHA-1 `57POWQZB6QHXZOI2RWLGZFE3KOCAVCMO` in base 32, and the file read has that digest. A copy downloaded from Nordic in a browser can be checked against the SHA-256 below; where it matches, this entry can cite Nordic's download instead of the Archive's capture.
- SHA-256: `6e5950cc7e8272d1803279b8cfe3a6ea2c48be46735db3bc3b0a351e3665ecdf`
- Not copied here: its Copyright notice (Legal notices, page 979) says "All rights are reserved. Reproduction in whole or in part is prohibited without the prior written permission of the copyright holder."
- What it is: the data sheet of the chip both sketches run on. The design reads its radio (section 6.20: packet layout, MODECNF0 and radio timing), clocks (5.4), current consumption and System OFF (5.2 and 5.3.3), GPIO and GPIOTE (6.9 and 6.10), SAADC (6.23), TIMER (6.30) and the aQFN73 ball assignments (7.1.1).
