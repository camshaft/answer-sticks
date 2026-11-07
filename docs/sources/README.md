# Sources

The manuals, data sheets and libraries the host and the stick are built on, each at the version the code relies on.

- nrf_to_nrf by TMRh20, the radio library of both sketches: https://github.com/TMRh20/nrf_to_nrf at tag v1.7.0, commit d1810ab1d6fb9ef8cedf6487d54fcacf0337ee65 (https://github.com/TMRh20/nrf_to_nrf/tree/d1810ab1d6fb9ef8cedf6487d54fcacf0337ee65). CI installs it as `nrf_to_nrf@1.7.0` from the Arduino Library Manager, which builds its release 1.7.0 from that tag.

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
