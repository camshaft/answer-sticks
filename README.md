# Answer Sticks

A quiz game for up to 64 players. Each player holds a stick with one button and one light; the
quizmaster has a host box with an LCD, a knob and three buttons. The first stick pressed in a
round lights up, the host shows its number and counts down the answer time, and the quizmaster
presses correct, wrong or cancel. Correct adds a point to that stick, and the LCD shows the
three highest scores.

Every board is a Pro Micro nRF52840 (a nice!nano v2 clone) with a lipo battery. The sticks and
the host talk over the nRF52840's own 2.4 GHz radio, without Bluetooth: the host sends a beacon
every 26.2 ms, and each stick sends its press in a time slot of its own, with how long ago the
button was pressed, so the host can tell which press came first.

Software versions, as CI builds them:

| What | Version |
| --- | --- |
| Adafruit nRF52 core (`adafruit:nrf52`) | 1.7.0 |
| nrf_to_nrf (TMRh20) | 1.7.0 |
| hd44780 (Bill Perry), host only | 1.3.2 |
| adafruit-nrfutil (Python), used by the core's build and upload | 0.5.3.post16 |
| arduino-cli | 1.5.1 |

## What is in this repository

| Folder | What it holds |
| --- | --- |
| `host/` | The host sketch: the beacon, the arbitration of presses, the buttons, the knob, the buzzer and the LCD. |
| `stick/` | The stick sketch: the button, the light, the radio slot and the sleep between rounds. |
| `libraries/AnswerSticks/` | The game's rules as plain C++ with no hardware calls: the packets, the time slots and the choice of the first press, the host's states, the scoreboard and the knob. Both sketches include it. |
| `test/` | Unit tests of the library, built with the computer's compiler. |
| `docs/sources/` | The data sheets and other sources the design reads, or where to find each one. |

## Installing the tools

With arduino-cli:

```sh
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
arduino-cli core update-index
arduino-cli core install adafruit:nrf52@1.7.0
arduino-cli lib install nrf_to_nrf@1.7.0 hd44780@1.3.2
pip install adafruit-nrfutil==0.5.3.post16
```

On Linux the core runs `adafruit-nrfutil` from the PATH, so install it where your shell finds
it.

With the Arduino IDE: add the same board index URL under Preferences, install "Adafruit nRF52"
version 1.7.0 in the Boards Manager, and install nrf_to_nrf 1.7.0 and hd44780 1.3.2 in the
Library Manager. Copy `libraries/AnswerSticks/` into the `libraries` folder of your sketchbook.

Both sketches build for the board "Nordic nRF52840DK (PCA10056)", `adafruit:nrf52:pca10056`. On
that board, Arduino pin n is the chip's GPIO P0.n and pin 32 + n is P1.n, so the sketches and
the tables below give pins as GPIOs.

## Building and uploading

Plug the board into USB and double-tap its reset button (short RST to GND twice within half a
second). The board starts its bootloader: a drive appears and a serial port appears. The drive's
`INFO_UF2.TXT` names the bootloader's version; the core's upload expects the Adafruit nRF52
Bootloader 0.6.0 with the S140 SoftDevice 6.1.1, as on the nice!nano.

From the repository's root, with the bootloader's serial port as PORT (such as `/dev/ttyACM0`
on Linux or `COM5` on Windows):

```sh
arduino-cli compile --upload -p PORT --fqbn adafruit:nrf52:pca10056 --library libraries/AnswerSticks host
arduino-cli compile --upload -p PORT --fqbn adafruit:nrf52:pca10056 --library libraries/AnswerSticks stick
```

The first line uploads the host sketch to the host's board, the second the stick sketch to a
stick. `arduino-cli board list` lists the ports.

## Wiring

Before you wire a board, check two things on it:

1. The pads R10 and R11 of the battery divider on P0.24 are empty, as on most boards. A fitted
   divider loads the host's wrong button and the stick's light.
2. Each pin in the tables below is where the board's printed pinout puts it. The header pin
   numbers come from a schematic of the board (J2 is one row of header pins, J3 the other).

The sketches drive the board's own pins at startup: P0.15, the blue LED, goes low on both; P0.13,
the switch of the board's VCC output, goes high on the host, which powers the LCD, the backpack
and the knob from VCC, and low on the stick, which doesn't use VCC.

### Host

| Part | Wiring | GPIO | Header pin |
| --- | --- | --- | --- |
| LCD backpack SDA | to the backpack's SDA | P0.17 | J2 pin 6 |
| LCD backpack SCL | to the backpack's SCL | P0.20 | J2 pin 7 |
| Correct button | from the pin to GND | P0.22 | J2 pin 8 |
| Wrong button | from the pin to GND | P0.24 | J2 pin 9 |
| Cancel button | from the pin to GND | P1.00 | J2 pin 10 |
| Knob (potentiometer) | ends to VCC and GND, wiper to the pin | P0.02 | J3 pin 6 |
| Piezo buzzer | from the pin to GND | P0.29 | J3 pin 7 |
| LCD backpack VCC and GND | to the board's VCC (3.3 V) and GND | | |
| Lipo battery | to the board's B+ and B- | | |

The parts:

- LCD: a 4x20 character LCD rated for a 3.3 V supply, the Newhaven NHD-0420H1Z-FL-GBW-33V3. A
  module rated for 5 V shows faint characters at 3.3 V. Its backlight takes 3.0 V and 150 mA
  typical, 3.2 V and 180 mA at most, so it doesn't go straight to VCC: fit a resistor of about
  2 ohm in place of the backpack's LED jumper. The backpack's contrast potentiometer holds the
  LCD's contrast pin near ground.
- Backpack: a PCF8574 or PCF8574A I2C backpack, powered from VCC. The sketch finds its address,
  0x20 to 0x27 or 0x38 to 0x3F, by itself. The bus runs at 100 kHz.
- Buttons: plain push buttons; the sketch turns on each pin's pull-up and debounces each press
  for 30 ms.
- Knob: a potentiometer between VCC and GND. One end sets 3 s to answer, the other 60 s.
- Buzzer: a TDK PS1240P02BT piezo buzzer, without its own oscillator. The sketch plays 4 kHz,
  its rated frequency, for 800 ms, and keeps the pin low between tones.
- Battery: a lipo cell, charged over USB at 100 mA by the board's charger.

### Stick

| Part | Wiring | GPIO | Header pin |
| --- | --- | --- | --- |
| Button | from the pin to GND | P0.22 | J2 pin 8 |
| LED | from the pin through a 330 ohm resistor to GND | P0.24 | J2 pin 9 |
| Lipo battery | to the board's B+ and B- | | |

The sketch sets the LED's pin to high drive, which guarantees 6 mA at a 0.4 V drop.

## Setting a stick's number

Each stick needs a number from 0 to 63, and each stick in a game needs a different one: two
sticks with the same number share one slot and play as one. The number is kept in the board's
flash.

1. Plug the stick into a computer's USB.
2. Open a serial monitor on its port, such as `arduino-cli monitor -p PORT`, or the Arduino
   IDE's Serial Monitor with "Newline" line endings.
3. Type `id 17` and Enter to set the number to 17. Type `id` to see the number.

The new number counts from the stick's next press. A stick with no number in its flash uses the
build flag `STICK_ID`, 0 unless the build sets it:

```sh
arduino-cli compile --upload -p PORT --fqbn adafruit:nrf52:pca10056 --library libraries/AnswerSticks --build-property "compiler.cpp.extra_flags=-DSTICK_ID=17" stick
```

## Playing

1. Switch on the host. The LCD shows `Press to answer #0`: the round's number. The host starts
   in round 0, and no stick has a score.
2. A player presses a stick's button. The stick wakes, sends its press, and the host waits for
   the end of the next frame for any earlier press from the other sticks. Then it chooses the
   press made first, ties going to the lower stick number.
3. The winner's light turns on, and the LCD shows `Stick 17: 45 s left`, counting down from the
   time the knob set at that moment.
4. The quizmaster presses correct, which adds a point to the winner, or wrong, or cancel. When
   the countdown reaches 0, the buzzer sounds and the round ends. Each of these opens the next
   round, and the winner's light turns off.
5. The buttons do nothing until a round has a winner.

Rows 2 to 4 of the LCD show the three highest scores, as `1. Stick 4    12`. Scores are kept
until the host is switched off.

A stick sleeps between rounds: once the round is over for it, it sleeps when its button is
released, and the next press wakes it. A stick that is waiting for the host's choice turns its
light off and sleeps after 5 frames in a row without a beacon, as when the host is switched off.

## Serial output

Each sketch prints each change of state and each packet it ignores on its USB serial port, at
any baud rate, so you can watch a game from a computer: the host prints each beacon's round,
phase and winner, each press it takes or ignores and why, the knob's reading and each button;
a stick prints the beacons it receives, the beacon period it measures, each press it sends,
its light, and how long the crystal oscillator took to start after the button's edge.

## Checks for the first hardware run

These values come from data sheets or estimates, and only a run on real boards confirms them.
On the first build, check each one and note what you measured:

- The bootloader's version in `INFO_UF2.TXT` is 0.6.0 with S140 6.1.1.
- Each pin matches the board's printed pinout, and R10 and R11 are empty.
- The knob's end at VCC reads 1023 (the host prints the reading when a winner is chosen).
- The LCD backlight draws less than 180 mA.
- The buzzer is loud enough, and the pin's current while it plays.
- P0.14 and P0.16, which the core drives high at startup, don't draw current on the board.
- The stick's timing budget: the radio's transmitter ramp-up at 2 Mbit (140 us assumed), the
  time from the slot's start to the send (about 140 us assumed), and the time from the button's
  edge to the start of the stick's timer, which sets each press's age.
- The crystal oscillator's start time after a wake, which the stick prints.
- The backpack's own pull-up resistors on SDA and SCL, whose value the sources don't state.
- A stick's current while it sleeps, about 17.37 uA for the chip plus the board's own draw.
