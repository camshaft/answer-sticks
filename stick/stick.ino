// The Answer Stick sketch: one contestant's button and light. The stick sleeps until its
// button is pressed, then receives the host's beacons, sends its PRESS in its own time
// slot, lights its LED where a beacon names it the winner, and sleeps again once the round
// is over for it. The AnswerSticks library holds the game's rules (stick_logic.h,
// arbitration.h, packets.h); this sketch drives the radio, TIMER3, the button, the LED and
// the sleep, and prints each change of state on the USB serial port. README.md lists the
// wiring and how to set the stick's number.
//
// Timing: TIMER3 counts at 16 MHz (PRESCALER 0) in 32-bit BITMODE. A PPI channel triggers
// TIMER3's CAPTURE[0] on the radio's END event, so CC[0] holds the end of the last packet
// the radio received. The stick's slot starts 600 + 400 x stick microseconds after the END
// of the beacon it received. The button's handler clears and starts TIMER3 at the button's
// edge, so TIMER3's count at the start of the slot, divided by 16, is the PRESS's age_us.

#include <AnswerSticks.h>
#include <arbitration.h>
#include <packets.h>
#include <stick_logic.h>

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <nrf_to_nrf.h>

using namespace Adafruit_LittleFS_Namespace;

// The stick's number where its flash doesn't hold one: 0 unless the build sets STICK_ID, as
// --build-property "compiler.cpp.extra_flags=-DSTICK_ID=17" does.
#ifndef STICK_ID
#define STICK_ID 0
#endif
static_assert(STICK_ID >= 0 && STICK_ID < answer_sticks::kMaxSticks,
              "STICK_ID is a stick number from 0 to 63");

// Pins, by GPIO: on the adafruit:nrf52:pca10056 board, pin n is P0.n.
const uint8_t kButtonPin = 22;   // P0.22: the answer button, to ground.
const uint8_t kLightPin = 24;    // P0.24: the LED, through 330 ohm to ground.
const uint8_t kVccPin = 13;      // P0.13: the board's VCC switch; low turns VCC off.
const uint8_t kBlueLedPin = 15;  // P0.15: the board's blue LED, kept off.

// The radio addresses. The host sketch uses the same two.
const uint64_t kBeaconAddress = 0xC3A5E19B42ULL;
const uint64_t kPressAddress = 0xC3A5E19B9CULL;

// The PPI channel that triggers TIMER3's CAPTURE[0] on the radio's END event.
const uint8_t kPpiChannel = 0;
// TIMER3's interrupt priority.
const uint8_t kTimerPriority = 2;

const uint32_t kCountsPerUs = answer_sticks::kCountsPerUs;
const uint32_t kFrameCounts = answer_sticks::kFrameUs * kCountsPerUs;
// The host starts each beacon a frame after the END of the beacon before it, so the time
// from one beacon's END to the next is a frame and the host's send time. The stick measures
// that period, and takes a measurement of up to 1 ms more than a frame.
const uint32_t kPeriodSpreadCounts = 1000 * kCountsPerUs;
// The period the stick expects before it has measured one.
const uint32_t kGuessPeriodCounts = kFrameCounts + 300 * kCountsPerUs;
// Once it has received two beacons in a row, the stick turns its receiver on 500 us before
// the next beacon's expected END, and counts the beacon as missed 300 us after it.
const uint32_t kOpenBeforeCounts = 500 * kCountsPerUs;
const uint32_t kCloseAfterCounts = 300 * kCountsPerUs;
// Until then, its receiver stays on, and a beacon counts as missed 2 ms after its expected
// END.
const uint32_t kSearchGraceCounts = 2000 * kCountsPerUs;
// How often the stick reads the radio's queue while its receiver is on.
const uint32_t kPollCounts = 100 * kCountsPerUs;
// A deadline nearer than this is waited for in the interrupt, since a compare value of the
// count now or the count after it may not raise the COMPARE event.
const uint32_t kGuardCounts = 2 * kCountsPerUs;
// How long the button stays released before the stick sleeps.
const uint32_t kReleaseMs = 30;
// The longest wait for the 32 MHz crystal oscillator (HFXO) after a wake.
const uint32_t kHfxoWaitMs = 10;

// The file in the board's flash that holds the stick's number, as decimal text.
const char kStickFile[] = "/stick_number";

// What TIMER3's handler reports to loop(), which prints it.
enum EventKind : uint8_t {
  kEventBeacon,      // a: round, b: phase, c: winner, value: 1 where this stick's bit is set
  kEventPeriod,      // value: the beacon period measured, in us
  kEventSent,        // a: round, b: 1 where write() finished, value: age_us
  kEventSlotPassed,  // a: round
  kEventMissed,      // a: frames in a row without a beacon
  kEventLight,       // a: 1 for on, 0 for off
  kEventSleep,
  kEventBadPacket,   // a: length, b: first byte
};

struct Event {
  uint8_t kind;
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint32_t value;
};

nrf_to_nrf radio;
uint8_t g_stick = STICK_ID;
answer_sticks::StickLogic g_logic(STICK_ID);

// Shared by the button's handler, TIMER3's handler and loop().
volatile bool g_asleep = false;         // asleep: the next press wakes the stick
volatile bool g_wake_pending = false;   // the button's handler woke the stick
volatile bool g_awake = false;          // TIMER3's handler runs the stick's schedule
volatile bool g_sleep_pending = false;  // TIMER3's handler asked loop() to sleep

// TIMER3's handler's state while the stick is awake. wake() sets it before g_awake.
bool g_receiving = false;
bool g_locked = false;  // two beacons in a row: the receiver opens around each beacon alone
bool g_period_known = false;
uint32_t g_period = kGuessPeriodCounts;
bool g_have_last = false;  // the last frame's beacon was received
uint32_t g_last_end = 0;
uint32_t g_expected = 0;  // the next beacon's expected END
uint32_t g_miss_at = 0;
bool g_open_due = false;
uint32_t g_open_at = 0;
bool g_send_due = false;
uint32_t g_send_at = 0;
answer_sticks::Press g_press;
uint32_t g_check_at = 0;
uint8_t g_missed = 0;
bool g_light_on = false;
// A microsecond clock for StickLogic, kept from TIMER3's counts.
uint32_t g_clock_count = 0;
uint32_t g_clock_us = 0;
// The last beacon printed, so that loop() prints each change once.
bool g_logged_any = false;
uint8_t g_logged_round = 0;
uint8_t g_logged_phase = 0;
uint8_t g_logged_winner = 0;
bool g_logged_heard = false;

// Events from TIMER3's handler to loop(): one writer, one reader.
const uint8_t kEventSlots = 32;
Event g_events[kEventSlots];
volatile uint8_t g_event_head = 0;
volatile uint8_t g_event_tail = 0;

// loop()'s serial state.
char g_command[16];
uint8_t g_command_length = 0;
bool g_serial_open = false;

extern "C" void TIMER3_IRQHandler(void);

// TIMER3's count now, read through CAPTURE[2]. loop() reads it through CAPTURE[3].
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

// The microsecond clock at count. It moves forward only, and a count before the last one
// read returns the clock as it stands. TIMER3's handler reads it at least once a frame.
static uint32_t clock_us(uint32_t count) {
  if (answer_sticks::before(g_clock_count, count)) {
    const uint32_t us = (count - g_clock_count) / kCountsPerUs;
    g_clock_us += us;
    g_clock_count += us * kCountsPerUs;
  }
  return g_clock_us;
}

static void set_light(bool on) {
  if (on) {
    NRF_P0->OUTSET = 1UL << kLightPin;
  } else {
    NRF_P0->OUTCLR = 1UL << kLightPin;
  }
}

static void receiver_on() {
  if (!g_receiving) {
    radio.startListening();
    g_receiving = true;
  }
  g_check_at = timer_now() + kPollCounts;
}

static void receiver_off() {
  if (g_receiving) {
    NRF_RADIO->TASKS_DISABLE = 1;
    g_receiving = false;
  }
}

// StickLogic's sleep: stops the schedule and asks loop() to sleep once the button is
// released.
static void go_to_sleep() {
  g_awake = false;
  g_send_due = false;
  g_open_due = false;
  receiver_off();
  if (g_light_on) {
    g_light_on = false;
    set_light(false);
    push_event(kEventLight, 0, 0, 0, 0);
  }
  push_event(kEventSleep, 0, 0, 0, 0);
  g_sleep_pending = true;
}

static void apply(const answer_sticks::StickAction& action) {
  if (action.sleep) {
    go_to_sleep();
    return;
  }
  if (action.light != g_light_on) {
    g_light_on = action.light;
    set_light(action.light);
    push_event(kEventLight, action.light ? 1 : 0, 0, 0, 0);
  }
}

// Sets when the receiver opens and when the next beacon counts as missed.
static void schedule_next_beacon() {
  if (g_locked) {
    receiver_off();
    g_open_at = g_expected - kOpenBeforeCounts;
    g_open_due = true;
    g_miss_at = g_expected + kCloseAfterCounts;
  } else {
    receiver_on();
    g_open_due = false;
    g_miss_at = g_expected + kSearchGraceCounts;
  }
}

// A beacon whose END TIMER3 captured at end.
static void on_beacon(const answer_sticks::Beacon& beacon, uint32_t end) {
  if (g_have_last) {
    const uint32_t measured = end - g_last_end;
    if (measured >= kFrameCounts && measured <= kFrameCounts + kPeriodSpreadCounts) {
      // The shortest period: a beacon the host sent late makes one period longer.
      if (!g_period_known || measured < g_period) {
        g_period = measured;
        push_event(kEventPeriod, 0, 0, 0, measured / kCountsPerUs);
      }
      g_period_known = true;
      g_locked = true;
    }
  }
  g_have_last = true;
  g_last_end = end;
  g_missed = 0;
  g_expected = end + g_period;

  const uint8_t phase = static_cast<uint8_t>(beacon.phase);
  const bool heard = answer_sticks::is_heard(beacon, g_logic.stick());
  if (!g_logged_any || beacon.round != g_logged_round || phase != g_logged_phase ||
      beacon.winner != g_logged_winner || heard != g_logged_heard) {
    g_logged_any = true;
    g_logged_round = beacon.round;
    g_logged_phase = phase;
    g_logged_winner = beacon.winner;
    g_logged_heard = heard;
    push_event(kEventBeacon, beacon.round, phase, beacon.winner, heard ? 1 : 0);
  }

  const answer_sticks::StickAction action = g_logic.on_beacon(beacon, clock_us(end));
  if (action.send_press) {
    const uint32_t at = answer_sticks::slot_start(end, g_logic.stick());
    if (answer_sticks::before(timer_now() + kGuardCounts, at)) {
      g_press = g_logic.press(at);
      g_send_at = at;
      g_send_due = true;
    } else {
      push_event(kEventSlotPassed, beacon.round, 0, 0, 0);
    }
  }
  schedule_next_beacon();
  apply(action);
}

// Reads each packet in the radio's queue, and returns true where one was a beacon.
static bool poll_queue() {
  bool beacon = false;
  while (radio.available()) {
    const uint8_t length = radio.getDynamicPayloadSize();
    uint8_t packet[32] = {0};
    radio.read(packet, sizeof(packet));
    answer_sticks::Beacon received;
    if (answer_sticks::decode(packet, length, received)) {
      on_beacon(received, NRF_TIMER3->CC[0]);
      beacon = true;
      if (!g_awake) {
        return true;
      }
    } else {
      push_event(kEventBadPacket, length, packet[0], 0, 0);
    }
  }
  return beacon;
}

static void missed_beacon() {
  if (g_receiving && poll_queue()) {
    return;
  }
  g_have_last = false;
  // Receive without a break until two beacons in a row come again.
  g_locked = false;
  if (g_missed < 255) {
    ++g_missed;
  }
  g_expected += g_period;
  push_event(kEventMissed, g_missed, 0, 0, 0);
  const answer_sticks::StickAction action = g_logic.on_missed_beacon(clock_us(timer_now()));
  schedule_next_beacon();
  apply(action);
}

// Sends the PRESS in the stick's slot. The PPI channel is off for the send, so that the
// stick's own END doesn't overwrite the beacon's capture. stopListening() ramps the
// transmitter up, write() sends once with multicast true, and startListening() right after
// it clears the SHORTS that would ramp the transmitter up again.
static void send_press() {
  uint8_t packet[answer_sticks::kPressSize];
  const size_t length = answer_sticks::encode(g_press, packet, sizeof(packet));
  NRF_PPI->CHENCLR = 1UL << kPpiChannel;
  radio.stopListening();
  const bool sent = radio.write(packet, static_cast<uint8_t>(length), true);
  radio.startListening();
  NRF_PPI->CHENSET = 1UL << kPpiChannel;
  if (!g_receiving) {
    NRF_RADIO->TASKS_DISABLE = 1;
  }
  push_event(kEventSent, g_press.round, sent ? 1 : 0, 0, g_press.age_us);
}

// Runs each deadline that is due, then sets CC[1] to the next one.
static void run_schedule() {
  while (g_awake) {
    const uint32_t now = timer_now();
    if (g_send_due && !answer_sticks::before(now, g_send_at)) {
      g_send_due = false;
      send_press();
      continue;
    }
    if (g_receiving && !answer_sticks::before(now, g_check_at)) {
      g_check_at = now + kPollCounts;
      poll_queue();
      continue;
    }
    if (!answer_sticks::before(now, g_miss_at)) {
      missed_beacon();
      continue;
    }
    if (g_open_due && !answer_sticks::before(now, g_open_at)) {
      g_open_due = false;
      receiver_on();
      continue;
    }
    uint32_t next = g_miss_at;
    if (g_send_due && answer_sticks::before(g_send_at, next)) {
      next = g_send_at;
    }
    if (g_receiving && answer_sticks::before(g_check_at, next)) {
      next = g_check_at;
    }
    if (g_open_due && answer_sticks::before(g_open_at, next)) {
      next = g_open_at;
    }
    if (!answer_sticks::before(now + kGuardCounts, next)) {
      while (answer_sticks::before(timer_now(), next)) {
      }
      continue;
    }
    NRF_TIMER3->CC[1] = next;
    if (answer_sticks::before(timer_now() + kGuardCounts, next)) {
      return;
    }
  }
}

extern "C" void TIMER3_IRQHandler(void) {
  NRF_TIMER3->EVENTS_COMPARE[1] = 0;
  (void)NRF_TIMER3->EVENTS_COMPARE[1];  // the read makes the write take effect first
  run_schedule();
}

// The button's falling edge. Asleep, it starts TIMER3 from 0 at the edge, starts HFXO and
// wakes loop(); awake, it does nothing.
static void on_button() {
  if (!g_asleep) {
    return;
  }
  NRF_TIMER3->TASKS_CLEAR = 1;
  NRF_TIMER3->TASKS_START = 1;
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  g_asleep = false;
  g_wake_pending = true;
  resumeLoop();
}

static bool hfxo_running() {
  const uint32_t want = (CLOCK_HFCLKSTAT_SRC_Xtal << CLOCK_HFCLKSTAT_SRC_Pos) |
                        (CLOCK_HFCLKSTAT_STATE_Running << CLOCK_HFCLKSTAT_STATE_Pos);
  return (NRF_CLOCK->HFCLKSTAT & (CLOCK_HFCLKSTAT_SRC_Msk | CLOCK_HFCLKSTAT_STATE_Msk)) == want;
}

// Whether USB power is present. USB needs HFXO, so the stick leaves it on then.
static bool usb_power() {
  return (NRF_POWER->USBREGSTATUS & POWER_USBREGSTATUS_VBUSDETECT_Msk) != 0;
}

static bool load_stick(uint8_t& stick) {
  File file(InternalFS);
  if (!file.open(kStickFile, FILE_O_READ)) {
    return false;
  }
  char text[4] = {0, 0, 0, 0};
  const int length = file.read(text, sizeof(text) - 1);
  file.close();
  if (length <= 0) {
    return false;
  }
  char* end = nullptr;
  const long value = strtol(text, &end, 10);
  if (end == text || value < 0 || value >= answer_sticks::kMaxSticks) {
    return false;
  }
  stick = static_cast<uint8_t>(value);
  return true;
}

static bool save_stick(uint8_t stick) {
  char text[4];
  const int length = snprintf(text, sizeof(text), "%u", static_cast<unsigned>(stick));
  InternalFS.remove(kStickFile);
  File file(InternalFS);
  if (!file.open(kStickFile, FILE_O_WRITE)) {
    return false;
  }
  const size_t wrote = file.write(reinterpret_cast<const uint8_t*>(text), length);
  file.close();
  return wrote == static_cast<size_t>(length);
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

static void print_event(const Event& event) {
  switch (event.kind) {
    case kEventBeacon:
      Serial.printf("Beacon: round %u, %s, winner ", event.a, phase_name(event.b));
      if (event.c == answer_sticks::kNoWinner) {
        Serial.print("none");
      } else {
        Serial.print(event.c);
      }
      Serial.println(event.value != 0 ? ", this stick heard" : ", this stick not heard");
      break;
    case kEventPeriod:
      Serial.printf("Beacon period: %lu us\n", static_cast<unsigned long>(event.value));
      break;
    case kEventSent:
      Serial.printf("Sent PRESS: round %u, age %lu us%s\n", event.a,
                    static_cast<unsigned long>(event.value),
                    event.b != 0 ? "" : ", but write() didn't finish");
      break;
    case kEventSlotPassed:
      Serial.printf("Round %u: the slot had passed when the beacon was read\n", event.a);
      break;
    case kEventMissed:
      Serial.printf("No beacon: %u frames in a row\n", event.a);
      break;
    case kEventLight:
      Serial.println(event.a != 0 ? "Light on: this stick answers" : "Light off");
      break;
    case kEventSleep:
      Serial.println("Round over for this stick: sleeping once the button is released");
      break;
    case kEventBadPacket:
      Serial.printf("Ignored a packet: %u bytes, first byte %u\n", event.a, event.b);
      break;
  }
}

static void print_status() {
  Serial.printf("Answer Stick %u. Type \"id N\" to set its number to N, 0 to 63.\n", g_stick);
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

// "id" prints the stick's number; "id N" stores N in flash.
static void run_command(char* line) {
  while (*line == ' ') {
    ++line;
  }
  if (strncmp(line, "id", 2) != 0 || (line[2] != '\0' && line[2] != ' ')) {
    Serial.println("Commands: \"id\" shows this stick's number, \"id N\" sets it to N, 0 to 63.");
    return;
  }
  const char* rest = line + 2;
  while (*rest == ' ') {
    ++rest;
  }
  if (*rest == '\0') {
    Serial.printf("Stick number %u\n", g_stick);
    return;
  }
  char* end = nullptr;
  const long value = strtol(rest, &end, 10);
  while (*end == ' ') {
    ++end;
  }
  if (end == rest || *end != '\0' || value < 0 || value >= answer_sticks::kMaxSticks) {
    Serial.println("A stick number is 0 to 63.");
    return;
  }
  if (!save_stick(static_cast<uint8_t>(value))) {
    Serial.println("Couldn't save the stick number in flash.");
    return;
  }
  g_stick = static_cast<uint8_t>(value);
  Serial.printf("Stick number %u saved. It counts from the next press.\n", g_stick);
}

static void read_serial() {
  if (!Serial) {
    return;
  }
  while (Serial.available() > 0) {
    const int c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (g_command_length > 0) {
        g_command[g_command_length] = '\0';
        run_command(g_command);
        g_command_length = 0;
      }
    } else if (g_command_length < sizeof(g_command) - 1) {
      g_command[g_command_length] = static_cast<char>(c);
      ++g_command_length;
    }
  }
}

// Starts the stick's schedule after the button's handler woke it.
static void wake() {
  const uint32_t started_ms = millis();
  while (!hfxo_running() && millis() - started_ms < kHfxoWaitMs) {
  }
  NRF_TIMER3->TASKS_CAPTURE[3] = 1;
  const uint32_t hfxo_count = NRF_TIMER3->CC[3];
  const bool hfxo = hfxo_running();
  // Packets left from before the sleep belong to an earlier frame.
  while (radio.available()) {
    uint8_t packet[32];
    radio.read(packet, sizeof(packet));
  }
  NRF_TIMER3->TASKS_CAPTURE[3] = 1;
  const uint32_t now = NRF_TIMER3->CC[3];
  g_logic = answer_sticks::StickLogic(g_stick);
  g_clock_count = now;
  g_clock_us = 0;
  g_locked = false;
  g_period_known = false;
  g_period = kGuessPeriodCounts;
  g_have_last = false;
  g_missed = 0;
  g_open_due = false;
  g_send_due = false;
  g_light_on = false;
  g_logged_any = false;
  g_expected = now + g_period;
  g_miss_at = g_expected + kSearchGraceCounts;
  radio.startListening();
  g_receiving = true;
  g_check_at = now + kPollCounts;
  g_awake = true;
  NVIC_SetPendingIRQ(TIMER3_IRQn);
  if (Serial) {
    Serial.printf("Woke as stick %u; HFXO %s %lu us after the button's edge\n", g_stick,
                  hfxo ? "running" : "not running",
                  static_cast<unsigned long>(hfxo_count / kCountsPerUs));
  }
}

// Sleeps once the button has been released for kReleaseMs: TIMER3 stops, and HFXO stops
// unless USB power is present. The next falling edge wakes the stick.
static void sleep_now() {
  uint32_t released_ms = millis();
  while (millis() - released_ms < kReleaseMs) {
    if (digitalRead(kButtonPin) == LOW) {
      released_ms = millis();
    }
    delay(1);
  }
  taskENTER_CRITICAL();
  NRF_TIMER3->TASKS_STOP = 1;
  if (!usb_power()) {
    NRF_CLOCK->TASKS_HFCLKSTOP = 1;
  }
  g_asleep = true;
  taskEXIT_CRITICAL();
  if (Serial) {
    Serial.println("Asleep: press the button to play");
  }
}

// Asleep: on USB power, loop() keeps running to read the serial monitor; on the battery, it
// suspends until the button's handler resumes it. The check and the suspend sit in one
// critical section, which holds back the button's interrupt, so a press between them still
// wakes the stick.
static void rest() {
  if (usb_power()) {
    delay(5);
    return;
  }
  taskENTER_CRITICAL();
  if (g_asleep && !g_wake_pending) {
    suspendLoop();
  }
  taskEXIT_CRITICAL();
}

void setup() {
  // The core's board setup drives P0.13 to P0.16 high before setup().
  digitalWrite(kVccPin, LOW);
  pinMode(kVccPin, OUTPUT);
  digitalWrite(kBlueLedPin, LOW);
  pinMode(kBlueLedPin, OUTPUT);
  // The LED pin, with high drive (H0H1).
  NRF_P0->OUTCLR = 1UL << kLightPin;
  NRF_P0->PIN_CNF[kLightPin] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
                               (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
                               (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                               (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos) |
                               (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
  pinMode(kButtonPin, INPUT_PULLUP);

  Serial.begin(115200);
  InternalFS.begin();
  uint8_t stored = 0;
  if (load_stick(stored)) {
    g_stick = stored;
  }

  // The radio: begin() starts HFXO and sets channel 76 and a 16-bit CRC.
  radio.begin();
  radio.setDataRate(NRF_2MBPS);
  radio.enableDynamicPayloads();
  radio.setAutoAck(false);
  radio.openWritingPipe(kPressAddress);
  radio.openReadingPipe(0, kBeaconAddress);

  // TIMER3: 16 MHz, 32 bits, stopped until the button's edge.
  NRF_TIMER3->TASKS_STOP = 1;
  NRF_TIMER3->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
  NRF_TIMER3->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
  NRF_TIMER3->PRESCALER = 0;
  NRF_TIMER3->SHORTS = 0;
  NRF_TIMER3->INTENCLR = 0xFFFFFFFFUL;
  NRF_TIMER3->EVENTS_COMPARE[1] = 0;
  NRF_TIMER3->INTENSET = TIMER_INTENSET_COMPARE1_Msk;
  NRF_TIMER3->TASKS_CLEAR = 1;
  NVIC_ClearPendingIRQ(TIMER3_IRQn);
  NVIC_SetPriority(TIMER3_IRQn, kTimerPriority);
  NVIC_EnableIRQ(TIMER3_IRQn);

  // The radio's END event triggers TIMER3's CAPTURE[0].
  NRF_PPI->CH[kPpiChannel].EEP = reinterpret_cast<uint32_t>(&NRF_RADIO->EVENTS_END);
  NRF_PPI->CH[kPpiChannel].TEP = reinterpret_cast<uint32_t>(&NRF_TIMER3->TASKS_CAPTURE[0]);
  NRF_PPI->CHENSET = 1UL << kPpiChannel;

  if (!usb_power()) {
    NRF_CLOCK->TASKS_HFCLKSTOP = 1;
  }
  g_asleep = true;
  attachInterrupt(kButtonPin, on_button, FALLING);
}

void loop() {
  print_events();
  read_serial();
  if (g_wake_pending) {
    g_wake_pending = false;
    wake();
  }
  if (g_sleep_pending) {
    g_sleep_pending = false;
    sleep_now();
  }
  if (g_asleep) {
    rest();
  } else {
    delay(1);
  }
}
