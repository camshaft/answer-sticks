// Tests of the host's state machine, registered as test_suites.h says, so that main() runs
// them. Together the checks cover every transition of the state machine.
#include <host_logic.h>

#include <cstdio>
#include <cstring>

#include "test_suites.h"

namespace {

using answer_sticks::Effects;
using answer_sticks::HostLogic;
using answer_sticks::Phase;
using answer_sticks::kLineWidth;
using answer_sticks::kNoWinner;
using answer_sticks::kWrongToneHz;
using answer_sticks::kWrongToneMs;

int failures = 0;

void check(bool ok, const char* what) {
  std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) {
    ++failures;
  }
}

// Whether an input's effects ask the sketch for nothing.
bool nothing(const Effects& effects) {
  return effects.scored == kNoWinner && !effects.wrong_tone && !effects.round_ended;
}

// Whether an input ended the round without a score or the tone, as wrong and cancel do.
bool ended_quietly(const Effects& effects) {
  return effects.scored == kNoWinner && !effects.wrong_tone && effects.round_ended;
}

bool is_ready(const HostLogic& host, uint8_t round) {
  return host.phase() == Phase::kReady && host.round() == round && host.winner() == kNoWinner;
}

bool is_deciding(const HostLogic& host, uint8_t round) {
  return host.phase() == Phase::kDeciding && host.round() == round &&
         host.winner() == kNoWinner;
}

bool is_answering(const HostLogic& host, uint8_t round, uint8_t winner) {
  return host.phase() == Phase::kAnswering && host.round() == round && host.winner() == winner;
}

bool line_is(const HostLogic& host, uint32_t now_ms, const char* expected) {
  char line[kLineWidth + 1];
  const size_t length = host.first_line(now_ms, line);
  return length == std::strlen(expected) && std::strcmp(line, expected) == 0;
}

// A host answering in round 0: stick 17 won at 1000 ms, with the knob at 45 s.
HostLogic answering() {
  HostLogic host;
  host.first_press();
  host.winner_chosen(17, 45, 1000);
  return host;
}

void starts_ready() {
  HostLogic host;
  check(is_ready(host, 0), "the host starts ready in round 0 with no winner");
  check(line_is(host, 0, "Press to answer #0"), "ready shows \"Press to answer\" and the round");
}

void ready_ignores_the_inputs() {
  HostLogic host;
  check(nothing(host.correct()) && is_ready(host, 0), "correct does nothing in ready");
  check(nothing(host.wrong()) && is_ready(host, 0), "wrong does nothing in ready");
  check(nothing(host.cancel()) && is_ready(host, 0), "cancel does nothing in ready");
  check(nothing(host.winner_chosen(17, 45, 0)) && is_ready(host, 0),
        "a winner before the first PRESS does nothing in ready");
  check(nothing(host.tick(100000)) && is_ready(host, 0), "the clock does nothing in ready");
}

void ready_to_deciding() {
  HostLogic host;
  check(nothing(host.first_press()) && is_deciding(host, 0),
        "the first PRESS moves ready to deciding");
  check(line_is(host, 0, "Press to answer #0"), "deciding shows the line of ready");
  check(nothing(host.first_press()) && is_deciding(host, 0), "a second PRESS keeps deciding");
}

void deciding_ignores_the_inputs() {
  HostLogic host;
  host.first_press();
  check(nothing(host.correct()) && is_deciding(host, 0), "correct does nothing in deciding");
  check(nothing(host.wrong()) && is_deciding(host, 0), "wrong does nothing in deciding");
  check(nothing(host.cancel()) && is_deciding(host, 0), "cancel does nothing in deciding");
  check(nothing(host.tick(100000)) && is_deciding(host, 0),
        "the clock does nothing in deciding");
  check(nothing(host.winner_chosen(64, 45, 0)) && is_deciding(host, 0),
        "stick 64 isn't a winner, and leaves the host deciding");
}

void deciding_to_answering() {
  HostLogic host;
  host.first_press();
  check(nothing(host.winner_chosen(17, 45, 1000)) && is_answering(host, 0, 17),
        "the winner chosen moves deciding to answering");
  check(line_is(host, 1000, "Stick 17: 45 s left"),
        "answering shows \"Stick 17\" and the seconds left");
}

void countdown() {
  HostLogic host = answering();
  check(host.seconds_left(1000) == 45, "45 s left as the winner is chosen");
  check(host.seconds_left(1999) == 45, "45 s left until a whole second passes");
  check(host.seconds_left(2000) == 44, "44 s left after 1 s");
  check(line_is(host, 2000, "Stick 17: 44 s left"), "the line shows the seconds left");
  check(host.seconds_left(45999) == 1, "1 s left until the countdown reaches 0");
  check(host.seconds_left(46000) == 0, "0 s left after 45 s");
  check(nothing(host.tick(45999)) && is_answering(host, 0, 17),
        "the clock before 0 keeps the host answering");
}

void knob_seconds_at_the_choice() {
  HostLogic shortest;
  shortest.first_press();
  shortest.winner_chosen(0, 3, 0);
  check(line_is(shortest, 0, "Stick 0: 3 s left") && shortest.seconds_left(2999) == 1,
        "the countdown starts from 3 s, the knob's shortest");
  HostLogic longest;
  longest.first_press();
  longest.winner_chosen(63, 60, 0);
  check(line_is(longest, 0, "Stick 63: 60 s left") && longest.seconds_left(59999) == 1,
        "the countdown starts from 60 s, the knob's longest");
}

void answering_ignores_a_press_and_a_winner() {
  HostLogic host = answering();
  check(nothing(host.first_press()) && is_answering(host, 0, 17),
        "a PRESS does nothing in answering");
  check(nothing(host.winner_chosen(5, 10, 2000)) && is_answering(host, 0, 17) &&
            host.seconds_left(2000) == 44,
        "a second winner does nothing in answering");
}

void correct_ends_the_round() {
  HostLogic host = answering();
  const Effects effects = host.correct();
  check(effects.scored == 17 && effects.round_ended && !effects.wrong_tone,
        "correct adds 1 to the winner's score and ends the round");
  check(is_ready(host, 1), "ending the round opens round 1 in ready");
  check(line_is(host, 2000, "Press to answer #1"), "the next round shows its number");
  check(nothing(host.first_press()) && is_deciding(host, 1),
        "the next round moves to deciding on its first PRESS");
}

void wrong_ends_the_round() {
  HostLogic host = answering();
  check(ended_quietly(host.wrong()) && is_ready(host, 1),
        "wrong ends the round without a score or the tone");
}

void cancel_ends_the_round() {
  HostLogic host = answering();
  check(ended_quietly(host.cancel()) && is_ready(host, 1),
        "cancel ends the round without a score or the tone");
}

void timeout_ends_the_round() {
  check(kWrongToneHz == 4000 && kWrongToneMs == 800, "the wrong-answer tone is 4 kHz for 800 ms");
  HostLogic host = answering();
  const Effects effects = host.tick(46000);
  check(effects.wrong_tone && effects.round_ended && effects.scored == kNoWinner,
        "the countdown reaching 0 plays the wrong-answer tone and ends the round");
  check(is_ready(host, 1), "the timeout opens round 1 in ready");
  check(nothing(host.tick(47000)) && is_ready(host, 1),
        "the clock after the timeout does nothing");
  HostLogic late = answering();
  check(late.tick(100000).wrong_tone && is_ready(late, 1),
        "a clock read well past 0 ends the round");
}

void clock_wraps() {
  HostLogic host;
  host.first_press();
  // The winner is chosen 500 ms before the 32-bit clock wraps to 0.
  host.winner_chosen(9, 3, 0xFFFFFFFFu - 499u);
  check(host.seconds_left(1500) == 1, "the countdown counts across the clock's wrap");
  check(nothing(host.tick(2499)) && is_answering(host, 0, 9),
        "1 ms before 0 across the clock's wrap, the host keeps answering");
  check(host.tick(2500).wrong_tone && is_ready(host, 1),
        "the countdown reaches 0 across the clock's wrap");
}

void round_wraps() {
  HostLogic host;
  for (int i = 0; i < 255; ++i) {
    host.first_press();
    host.winner_chosen(1, 3, 0);
    host.cancel();
  }
  char line[kLineWidth + 1];
  check(host.round() == 255 && host.first_line(0, line) == kLineWidth &&
            line_is(host, 0, "Press to answer #255"),
        "round 255's line fills the screen's 20 characters");
  host.first_press();
  host.winner_chosen(1, 3, 0);
  check(host.cancel().round_ended && is_ready(host, 0), "the round after 255 is 0");
}

int run() {
  failures = 0;
  starts_ready();
  ready_ignores_the_inputs();
  ready_to_deciding();
  deciding_ignores_the_inputs();
  deciding_to_answering();
  countdown();
  knob_seconds_at_the_choice();
  answering_ignores_a_press_and_a_winner();
  correct_ends_the_round();
  wrong_ends_the_round();
  cancel_ends_the_round();
  timeout_ends_the_round();
  clock_wraps();
  round_wraps();
  return failures;
}

answer_sticks_test::AddSuite add_host_logic_suite("host state machine", run);

}  // namespace
