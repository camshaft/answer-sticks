// Tests of the frame's time slots, the press times and the host's choice of the first press
// of a round: slot times, press times for several ages and timer counts, the arbitration
// rule with simulated ages and arrival orders, the heard mask, and round binding.
#include <arbitration.h>

#include <cstddef>
#include <cstdio>

#include "test_suites.h"

namespace {

using answer_sticks::Arbiter;
using answer_sticks::Phase;
using answer_sticks::Press;
using answer_sticks::before;
using answer_sticks::is_heard;
using answer_sticks::kCountsPerUs;
using answer_sticks::kFrameUs;
using answer_sticks::kNoWinner;
using answer_sticks::press_time;
using answer_sticks::slot_offset_counts;
using answer_sticks::slot_offset_us;
using answer_sticks::slot_start;

// The failed checks of the suite's run.
int failures = 0;

void check(bool ok, const char* what) {
  std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) {
    ++failures;
  }
}

// The timer's counts in one frame.
constexpr uint32_t kFrameCounts = kFrameUs * kCountsPerUs;

// Microseconds as the timer's counts.
constexpr uint32_t us(uint32_t microseconds) { return microseconds * kCountsPerUs; }

// The count at which frame F's beacon ended in most cases below: past 2^16, so that a count
// that kept only 16 bits fails them.
constexpr uint32_t kEnd = 100000;

// The PRESS that stick sends for round in the frame whose beacon ended at count end, for a
// button pressed at count pressed: its age is the microseconds from the press to the start
// of its slot. The simulation counts the host's timer and the stick's as one timer.
Press sent(uint8_t stick, uint8_t round, uint32_t pressed, uint32_t end) {
  Press press;
  press.stick = stick;
  press.round = round;
  press.age_us = (slot_start(end, stick) - pressed) / kCountsPerUs;
  return press;
}

bool no_stick_heard(const Arbiter& arbiter) {
  for (size_t i = 0; i < sizeof arbiter.beacon().heard; ++i) {
    if (arbiter.beacon().heard[i] != 0) {
      return false;
    }
  }
  return true;
}

// The winner of round 7 where count presses arrive in the order given, all in frame F,
// whose beacon ended at end, and frame F+1 brings none.
uint8_t winner_of(const Press* presses, size_t count, uint32_t end) {
  Arbiter arbiter(7);
  for (size_t i = 0; i < count; ++i) {
    arbiter.take(presses[i], end);
  }
  arbiter.end_frame();
  arbiter.end_frame();
  return arbiter.beacon().winner;
}

void slots() {
  check(kFrameUs == 26200, "a frame is 26200 us");
  check(slot_offset_us(0) == 600, "stick 0's slot starts 600 us after the beacon's end");
  check(slot_offset_us(63) == 25800, "stick 63's slot starts 25800 us after the beacon's end");
  check(slot_offset_us(63) + 400 == kFrameUs, "stick 63's slot ends at the end of the frame");
  check(slot_offset_counts(0) == 9600, "stick 0's offset is 9600 counts at 16 a microsecond");
  check(slot_offset_counts(63) == 412800, "stick 63's offset is 412800 counts");
  check(slot_start(kEnd, 0) == 109600,
        "stick 0's slot starts at the beacon's end capture plus 9600 counts");
  check(slot_start(kEnd, 63) == 512800,
        "stick 63's slot starts at the beacon's end capture plus 412800 counts");
}

void press_times() {
  // Stick 12's slot starts 5400 us, 86400 counts, after the beacon's end.
  check(press_time(kEnd, 12, 4000) == 122400,
        "an age of 4000 us, under 4.1 ms, is 64000 counts before stick 12's slot");
  // A button pressed at the start of stick 12's slot in frame F, count 186400, and sent in
  // each of the next two frames, whose beacons end one and two frames later.
  check(press_time(kEnd + kFrameCounts, 12, 26200) == 186400,
        "an age of one frame, in frame F+1, is the press's count in frame F");
  check(press_time(kEnd + 2 * kFrameCounts, 12, 52400) == 186400,
        "an age of two frames, in frame F+2, is the press's count in frame F");
  check(press_time(65541, 0, 600) == 65541,
        "a press at a beacon's end captured past 2^16 keeps the count's high bits");
  check(press_time(0xFFFFFF00u, 0, 0) == 9344,
        "a slot start past the 32-bit timer's wrap wraps as the timer does");
}

void timer_order() {
  check(before(1, 2) && !before(2, 1), "count 1 comes before count 2");
  check(!before(5, 5), "a count doesn't come before itself");
  check(before(0xFFFFFF00u, 0x100u) && !before(0x100u, 0xFFFFFF00u),
        "a count just before the 32-bit timer's wrap comes before one just after it");
}

void ages_and_arrival_orders() {
  // Stick 40 was pressed first, 500 us after frame F's beacon ended, then stick 25 and then
  // stick 10. The host receives their PRESS packets in the order of their slots, and in two
  // other orders.
  const Press s10 = sent(10, 7, kEnd + us(3000), kEnd);
  const Press s25 = sent(25, 7, kEnd + us(2000), kEnd);
  const Press s40 = sent(40, 7, kEnd + us(500), kEnd);
  check(s10.age_us == 1600 && s25.age_us == 8600 && s40.age_us == 16100,
        "the first press has the largest age and the last slot");
  const Press slot_order[3] = {s10, s25, s40};
  const Press reversed[3] = {s40, s25, s10};
  const Press mixed[3] = {s25, s40, s10};
  check(winner_of(slot_order, 3, kEnd) == 40,
        "the earliest press wins where its PRESS arrives last");
  check(winner_of(reversed, 3, kEnd) == 40,
        "the earliest press wins where its PRESS arrives first");
  check(winner_of(mixed, 3, kEnd) == 40,
        "the earliest press wins where its PRESS arrives second");
}

void sixty_four_sticks() {
  // Every stick presses in frame F: stick 37 at the beacon's end, and each other stick
  // 100 + its number microseconds after it, before its slot.
  Press presses[64];
  for (uint8_t stick = 0; stick < 64; ++stick) {
    const uint32_t pressed = kEnd + (stick == 37 ? 0u : us(100u + stick));
    presses[stick] = sent(stick, 7, pressed, kEnd);
  }
  Arbiter arbiter(7);
  bool all_taken = true;
  for (size_t i = 0; i < 64; ++i) {
    all_taken = arbiter.take(presses[i], kEnd) && all_taken;
  }
  check(all_taken, "the host takes the PRESS of each of 64 sticks in one frame");
  bool all_heard = true;
  for (uint8_t stick = 0; stick < 64; ++stick) {
    all_heard = all_heard && is_heard(arbiter.beacon(), stick);
  }
  check(all_heard, "the heard mask has the bit of each of the 64 sticks");
  check(!arbiter.end_frame() && arbiter.end_frame() && arbiter.beacon().winner == 37,
        "the earliest of 64 presses in one frame wins at the end of frame F+1");

  Press reversed[64];
  for (size_t i = 0; i < 64; ++i) {
    reversed[i] = presses[63 - i];
  }
  check(winner_of(reversed, 64, kEnd) == 37,
        "the earliest of 64 presses wins where they arrive in the reverse order of their slots");
}

void presses_twenty_ms_apart() {
  // Stick 5, A, was pressed 100 us before frame F's beacon ended, too late to receive it, and
  // sends in frame F+1. Stick 60, B, was pressed 20 ms after A and sends in frame F, so its
  // PRESS is the round's first.
  const uint32_t end_f1 = kEnd + kFrameCounts;
  const uint32_t a_pressed = kEnd - us(100);
  const uint32_t b_pressed = a_pressed + us(20000);
  Arbiter arbiter(7);
  check(arbiter.take(sent(60, 7, b_pressed, kEnd), kEnd) &&
            arbiter.beacon().phase == Phase::kDeciding,
        "B's PRESS in frame F, pressed 20 ms after A, moves the round to deciding");
  check(!arbiter.end_frame(), "the end of frame F doesn't decide");
  check(arbiter.take(sent(5, 7, a_pressed, end_f1), end_f1), "A's PRESS in frame F+1 is taken");
  check(arbiter.end_frame() && arbiter.beacon().winner == 5,
        "A, pressed 20 ms before B, wins though its PRESS arrived a frame later");
}

void press_lost_once() {
  // Stick 10 was pressed 1000 us after frame F's beacon ended, and its PRESS in frame F was
  // lost. Stick 20 was pressed 1000 us later, and its PRESS in frame F arrived.
  const uint32_t end_f1 = kEnd + kFrameCounts;
  const uint32_t first = kEnd + us(1000);
  Arbiter arbiter(7);
  arbiter.take(sent(20, 7, kEnd + us(2000), kEnd), kEnd);
  check(!arbiter.end_frame(), "the end of frame F doesn't decide");
  const Press again = sent(10, 7, first, end_f1);
  check(again.age_us == sent(10, 7, first, kEnd).age_us + kFrameUs,
        "stick 10's PRESS sent again in frame F+1 is one frame older");
  check(arbiter.take(again, end_f1), "the PRESS sent again in frame F+1 is taken");
  check(arbiter.end_frame() && arbiter.beacon().winner == 10,
        "stick 10, pressed first, wins with its PRESS sent again in frame F+1");
}

void ties() {
  // Sticks 9 and 30 were pressed at the same count.
  const uint32_t pressed = kEnd + us(500);
  const Press s9 = sent(9, 7, pressed, kEnd);
  const Press s30 = sent(30, 7, pressed, kEnd);
  check(press_time(kEnd, 9, s9.age_us) == press_time(kEnd, 30, s30.age_us),
        "sticks 9 and 30 have the same press time");
  const Press higher_first[2] = {s30, s9};
  const Press lower_first[2] = {s9, s30};
  check(winner_of(higher_first, 2, kEnd) == 9,
        "a tie goes to the lower stick where the higher stick's PRESS arrives first");
  check(winner_of(lower_first, 2, kEnd) == 9,
        "a tie goes to the lower stick where its PRESS arrives first");
}

void decision_after_frame_f_plus_1() {
  Arbiter arbiter(7);
  check(!arbiter.end_frame() && arbiter.beacon().phase == Phase::kReady,
        "a frame without a PRESS leaves the round ready");
  arbiter.take(sent(3, 7, kEnd + us(100), kEnd), kEnd);
  check(arbiter.beacon().phase == Phase::kDeciding && arbiter.beacon().winner == kNoWinner,
        "the round's first PRESS moves it to deciding, without a winner yet");
  check(!arbiter.end_frame() && arbiter.beacon().phase == Phase::kDeciding &&
            arbiter.beacon().winner == kNoWinner,
        "the end of frame F leaves the round deciding");
  check(arbiter.end_frame() && arbiter.beacon().phase == Phase::kAnswering &&
            arbiter.beacon().winner == 3,
        "the end of frame F+1 sets the winner and moves the round to answering");
  check(!arbiter.end_frame() && arbiter.beacon().phase == Phase::kAnswering &&
            arbiter.beacon().winner == 3,
        "the end of a later frame changes nothing");
}

void heard_mask_and_one_count() {
  Arbiter arbiter(7);
  arbiter.take(sent(17, 7, kEnd + us(5000), kEnd), kEnd);
  arbiter.take(sent(0, 7, kEnd + us(100), kEnd), kEnd);
  arbiter.take(sent(63, 7, kEnd + us(200), kEnd), kEnd);
  const uint8_t expected[8] = {0x01, 0, 0x02, 0, 0, 0, 0, 0x80};
  bool same = true;
  for (size_t i = 0; i < 8; ++i) {
    same = same && arbiter.beacon().heard[i] == expected[i];
  }
  check(same, "the heard mask has the bits of sticks 0, 17 and 63 alone");

  // Stick 17 sends again, with an age that would make its press the earliest.
  check(!arbiter.take(sent(17, 7, kEnd - us(1000), kEnd), kEnd),
        "a second PRESS of stick 17 in the round isn't taken");
  same = true;
  for (size_t i = 0; i < 8; ++i) {
    same = same && arbiter.beacon().heard[i] == expected[i];
  }
  check(same, "a second PRESS of stick 17 leaves the heard mask as it was");
  check(!arbiter.end_frame() && arbiter.end_frame() && arbiter.beacon().winner == 0,
        "the host counts stick 17's first PRESS alone, and stick 0 wins");
}

void round_binding() {
  Arbiter arbiter(5);
  check(!arbiter.take(sent(3, 4, kEnd + us(100), kEnd), kEnd) &&
            arbiter.beacon().phase == Phase::kReady && !is_heard(arbiter.beacon(), 3),
        "a PRESS naming round 4 isn't taken in round 5");
  check(!arbiter.take(sent(3, 6, kEnd + us(100), kEnd), kEnd) &&
            arbiter.beacon().phase == Phase::kReady && !is_heard(arbiter.beacon(), 3),
        "a PRESS naming round 6 isn't taken in round 5");
  check(arbiter.take(sent(3, 5, kEnd + us(100), kEnd), kEnd) &&
            arbiter.beacon().phase == Phase::kDeciding,
        "a PRESS naming round 5 is taken in round 5");
  arbiter.end_frame();
  arbiter.end_frame();
  check(!arbiter.take(sent(8, 5, kEnd + us(100), kEnd), kEnd) &&
            !is_heard(arbiter.beacon(), 8) && arbiter.beacon().winner == 3,
        "a PRESS of the current round isn't taken in answering");

  arbiter.open_next();
  check(arbiter.beacon().round == 6 && arbiter.beacon().phase == Phase::kReady &&
            arbiter.beacon().winner == kNoWinner && no_stick_heard(arbiter),
        "the next round is one higher, in ready, with no stick heard and no winner");
  check(!arbiter.take(sent(3, 5, kEnd + us(100), kEnd), kEnd) &&
            arbiter.beacon().phase == Phase::kReady && no_stick_heard(arbiter),
        "a PRESS naming the old round 5 is ignored in round 6");
  check(arbiter.take(sent(3, 6, kEnd + us(100), kEnd), kEnd),
        "a PRESS naming round 6 is taken in round 6");

  Press past_the_last;
  past_the_last.stick = 64;
  past_the_last.round = 6;
  check(!arbiter.take(past_the_last, kEnd), "a PRESS of stick 64 isn't taken");
}

void round_wrap() {
  Arbiter arbiter(255);
  arbiter.open_next();
  check(arbiter.beacon().round == 0 && arbiter.beacon().phase == Phase::kReady,
        "the round after round 255 is round 0");
  check(!arbiter.take(sent(2, 255, kEnd + us(100), kEnd), kEnd) && no_stick_heard(arbiter),
        "a PRESS naming round 255 is ignored in round 0");
  check(arbiter.take(sent(2, 0, kEnd + us(100), kEnd), kEnd) && is_heard(arbiter.beacon(), 2),
        "a PRESS naming round 0 is taken in round 0");
}

void across_the_timer_wrap() {
  // Frame F's beacon ended 0x1000 counts before the 32-bit timer wrapped. Stick 63 was
  // pressed before the wrap and stick 0 after it, so stick 0's press has the smaller count.
  const uint32_t end = 0xFFFFF000u;
  const Press s0 = sent(0, 7, 0x100u, end);
  const Press s63 = sent(63, 7, 0xFFFFF100u, end);
  check(s0.age_us == 328 && s63.age_us == 25784,
        "the ages of presses on both sides of the timer's wrap");
  const Press both[2] = {s0, s63};
  check(winner_of(both, 2, end) == 63,
        "stick 63, pressed before the timer's wrap, wins over stick 0, pressed after it");
}

int run() {
  failures = 0;
  slots();
  press_times();
  timer_order();
  ages_and_arrival_orders();
  sixty_four_sticks();
  presses_twenty_ms_apart();
  press_lost_once();
  ties();
  decision_after_frame_f_plus_1();
  heard_mask_and_one_count();
  round_binding();
  round_wrap();
  across_the_timer_wrap();
  return failures;
}

answer_sticks_test::AddSuite add_arbitration_suite("arbitration", run);

}  // namespace
