// Tests of the stick's rules for each beacon it receives and its guards for lost beacons:
// each rule, a stick that stops sending once its bit is set, a stick that receives a beacon
// in answering and sends nothing, the sleep after 5 frames without a beacon, and the winner's
// light kept through lost beacons until 61 s pass.
#include <stick_logic.h>

#include <cstdio>

#include "test_suites.h"

namespace {

using answer_sticks::Beacon;
using answer_sticks::Phase;
using answer_sticks::Press;
using answer_sticks::StickAction;
using answer_sticks::StickLogic;
using answer_sticks::age_us_from_timer;
using answer_sticks::kNoWinner;
using answer_sticks::set_heard;
using answer_sticks_test::check_number;

// The design's frame: "about 0.8 ms of every 26.2 ms frame". The tests step a stick's clock
// by one frame a beacon.
constexpr uint32_t kFrameUs = 26200;

// "for up to 61 s without a beacon".
constexpr uint32_t kSixtyOneSecondsUs = 61000000;

int check(bool ok, const char* what) {
  std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
  return ok ? 0 : 1;
}

Beacon beacon(uint8_t round, Phase phase, uint8_t winner = kNoWinner) {
  Beacon made;
  made.round = round;
  made.phase = phase;
  made.winner = winner;
  return made;
}

Beacon heard(Beacon with, uint8_t stick) {
  set_heard(with, stick);
  return with;
}

// The four answers a stick returns: keep receiving, send a PRESS, light on, and sleep.
bool receiving(const StickAction& action) {
  return !action.send_press && !action.light && !action.sleep;
}
bool sending(const StickAction& action) {
  return action.send_press && !action.light && !action.sleep;
}
bool lit(const StickAction& action) {
  return !action.send_press && action.light && !action.sleep;
}
bool sleeping(const StickAction& action) {
  return !action.send_press && !action.light && action.sleep;
}

// A stick that pressed in round, was heard, and won: its light is on at clock now_us.
StickLogic winner(uint8_t stick, uint8_t round, uint32_t now_us) {
  StickLogic logic(stick);
  logic.on_beacon(beacon(round, Phase::kReady), now_us - 2 * kFrameUs);
  logic.on_beacon(heard(beacon(round, Phase::kDeciding), stick), now_us - kFrameUs);
  logic.on_beacon(heard(beacon(round, Phase::kAnswering, stick), stick), now_us);
  return logic;
}

int rules() {
  int failures = 0;

  StickLogic ready(5);
  failures += check(sending(ready.on_beacon(beacon(7, Phase::kReady), 0)),
                    "ready with its bit clear: the stick sends a PRESS this frame");
  Press press = ready.press(16 * 4100);
  failures += check(press.stick == 5 && press.round == 7 && press.age_us == 4100,
                    "the PRESS names the stick, the beacon's round and the age from the timer");

  StickLogic deciding(5);
  Beacon other_heard = heard(beacon(7, Phase::kDeciding), 4);
  failures += check(sending(deciding.on_beacon(other_heard, 0)),
                    "deciding with its bit clear: the stick sends a PRESS this frame");

  StickLogic heard_stick(5);
  heard_stick.on_beacon(beacon(7, Phase::kReady), 0);
  Beacon own_heard = heard(beacon(7, Phase::kDeciding), 5);
  failures += check(receiving(heard_stick.on_beacon(own_heard, kFrameUs)),
                    "deciding with its bit set: the stick keeps receiving and doesn't send");

  StickLogic won(5);
  won.on_beacon(beacon(7, Phase::kReady), 0);
  won.on_beacon(own_heard, kFrameUs);
  Beacon won_round = heard(beacon(7, Phase::kAnswering, 5), 5);
  failures += check(lit(won.on_beacon(won_round, 2 * kFrameUs)),
                    "answering with itself as winner in its round: light on, it keeps receiving");

  StickLogic lost(5);
  lost.on_beacon(beacon(7, Phase::kReady), 0);
  Beacon other_won = heard(beacon(7, Phase::kAnswering, 4), 5);
  failures += check(sleeping(lost.on_beacon(other_won, kFrameUs)),
                    "answering with another winner: light off, sleep");

  StickLogic nobody(5);
  nobody.on_beacon(beacon(7, Phase::kReady), 0);
  Beacon none_won = heard(beacon(7, Phase::kAnswering), 5);
  failures += check(sleeping(nobody.on_beacon(none_won, kFrameUs)),
                    "answering with winner 255: light off, sleep");

  StickLogic late(5);
  failures += check(sleeping(late.on_beacon(beacon(7, Phase::kAnswering, 5), 0)),
                    "answering before it sent a PRESS, with itself as winner: light off, sleep");

  StickLogic next_round(5);
  next_round.on_beacon(beacon(7, Phase::kReady), 0);
  failures += check(sleeping(next_round.on_beacon(beacon(8, Phase::kReady), kFrameUs)),
                    "a ready beacon of another round, its bit clear: sleep without a PRESS");

  StickLogic old_answer(5);
  old_answer.on_beacon(beacon(7, Phase::kReady), 0);
  Beacon next_won = heard(beacon(8, Phase::kAnswering, 5), 5);
  failures += check(sleeping(old_answer.on_beacon(next_won, kFrameUs)),
                    "answering with itself as winner in another round: light off, sleep");

  return failures;
}

int stops_sending_once_heard() {
  int failures = 0;
  StickLogic logic(63);
  failures += check(sending(logic.on_beacon(beacon(9, Phase::kReady), 0)),
                    "frame F: the stick sends its PRESS");
  Beacon others = heard(beacon(9, Phase::kDeciding), 0);
  failures += check(sending(logic.on_beacon(others, kFrameUs)),
                    "frame F+1, the host didn't hear its PRESS: the stick sends it again");
  Beacon own = heard(others, 63);
  bool quiet = true;
  for (uint32_t frame = 2; frame < 7; ++frame) {
    quiet = receiving(logic.on_beacon(own, frame * kFrameUs)) && quiet;
  }
  failures += check(quiet, "once its bit is set, the stick doesn't send again in 5 beacons");
  failures += check(logic.pressed() && logic.press_round() == 9, "the PRESS named round 9");
  return failures;
}

int answering_sends_nothing() {
  int failures = 0;
  StickLogic woke_late(20);
  StickAction action = woke_late.on_beacon(heard(beacon(4, Phase::kAnswering, 3), 3), 0);
  failures += check(!action.send_press && action.sleep,
                    "a stick that receives a beacon in answering sends nothing and sleeps");
  failures += check(!woke_late.pressed(), "that stick didn't send a PRESS");

  StickLogic logic = winner(20, 4, 3 * kFrameUs);
  Beacon won = heard(beacon(4, Phase::kAnswering, 20), 20);
  bool quiet = true;
  for (uint32_t frame = 4; frame < 9; ++frame) {
    quiet = lit(logic.on_beacon(won, frame * kFrameUs)) && quiet;
  }
  failures += check(quiet, "the winner sends nothing in 5 beacons of answering");
  return failures;
}

int winner_off_at_once() {
  int failures = 0;
  const uint32_t lit_at = 3 * kFrameUs;

  StickLogic other_winner = winner(10, 2, lit_at);
  Beacon other_won = heard(heard(beacon(2, Phase::kAnswering, 11), 10), 11);
  failures += check(sleeping(other_winner.on_beacon(other_won, lit_at + kFrameUs)),
                    "the winner's light goes off on a beacon with another winner");

  StickLogic deciding = winner(10, 2, lit_at);
  Beacon deciding_round = heard(beacon(2, Phase::kDeciding), 10);
  failures += check(sleeping(deciding.on_beacon(deciding_round, lit_at + kFrameUs)),
                    "the winner's light goes off on a deciding beacon of its round");

  StickLogic ready = winner(10, 2, lit_at);
  failures += check(sleeping(ready.on_beacon(beacon(2, Phase::kReady), lit_at + kFrameUs)),
                    "the winner's light goes off on a ready beacon of its round, without a PRESS");

  StickLogic next = winner(10, 2, lit_at);
  failures += check(sleeping(next.on_beacon(beacon(3, Phase::kReady), lit_at + kFrameUs)),
                    "the winner's light goes off on a beacon of the next round");
  return failures;
}

int round_wraps() {
  int failures = 0;
  StickLogic logic = winner(1, 255, 3 * kFrameUs);
  failures += check(logic.light(), "a stick that pressed in round 255 lights as its winner");
  failures += check(sleeping(logic.on_beacon(beacon(0, Phase::kReady), 4 * kFrameUs)),
                    "round 0 after round 255 is another round: light off, sleep");
  return failures;
}

int sleeps_after_5_missed_beacons() {
  int failures = 0;
  StickLogic logic(30);
  logic.on_beacon(beacon(6, Phase::kReady), 0);
  logic.on_beacon(heard(beacon(6, Phase::kDeciding), 30), kFrameUs);
  bool awake = true;
  for (uint32_t frame = 2; frame < 6; ++frame) {
    awake = receiving(logic.on_missed_beacon(frame * kFrameUs)) && awake;
  }
  failures += check(awake, "in deciding, the stick stays awake through 4 frames without a beacon");
  failures += check(sleeping(logic.on_missed_beacon(6 * kFrameUs)),
                    "in deciding, the 5th frame in a row without a beacon: light off, sleep");
  failures += check(sleeping(logic.on_beacon(beacon(7, Phase::kReady), 7 * kFrameUs)) &&
                        logic.press_round() == 6,
                    "a beacon after the sleep doesn't wake the stick or send a PRESS");

  StickLogic counted(30);
  counted.on_beacon(beacon(6, Phase::kReady), 0);
  for (uint32_t frame = 1; frame < 5; ++frame) {
    counted.on_missed_beacon(frame * kFrameUs);
  }
  counted.on_beacon(heard(beacon(6, Phase::kDeciding), 30), 5 * kFrameUs);
  awake = true;
  for (uint32_t frame = 6; frame < 10; ++frame) {
    awake = receiving(counted.on_missed_beacon(frame * kFrameUs)) && awake;
  }
  failures += check(awake, "a beacon starts the count of frames without a beacon again");
  failures += check(sleeping(counted.on_missed_beacon(10 * kFrameUs)),
                    "5 frames without a beacon after that beacon: sleep");

  StickLogic no_host(30);
  for (uint32_t frame = 0; frame < 4; ++frame) {
    no_host.on_missed_beacon(frame * kFrameUs);
  }
  failures += check(!no_host.asleep() && sleeping(no_host.on_missed_beacon(4 * kFrameUs)),
                    "before its first beacon, the stick sleeps on the 5th frame without one");
  failures += check(sleeping(no_host.on_beacon(beacon(1, Phase::kReady), 5 * kFrameUs)) &&
                        !no_host.pressed(),
                    "a ready beacon after that sleep doesn't wake the stick or send a PRESS");
  return failures;
}

int winner_kept_through_lost_beacons() {
  int failures = 0;
  const uint32_t lit_at = 3 * kFrameUs;
  StickLogic logic = winner(40, 5, lit_at);
  bool on = true;
  uint32_t lost = 0;
  for (uint32_t elapsed = kFrameUs; elapsed < kSixtyOneSecondsUs; elapsed += kFrameUs) {
    on = lit(logic.on_missed_beacon(lit_at + elapsed)) && on;
    ++lost;
  }
  failures += check(on && lost > 5,
                    "the winner's light stays on through each lost beacon before 61 s");
  failures += check(lit(logic.on_missed_beacon(lit_at + kSixtyOneSecondsUs - 1)),
                    "the winner's light is on 1 us before 61 s without a beacon");
  failures += check(sleeping(logic.on_missed_beacon(lit_at + kSixtyOneSecondsUs)),
                    "61 s without a beacon: the winner's light goes off and it sleeps");

  const uint32_t near_wrap = 0xFFFFFFFFu - 1000000;
  StickLogic wrapped = winner(41, 5, near_wrap);
  failures += check(lit(wrapped.on_missed_beacon(near_wrap + kSixtyOneSecondsUs - 1)),
                    "the light stays on where the microsecond clock wraps before 61 s");
  failures += check(sleeping(wrapped.on_missed_beacon(near_wrap + kSixtyOneSecondsUs)),
                    "the light goes off 61 s after the last beacon across the clock's wrap");

  StickLogic renewed = winner(42, 5, lit_at);
  renewed.on_missed_beacon(lit_at + 30000000);
  renewed.on_beacon(heard(beacon(5, Phase::kAnswering, 42), 42), lit_at + 60000000);
  failures += check(lit(renewed.on_missed_beacon(lit_at + kSixtyOneSecondsUs)),
                    "a beacon with itself as winner starts the 61 s again");
  failures += check(sleeping(renewed.on_missed_beacon(lit_at + 60000000 + kSixtyOneSecondsUs)),
                    "61 s after that beacon, the light goes off");
  return failures;
}

int ages() {
  int failures = 0;
  failures += check_number(age_us_from_timer(0), 0, "timer count 0 is an age of 0 us");
  failures += check_number(age_us_from_timer(15), 0, "timer count 15 is an age of 0 us");
  failures += check_number(age_us_from_timer(16), 1, "timer count 16 is an age of 1 us");
  failures += check_number(age_us_from_timer(65536 + 16), 4097,
                           "timer count 2^16 + 16 is an age of 4097 us");
  failures += check_number(age_us_from_timer(0xFFFFFFFFu), 268435455,
                           "the highest 32-bit timer count is an age of 268435455 us");
  return failures;
}

int run() {
  return rules() + stops_sending_once_heard() + answering_sends_nothing() +
         winner_off_at_once() + round_wraps() + sleeps_after_5_missed_beacons() +
         winner_kept_through_lost_beacons() + ages();
}

answer_sticks_test::AddSuite add_stick_logic_suite("stick logic", run);

}  // namespace
