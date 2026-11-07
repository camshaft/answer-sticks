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

## Stick board: the Pro Micro nRF52840 and the nice!nano layout

The stick runs on the Pro Micro nRF52840, which its maker, Nologo, called the SuperMini NRF52840 until 2023-09-28. It follows the pin layout of the nice!nano v2. Its maker doesn't publish a schematic, so the board's facts come from the maker's product page, the nice!nano documents, a community wiki page about the board, a schematic drawn from a board, and the data sheets of its charger and regulator chips. Each file was read on 2026-11-07.

### Copied here

These three files are under the MIT license, so they are copied into this directory byte for byte as read.

- `zmk/nice_nano_nrf52840_zmk_2_0_0.overlay` and `zmk/nice_nano_nrf52840_zmk_1_0_0.overlay`: ZMK's devicetree overlays for the nice!nano v2 and v1, from https://github.com/zmkfirmware/zmk at commit a23aa009d7a07d67fe5609caca7372e16610f238 (2026-03-03), path `app/boards/nicekeyboards/nice_nano/`. License: MIT. Sizes and SHA-256: 484 bytes, `cd3fd2f738bd76290b261ca5c6d5a77105ef8422df4a67ebb1f6d482dbe5e15e` (v2), and 534 bytes, `0ed7cfa62678f9f9cc966b5b2ad09bac4d5b7706bf30fa0dc59e349346faa28f` (v1). What they are: the GPIO that switches VCC, P0.13, with the level that turns VCC on for each board version, and how each version reads its battery voltage.
- `adafruit-nrf52-bootloader/nice_nano/board.h`: the nice!nano board file of the Adafruit nRF52 Bootloader, from https://github.com/adafruit/Adafruit_nRF52_Bootloader at commit 7cf6787810c83029f09866e141be804264d251df (2026-02-02), path `src/boards/nice_nano/board.h`. License: MIT, copyright 2020 Nick Winans. 2399 bytes, SHA-256 `735fa411e8ebc7fd8ad2ad16b10190e3f23415a5397e218da54a76edecc5c4ac`. What it is: the bootloader's settings for the board: the 3.3 V setting of the nRF52840's REG0 output (`UICR_REGOUT0`), the blue LED on P0.15, and the UF2 drive's label NICENANO.

### Listed here

These sources don't state a license to copy them, so each is listed by its title, its publisher, its URL, its revision, and the size and SHA-256 of the file read.

#### Nologo, the Pro Micro nRF52840 product page

- Title: the product page of the ProMicro52840, formerly the SuperMiniNRF52840 (its text is in Chinese)
- Publisher: Shenzhen Yunzao Technology Network Co., Ltd. (Nologo), the name in the page's copyright line
- URL: https://www.nologo.tech/en/product/otherboard/NRF52840.html
- Revision: the page doesn't state one; Last-Modified Fri, 14 Aug 2026 08:17:58 GMT
- Read on 2026-11-07: 86445 bytes
- SHA-256: `0aeb442600f529e247c6b9d0d08544771f20138e49f008a1d3d8a556b5b239b8`
- Not copied here: the page states a copyright of 2014-2026 and doesn't state a license to copy it.
- What it is: the maker's description of the board: its standby current, its charge current and BOOST pads, its battery pins, the VCC switch on P0.13, and how to start its bootloader.

#### Nice Keyboards, the nice!nano documents

- Title: the nice!nano documentation, three pages: its main page, its pinout and schematic page, and its FAQ
- Publisher: Nice Keyboards
- URLs, sizes and SHA-256, each read on 2026-11-07:
  - https://nicekeyboards.com/docs/nice-nano/: 92119 bytes, `8eb687b3941134744b246b6915171d9dfc00ad89c65112bd0746ae39ac0c51a3`
  - https://nicekeyboards.com/docs/nice-nano/pinout-schematic/: 89479 bytes, `277e89b25ed58f512d28b31963e6cc993b99faf61fbef98fc4ca6ae8d745fcc1`
  - https://nicekeyboards.com/docs/nice-nano/faq/: 87484 bytes, `ca21a216b7146630877a00d2c088cebf4fff13c17b441e3575e021b6c6565c5a`
- Revision: the pages don't state one
- Not copied here: the pages don't state a license to copy them.
- What it is: the documents of the board whose layout the Pro Micro nRF52840 follows: its bootloader and SoftDevice versions, its battery and charge current, its VCC switch on P0.13, and its pinout, which the pinout page shows as images.

#### joric/nrfmicro wiki, the Alternatives page

- Title: Alternatives
- Publisher: joric, on the wiki of https://github.com/joric/nrfmicro
- URL: https://github.com/joric/nrfmicro/wiki/Alternatives
- Revision: wiki commit 081efb21d7c7e2a773f328b4a10aada1cec7ade0 (2026-10-08)
- SHA-256 of `Alternatives.md` at that commit: `76315674f546b37c7a34779c14f05c6b25274a58015b151553b326df7f2973ca`
- Not copied here: the wiki doesn't state a license to copy it.
- What it is: a community record of the board's revisions: its charger, its regulator in each batch, the VCC pull-up fix of April 2024, the unpopulated battery divider on P0.24, and reports about its 32.768 kHz crystal.

#### sasodoma/nrf52840-promicro

- Title: nrf52840-promicro, a KiCad schematic drawn from a Pro Micro nRF52840 board
- Publisher: GitHub user sasodoma
- URL: https://github.com/sasodoma/nrf52840-promicro
- Revision: commit fb046164890ad2034fc87daf87fb5a5bcb22dee3 (2025-10-23, "Fix USB pins")
- Files read at that commit:
  - `README.md`: 2557 bytes, SHA-256 `64a2050a1785460824ed9119c27914f26abaf04b9e321980c9d963ea7048c61c`
  - `KiCad/promicro.kicad_sch`: 283914 bytes, SHA-256 `29b73bde86c803030f399c9e9a5c1db004c22a19b63dcf33e20d7323b5f1dc0c`
- Not copied here: the repository doesn't hold a license file.
- What it is: a schematic drawn from a board: its parts (the ME6217 regulator, the TP4054 charger, the AO3401A transistor, the two crystals), the GPIO of each header pin, the VCC switch on P0.13 and the battery divider pads on P0.24.

#### UTD Semiconductor TP4054

- Title: TP4054, Miniature linear battery management chip
- Publisher: UTD Semiconductor (UMW, www.umw-ic.com); the file is Digi-Key's copy
- URL: https://media.digikey.com/pdf/Data%20Sheets/UTD%20Semi%20PDFs/TP4054.pdf
- Revision: the data sheet doesn't state one; its PDF was created on 2022-06-14; 11 pages
- Read on 2026-11-07: 804163 bytes, Last-Modified 2023-01-09
- SHA-256: `2bad6f490f0fbcd875c1e6c5d355da31b3c041156e959b46608ae288144367c2`
- Not copied here: the data sheet doesn't state a license to copy it.
- What it is: the data sheet of the charger chip the drawn schematic shows (U3).

#### TP4054, LaskaKit's copy

- Title: TP4054, Standalone Linear Li-Ion Battery Charger with Thermal Regulation in SOT
- Publisher: the data sheet doesn't name its maker; the file is LaskaKit's copy
- URL: https://www.laskakit.cz/user/related_files/tp4054.pdf
- Revision: the data sheet doesn't state one; its PDF dates are 2007-08-21 and 2009-12-27; 13 pages
- Read on 2026-11-07: 551303 bytes, Last-Modified 2020-11-04
- SHA-256: `77c8fa6868701b44b8c904d04b4d45fbfeec5ce70896dc9497bb92a6c8fb8021`
- Not copied here: the data sheet doesn't state a license to copy it.
- What it is: a second maker's TP4054 data sheet, whose charge current table and sleep current differ from UTD's.

#### Microne ME6217

- Title: ME6217, data sheet V3.0
- Publisher: Nanjing Micro One Electronics Inc. (Microne); the file is datasheet4u's copy
- URL: https://datasheet4u.com/pdf/1334140/ME6217.pdf
- Revision: V3.0; 10 pages
- Read on 2026-11-07: 643324 bytes
- SHA-256: `67e6b15396305e43474d4baa849edda714f49a8b4e6fcfcd1f79f441a49734dc`
- Not copied here: its page 10 says it may not be copied without the maker's permission.
- What it is: the data sheet of the 3.3 V regulator that feeds VCC, the ME6217C33M5G (U2 on the drawn schematic).
