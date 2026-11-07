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
