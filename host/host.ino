// The Answer Stick host sketch: the quizmaster's box. It sends a beacon at the start of each
// 26.2 ms frame, receives each stick's PRESS in the stick's own time slot, lets the
// arbitration choose the round's winner, reads the knob and the three buttons, plays the
// wrong-answer tone, and shows the round, the countdown and the three highest scores on the
// LCD. The AnswerSticks library holds the game's rules (arbitration.h, host_logic.h,
// scoreboard.h, knob.h, packets.h); this sketch drives the radio, TIMER3, the buttons, the
// knob, the buzzer and the LCD, and prints each change of state and each packet it ignores on
// the USB serial port. README.md lists the wiring.
//
// Timing: TIMER3 counts at 16 MHz (PRESCALER 0) in 32-bit BITMODE. A PPI channel triggers
// TIMER3's CAPTURE[0] on the radio's END event, so CC[0] holds the END of the host's beacon
// once write() returns; the sketch reads it before startListening(), so that a PRESS's END
// doesn't overwrite it. TIMER3's compare interrupt runs at priority 7, the radio library's
// own, so that neither handler interrupts the other while the radio's queue is read. At the
// end of each stick slot the handler reads the queue and hands each PRESS to the arbitration;
// at the end of the last slot it reads the queue once more, ends the frame and sends the next
// beacon. Each frame's slots count from the END of its beacon, so the next beacon starts
// 26.2 ms after the END of the one before it.

#include <AnswerSticks.h>
#include <arbitration.h>
#include <host_logic.h>
#include <knob.h>
#include <packets.h>
#include <scoreboard.h>

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <nrf_to_nrf.h>

// Pins, by GPIO: on the adafruit:nrf52:pca10056 board, pin n is P0.n and pin 32 + n is P1.n.
const uint8_t kCorrectPin = 22;  // P0.22: the correct button, to ground.
const uint8_t kWrongPin = 24;    // P0.24: the wrong button, to ground.
const uint8_t kCancelPin = 32;   // P1.00: the cancel button, to ground.
const uint8_t kKnobPin = 2;      // P0.02, AIN0: the knob's wiper.
const uint8_t kBuzzerPin = 29;   // P0.29: the piezo buzzer, to ground.
const uint8_t kSdaPin = 17;      // P0.17: the LCD backpack's SDA.
const uint8_t kSclPin = 20;      // P0.20: the LCD backpack's SCL.
const uint8_t kVccPin = 13;      // P0.13: the board's VCC switch; high turns VCC on.
const uint8_t kBlueLedPin = 15;  // P0.15: the board's blue LED, kept off.

// The radio addresses. The stick sketch uses the same two.
const uint64_t kBeaconAddress = 0xC3A5E19B42ULL;
const uint64_t kPressAddress = 0xC3A5E19B9CULL;

// The PPI channel that triggers TIMER3's CAPTURE[0] on the radio's END event.
const uint8_t kPpiChannel = 0;
// TIMER3's interrupt priority: the radio library's own.
const uint8_t kTimerPriority = 7;

const uint32_t kCountsPerUs = answer_sticks::kCountsPerUs;
const uint32_t kStickSlotCounts = answer_sticks::kStickSlotUs * kCountsPerUs;
const uint8_t kLastSlot = answer_sticks::kMaxSticks - 1;
// A deadline nearer than this is waited for in the interrupt, since a compare value of the
// count now or the count after it may not raise the COMPARE event.
const uint32_t kGuardCounts = 2 * kCountsPerUs;

// How long a button's level holds before the sketch counts it.
const uint32_t kDebounceMs = 30;
// The LCD's controller is ready 40 ms after its supply reaches 2.7 V; the sketch starts the
// LCD 50 ms after it turns VCC on.
const uint32_t kLcdStartMs = 50;
const uint8_t kLcdColumns = 20;
const uint8_t kLcdRows = 4;
static_assert(answer_sticks::kLineWidth == kLcdColumns, "the first line fills a row");
static_assert(answer_sticks::kLineChars == kLcdColumns, "a score line fills a row");
static_assert(answer_sticks::kScoreRows + 1 == kLcdRows, "the scores fill rows 2 to 4");

// What TIMER3's handler reports to loop(), which prints it.
enum EventKind : uint8_t {
  kEventBeacon,      // a: round, b: phase, c: winner
  kEventTaken,       // a: stick, b: round, value: age_us
  kEventIgnored,     // a: stick, b: round, c: why (IgnoredWhy), value: age_us
  kEventBadPacket,   // a: length, b: first byte
  kEventWinner,      // a: stick, b: round
  kEventSendFailed,  // a: round
};

// Why the host didn't take a PRESS.
enum IgnoredWhy : uint8_t {
  kIgnoredEarly,    // it came before the host's first beacon
  kIgnoredAnswering,   // the round has its winner
  kIgnoredOtherRound,  // it is a PRESS of another round
  kIgnoredRepeat,      // the host took this stick's PRESS in this round already
};

struct Event {
  uint8_t kind;
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint32_t value;
};

// A button from a GPIO to ground with the internal pull-up, counted once its level holds for
// kDebounceMs.
struct Button {
  uint8_t pin;
  const char* name;
  bool stable_high;     // the level that held for kDebounceMs
  bool last_high;       // the level read last
  uint32_t changed_ms;  // when the level read last changed
};

nrf_to_nrf radio;
hd44780_I2Cexp lcd;

// TIMER3's handler's state. loop() changes g_arbiter only with TIMER3's interrupt held off.
answer_sticks::Arbiter g_arbiter;
bool g_frame_started = false;
uint32_t g_beacon_end = 0;        // the END of the current frame's beacon
uint8_t g_next_slot = kLastSlot;  // the stick slot whose end comes next
uint32_t g_next_at = 0;           // the end of that slot
// The last beacon printed, so that loop() prints each change once.
bool g_logged_any = false;
uint8_t g_logged_round = 0;
uint8_t g_logged_phase = 0;
uint8_t g_logged_winner = 0;

// TIMER3's handler sets these; loop() reads and clears them with TIMER3's interrupt held off.
volatile bool g_first_press = false;  // the round's first PRESS moved it to deciding
volatile bool g_winner_ready = false;  // the arbitration chose g_winner
volatile uint8_t g_winner = answer_sticks::kNoWinner;

// Events from TIMER3's handler to loop(): one writer, one reader. A frame holds up to 64
// PRESS packets.
const uint8_t kEventSlots = 128;
Event g_events[kEventSlots];
volatile uint8_t g_event_head = 0;
volatile uint8_t g_event_tail = 0;

// loop()'s state.
answer_sticks::HostLogic g_host;
answer_sticks::Scoreboard g_scores;
Button g_buttons[3] = {
    {kCorrectPin, "correct", true, true, 0},
    {kWrongPin, "wrong", true, true, 0},
    {kCancelPin, "cancel", true, true, 0},
};
bool g_tone_playing = false;
uint32_t g_tone_started_ms = 0;
bool g_lcd_ok = false;
int g_lcd_status = 0;
// The text each LCD row shows, so that loop() writes a row only when it changes.
char g_shown[kLcdRows][kLcdColumns + 1];
bool g_serial_open = false;

extern "C" void TIMER3_IRQHandler(void);

// TIMER3's count now, read through CAPTURE[2].
static uint32_t timer_now() {
  NRF_TIMER3->TASKS_CAPTURE[2] = 1;
  return NRF_TIMER3->CC[2];
}

static void push_event(uint8_t kind, uint8_t a, uint8_t b, uint8_t c, uint32_t value) {
  const uint8_t head = g_event_head;
  const uint8_t next = static_cast<uint8_t>((head + 1) % kEventSlots);
  if (next == g_event_tail) {
    return;  // loop() hasn't printed the older events yet, so this one isn't kept
  }
  g_events[head].kind = kind;
  g_events[head].a = a;
  g_events[head].b = b;
  g_events[head].c = c;
  g_events[head].value = value;
  __DMB();
  g_event_head = next;
}

static void log_beacon(const answer_sticks::Beacon& beacon) {
  const uint8_t phase = static_cast<uint8_t>(beacon.phase);
  if (g_logged_any && beacon.round == g_logged_round && phase == g_logged_phase &&
      beacon.winner == g_logged_winner) {
    return;
  }
  g_logged_any = true;
  g_logged_round = beacon.round;
  g_logged_phase = phase;
  g_logged_winner = beacon.winner;
  push_event(kEventBeacon, beacon.round, phase, beacon.winner, 0);
}

// Reads each packet in the radio's queue, and hands each PRESS to the arbitration as a PRESS
// of the frame whose beacon's END TIMER3 captured at g_beacon_end.
static void read_presses() {
  while (radio.available()) {
    const uint8_t length = radio.getDynamicPayloadSize();
    uint8_t packet[32] = {0};
    radio.read(packet, sizeof(packet));
    answer_sticks::Press press;
    if (!answer_sticks::decode(packet, length, press)) {
      push_event(kEventBadPacket, length, packet[0], 0, 0);
      continue;
    }
    uint8_t why = kIgnoredEarly;
    if (g_frame_started) {
      const bool was_ready = g_arbiter.beacon().phase == answer_sticks::Phase::kReady;
      if (g_arbiter.take(press, g_beacon_end)) {
        push_event(kEventTaken, press.stick, press.round, 0, press.age_us);
        if (was_ready) {
          g_first_press = true;
        }
        continue;
      }
      const answer_sticks::Beacon& beacon = g_arbiter.beacon();
      if (beacon.phase == answer_sticks::Phase::kAnswering) {
        why = kIgnoredAnswering;
      } else if (press.round != beacon.round) {
        why = kIgnoredOtherRound;
      } else {
        why = kIgnoredRepeat;
      }
    }
    push_event(kEventIgnored, press.stick, press.round, why, press.age_us);
  }
}

// Ends the frame and sends the next beacon: reads the queue a last time, ends the frame in
// the arbitration, which chooses the winner at the end of the second frame of deciding, sends
// the beacon, and reads its END from CAPTURE[0] before startListening().
static void start_frame() {
  read_presses();
  if (g_arbiter.end_frame()) {
    g_winner = g_arbiter.beacon().winner;
    g_winner_ready = true;
    push_event(kEventWinner, g_arbiter.beacon().winner, g_arbiter.beacon().round, 0, 0);
  }
  const answer_sticks::Beacon& beacon = g_arbiter.beacon();
  uint8_t packet[answer_sticks::kBeaconSize];
  const size_t length = answer_sticks::encode(beacon, packet, sizeof(packet));
  radio.stopListening();
  const bool sent = radio.write(packet, static_cast<uint8_t>(length), true);
  uint32_t end = NRF_TIMER3->CC[0];
  if (!sent) {
    // The frame counts from now where the radio didn't report the beacon's END.
    end = timer_now();
    push_event(kEventSendFailed, beacon.round, 0, 0, 0);
  }
  radio.startListening();
  g_beacon_end = end;
  g_frame_started = true;
  g_next_slot = 0;
  g_next_at = answer_sticks::slot_start(end, 0) + kStickSlotCounts;
  log_beacon(beacon);
}

// Runs each slot end that is due, then sets CC[1] to the next one.
static void run_schedule() {
  for (;;) {
    const uint32_t now = timer_now();
    if (!answer_sticks::before(now, g_next_at)) {
      if (g_next_slot >= kLastSlot) {
        start_frame();
      } else {
        read_presses();
        ++g_next_slot;
        g_next_at += kStickSlotCounts;
      }
      continue;
    }
    if (!answer_sticks::before(now + kGuardCounts, g_next_at)) {
      while (answer_sticks::before(timer_now(), g_next_at)) {
      }
      continue;
    }
    NRF_TIMER3->CC[1] = g_next_at;
    if (answer_sticks::before(timer_now() + kGuardCounts, g_next_at)) {
      return;
    }
  }
}

extern "C" void TIMER3_IRQHandler(void) {
  NRF_TIMER3->EVENTS_COMPARE[1] = 0;
  (void)NRF_TIMER3->EVENTS_COMPARE[1];  // the read makes the write take effect first
  run_schedule();
}

static const char* phase_name(uint8_t phase) {
  switch (phase) {
    case 0:
      return "ready";
    case 1:
      return "deciding";
    case 2:
      return "answering";
  }
  return "unknown";
}

static const char* ignored_why(uint8_t why) {
  switch (why) {
    case kIgnoredEarly:
      return "it came before the host's first beacon";
    case kIgnoredAnswering:
      return "the round has its winner";
    case kIgnoredOtherRound:
      return "it is a PRESS of another round";
    case kIgnoredRepeat:
      return "the host took this stick's PRESS in this round already";
  }
  return "unknown";
}

static void print_event(const Event& event) {
  switch (event.kind) {
    case kEventBeacon:
      Serial.printf("Beacon: round %u, %s, winner ", event.a, phase_name(event.b));
      if (event.c == answer_sticks::kNoWinner) {
        Serial.println("none");
      } else {
        Serial.println(event.c);
      }
      break;
    case kEventTaken:
      Serial.printf("PRESS from stick %u, round %u, pressed %lu us before its slot: taken\n",
                    event.a, event.b, static_cast<unsigned long>(event.value));
      break;
    case kEventIgnored:
      Serial.printf("Ignored a PRESS from stick %u, round %u: %s\n", event.a, event.b,
                    ignored_why(event.c));
      break;
    case kEventBadPacket:
      Serial.printf("Ignored a packet: %u bytes, first byte %u\n", event.a, event.b);
      break;
    case kEventWinner:
      Serial.printf("Round %u: stick %u pressed first\n", event.b, event.a);
      break;
    case kEventSendFailed:
      Serial.printf("Round %u: write() didn't finish the beacon\n", event.a);
      break;
  }
}

static void print_status() {
  Serial.printf("Answer Stick host, round %u, %s.\n", g_host.round(),
                phase_name(static_cast<uint8_t>(g_host.phase())));
  if (!g_lcd_ok) {
    Serial.printf("The LCD didn't start: hd44780's begin() answered %d.\n", g_lcd_status);
  }
}

// Prints the events TIMER3's handler reported, where the serial monitor is open.
static void print_events() {
  const bool open = Serial;
  if (open && !g_serial_open) {
    print_status();
  }
  g_serial_open = open;
  while (g_event_tail != g_event_head) {
    __DMB();
    const Event event = g_events[g_event_tail];
    g_event_tail = static_cast<uint8_t>((g_event_tail + 1) % kEventSlots);
    if (open) {
      print_event(event);
    }
  }
}

// Reads the news of TIMER3's handler with its interrupt held off, so that a flag and the
// winner are read together, and hands it to the host's state.
static void take_news() {
  NVIC_DisableIRQ(TIMER3_IRQn);
  const bool first = g_first_press;
  const bool chosen = g_winner_ready;
  const uint8_t winner = g_winner;
  g_first_press = false;
  g_winner_ready = false;
  NVIC_EnableIRQ(TIMER3_IRQn);
  if (first) {
    g_host.first_press();
    if (Serial) {
      Serial.printf("Round %u: deciding\n", g_host.round());
    }
  }
  if (chosen) {
    // The knob's value at the moment the winner was chosen sets the answer time.
    const uint32_t reading = analogRead(kKnobPin);
    const uint8_t seconds = answer_sticks::knob_seconds(reading);
    g_host.winner_chosen(winner, seconds, millis());
    if (Serial) {
      Serial.printf("Round %u: stick %u answers; the knob reads %lu, %u s\n", g_host.round(),
                    winner, static_cast<unsigned long>(reading), seconds);
    }
  }
}

static void start_tone() {
  tone(kBuzzerPin, answer_sticks::kWrongToneHz, answer_sticks::kWrongToneMs);
  g_tone_playing = true;
  g_tone_started_ms = millis();
}

// Once the tone's time is over, the buzzer's pin rests low, since the buzzer mustn't have a
// DC bias.
static void end_tone() {
  if (g_tone_playing && millis() - g_tone_started_ms >= answer_sticks::kWrongToneMs) {
    noTone(kBuzzerPin);
    digitalWrite(kBuzzerPin, LOW);
    pinMode(kBuzzerPin, OUTPUT);
    g_tone_playing = false;
  }
}

// Acts on an input that ended the round: adds the winner's point, plays the tone, and opens
// the next round in the arbitration, whose next beacon carries it.
static void finish_round(const answer_sticks::Effects& effects, uint8_t round,
                         const char* why) {
  if (effects.scored != answer_sticks::kNoWinner) {
    g_scores.add_point(effects.scored);
  }
  if (effects.wrong_tone) {
    start_tone();
  }
  NVIC_DisableIRQ(TIMER3_IRQn);
  g_arbiter.open_next();
  g_first_press = false;
  g_winner_ready = false;
  NVIC_EnableIRQ(TIMER3_IRQn);
  if (Serial) {
    Serial.printf("Round %u ended: %s", round, why);
    if (effects.scored != answer_sticks::kNoWinner) {
      Serial.printf("; stick %u has %u points", effects.scored,
                    g_scores.score(effects.scored));
    }
    Serial.printf(". Round %u is ready.\n", g_host.round());
  }
}

// Whether the button was pressed: its level went low and held for kDebounceMs.
static bool pressed(Button& button, uint32_t now_ms) {
  const bool high = digitalRead(button.pin) == HIGH;
  if (high != button.last_high) {
    button.last_high = high;
    button.changed_ms = now_ms;
    return false;
  }
  if (high == button.stable_high || now_ms - button.changed_ms < kDebounceMs) {
    return false;
  }
  button.stable_high = high;
  return !high;
}

static void read_buttons() {
  const uint32_t now_ms = millis();
  for (uint8_t i = 0; i < 3; ++i) {
    if (!pressed(g_buttons[i], now_ms)) {
      continue;
    }
    const uint8_t round = g_host.round();
    answer_sticks::Effects effects;
    if (g_buttons[i].pin == kCorrectPin) {
      effects = g_host.correct();
    } else if (g_buttons[i].pin == kWrongPin) {
      effects = g_host.wrong();
    } else {
      effects = g_host.cancel();
    }
    if (effects.round_ended) {
      finish_round(effects, round, g_buttons[i].name);
    } else if (Serial) {
      Serial.printf("Ignored the %s button: round %u is in %s\n", g_buttons[i].name, round,
                    phase_name(static_cast<uint8_t>(g_host.phase())));
    }
  }
}

static void check_timer() {
  const uint8_t round = g_host.round();
  const answer_sticks::Effects effects = g_host.tick(millis());
  if (effects.round_ended) {
    finish_round(effects, round, "time is up");
  }
}

static void show_line(uint8_t row, const char* line) {
  if (strcmp(g_shown[row], line) == 0) {
    return;
  }
  lcd.setCursor(0, row);
  lcd.print(line);
  strncpy(g_shown[row], line, kLcdColumns);
  g_shown[row][kLcdColumns] = '\0';
}

// Writes each LCD row whose text changed: the round or the countdown on row 1, and the three
// highest scores on rows 2 to 4.
static void update_lcd() {
  if (!g_lcd_ok) {
    return;
  }
  char line[answer_sticks::kLineWidth + 1];
  const size_t length = g_host.first_line(millis(), line);
  for (size_t i = length; i < answer_sticks::kLineWidth; ++i) {
    line[i] = ' ';
  }
  line[answer_sticks::kLineWidth] = '\0';
  show_line(0, line);
  for (uint8_t row = 0; row < answer_sticks::kScoreRows; ++row) {
    char score_line[answer_sticks::kLineChars + 1];
    g_scores.format_row(row, score_line);
    show_line(static_cast<uint8_t>(row + 1), score_line);
  }
}

void setup() {
  // The core's board setup drives P0.13 to P0.16 high before setup().
  digitalWrite(kVccPin, HIGH);
  pinMode(kVccPin, OUTPUT);
  const uint32_t vcc_on_ms = millis();
  digitalWrite(kBlueLedPin, LOW);
  pinMode(kBlueLedPin, OUTPUT);
  digitalWrite(kBuzzerPin, LOW);
  pinMode(kBuzzerPin, OUTPUT);
  for (uint8_t i = 0; i < 3; ++i) {
    pinMode(g_buttons[i].pin, INPUT_PULLUP);
  }
  // The knob reads 0 to VDD: VDD/4 as the reference, with a gain of 1/4.
  analogReference(AR_VDD4);

  Serial.begin(115200);

  // The radio: begin() starts HFXO and sets channel 76 and a 16-bit CRC.
  radio.begin();
  radio.setDataRate(NRF_2MBPS);
  radio.enableDynamicPayloads();
  radio.setAutoAck(false);
  radio.openWritingPipe(kBeaconAddress);
  radio.openReadingPipe(0, kPressAddress);
  radio.startListening();

  // TIMER3: 16 MHz, 32 bits, running from now on.
  NRF_TIMER3->TASKS_STOP = 1;
  NRF_TIMER3->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
  NRF_TIMER3->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
  NRF_TIMER3->PRESCALER = 0;
  NRF_TIMER3->SHORTS = 0;
  NRF_TIMER3->INTENCLR = 0xFFFFFFFFUL;
  NRF_TIMER3->EVENTS_COMPARE[1] = 0;
  NRF_TIMER3->INTENSET = TIMER_INTENSET_COMPARE1_Msk;
  NRF_TIMER3->TASKS_CLEAR = 1;

  // The radio's END event triggers TIMER3's CAPTURE[0].
  NRF_PPI->CH[kPpiChannel].EEP = reinterpret_cast<uint32_t>(&NRF_RADIO->EVENTS_END);
  NRF_PPI->CH[kPpiChannel].TEP = reinterpret_cast<uint32_t>(&NRF_TIMER3->TASKS_CAPTURE[0]);
  NRF_PPI->CHENSET = 1UL << kPpiChannel;

  // The first pass of TIMER3's handler ends the last slot of a frame and sends the first
  // beacon.
  NRF_TIMER3->TASKS_START = 1;
  g_next_slot = kLastSlot;
  g_next_at = timer_now();
  NVIC_ClearPendingIRQ(TIMER3_IRQn);
  NVIC_SetPriority(TIMER3_IRQn, kTimerPriority);
  NVIC_EnableIRQ(TIMER3_IRQn);
  NVIC_SetPendingIRQ(TIMER3_IRQn);

  // The LCD, once its controller is ready. setPins() only stores the pins; hd44780 calls
  // Wire.begin() itself.
  while (millis() - vcc_on_ms < kLcdStartMs) {
    delay(1);
  }
  Wire.setPins(kSdaPin, kSclPin);
  g_lcd_status = lcd.begin(kLcdColumns, kLcdRows);
  g_lcd_ok = g_lcd_status == 0;
}

void loop() {
  print_events();
  take_news();
  read_buttons();
  check_timer();
  end_tone();
  update_lcd();
  delay(1);
}
