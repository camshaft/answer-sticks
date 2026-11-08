// The host's state machine, as plain C++ with no hardware calls: the round, its phase, the
// winner, the countdown of the winner's answer and the first line of the host's screen. The
// game's design sets each state, input and value, quoted beside each one below. The host
// sketch reads the buttons, the knob, the clock and the arbitration's choice, calls the
// input each one stands for, and acts on the Effects that input returns: it adds 1 to the
// winner's score, plays the tone, and sends the round, the phase and the winner in its next
// beacon.
#ifndef ANSWER_STICKS_HOST_LOGIC_H
#define ANSWER_STICKS_HOST_LOGIC_H

#include <stddef.h>
#include <stdint.h>

#include "AnswerSticks.h"
#include "packets.h"

namespace answer_sticks {

// "Wrong-answer tone: 4 kHz for 800 ms on the piezo, with `tone()`. 4 kHz is the buzzer's
// rated frequency, and the only one at which its data sheet states a sound level, 70 dB(A)
// at 10 cm with a 3 V square wave [TDK p3, p4]". The design sets the 800 ms.
constexpr uint16_t kWrongToneHz = 4000;
constexpr uint16_t kWrongToneMs = 800;

// The characters of one line of the host's screen: "its four lines start at display
// addresses 00h, 40h, 14h and 54h [NHD p5; HD p29; ST p20]", so the first line runs from
// 00h to 13h, 20 characters, and the third line starts at 14h.
constexpr size_t kLineWidth = 20;

// What an input asks the host sketch to do. A field left at its default asks for nothing.
struct Effects {
  // The stick whose score goes up by 1, or kNoWinner where no score changes: "correct adds
  // 1 to the winner's score".
  uint8_t scored = kNoWinner;
  // Whether to play the wrong-answer tone, kWrongToneHz for kWrongToneMs: "the timer
  // reaching 0 plays the wrong-answer tone and ends the round".
  bool wrong_tone = false;
  // Whether the input ended the round and opened the next round in ready.
  bool round_ended = false;
};

// The host's state: the round, and its phase of ready, deciding or answering, which the
// host's beacon carries, with the winner and the countdown of the winner's answer.
class HostLogic {
 public:
  // The round's number. The host starts in round 0, ready.
  uint8_t round() const { return round_; }
  Phase phase() const { return phase_; }
  // The winner's stick number in answering, and kNoWinner in ready and deciding.
  uint8_t winner() const { return winner_; }

  // The first PRESS of the round that the arbitration accepts: "deciding: from the first
  // PRESS of the round to the end of the next frame". It moves ready to deciding, and does
  // nothing in deciding and answering. The arbitration decides which PRESS counts.
  Effects first_press() {
    if (phase_ == Phase::kReady) {
      phase_ = Phase::kDeciding;
    }
    return Effects();
  }

  // The arbitration's winner, with the knob's seconds read at that moment: "answering: the
  // winner is chosen" and "the timer counts down from the knob's value read at the moment
  // the winner was chosen". now_ms is the host's clock in milliseconds, such as millis(). It
  // moves deciding to answering, and does nothing in ready and answering, or for a number
  // past the last stick.
  Effects winner_chosen(uint8_t stick, uint8_t seconds, uint32_t now_ms) {
    if (phase_ == Phase::kDeciding && stick < kMaxSticks) {
      phase_ = Phase::kAnswering;
      winner_ = stick;
      seconds_ = seconds;
      started_ms_ = now_ms;
    }
    return Effects();
  }

  // The correct button: "From answering: correct adds 1 to the winner's score and ends the
  // round". The three buttons do nothing in ready and deciding: "In ready, the three
  // buttons do nothing", and in deciding "the inputs are ignored".
  Effects correct() {
    Effects effects;
    if (phase_ == Phase::kAnswering) {
      effects.scored = winner_;
      end_round(effects);
    }
    return effects;
  }

  // The wrong button: "wrong ends the round".
  Effects wrong() {
    Effects effects;
    if (phase_ == Phase::kAnswering) {
      end_round(effects);
    }
    return effects;
  }

  // The cancel button: "cancel ends the round".
  Effects cancel() {
    Effects effects;
    if (phase_ == Phase::kAnswering) {
      end_round(effects);
    }
    return effects;
  }

  // The host's clock, read on each pass of the sketch's loop: in answering, once the
  // countdown reaches 0, "the timer reaching 0 plays the wrong-answer tone and ends the
  // round". It does nothing in ready and deciding, and before the countdown reaches 0.
  Effects tick(uint32_t now_ms) {
    Effects effects;
    if (phase_ == Phase::kAnswering && elapsed_ms(now_ms) >= countdown_ms()) {
      effects.wrong_tone = true;
      end_round(effects);
    }
    return effects;
  }

  // The whole seconds left of the countdown at now_ms, rounded up, so that the screen shows
  // 1 until the countdown reaches 0; 0 in ready and deciding.
  uint8_t seconds_left(uint32_t now_ms) const {
    if (phase_ != Phase::kAnswering) {
      return 0;
    }
    const uint32_t elapsed = elapsed_ms(now_ms);
    const uint32_t countdown = countdown_ms();
    if (elapsed >= countdown) {
      return 0;
    }
    return static_cast<uint8_t>((countdown - elapsed + 999u) / 1000u);
  }

  // Writes the screen's first line at now_ms, with its ending zero, to line, and returns its
  // length, at most kLineWidth. Ready shows "Press to answer" and the round, as
  // "Press to answer #12". Deciding shows the line of ready, since the design sets the line
  // of ready and of answering alone. Answering shows "Stick 17" and the seconds left, as
  // "Stick 17: 45 s left".
  size_t first_line(uint32_t now_ms, char (&line)[kLineWidth + 1]) const {
    size_t at = 0;
    if (phase_ == Phase::kAnswering) {
      at = append_text(line, at, "Stick ");
      at = append_number(line, at, winner_);
      at = append_text(line, at, ": ");
      at = append_number(line, at, seconds_left(now_ms));
      at = append_text(line, at, " s left");
    } else {
      at = append_text(line, at, "Press to answer #");
      at = append_number(line, at, round_);
    }
    line[at] = '\0';
    return at;
  }

 private:
  // The milliseconds since the winner was chosen. The unsigned subtraction stays right
  // across the wrap of a 32-bit clock, such as millis() after about 49.7 days.
  uint32_t elapsed_ms(uint32_t now_ms) const { return now_ms - started_ms_; }

  uint32_t countdown_ms() const { return static_cast<uint32_t>(seconds_) * 1000u; }

  // "once a round ends (correct, wrong, cancel or timeout), the host opens the next round,
  // its number one higher, in ready, and the next beacon carries it". The number after 255
  // is 0, as a beacon's round is "counting up and wrapping at 255" (packets.h).
  void end_round(Effects& effects) {
    effects.round_ended = true;
    round_ = static_cast<uint8_t>(round_ + 1);
    phase_ = Phase::kReady;
    winner_ = kNoWinner;
    seconds_ = 0;
    started_ms_ = 0;
  }

  // Appends text to line from at, up to kLineWidth characters, and returns the position
  // after it.
  static size_t append_text(char* line, size_t at, const char* text) {
    while (*text != '\0' && at < kLineWidth) {
      line[at] = *text;
      ++at;
      ++text;
    }
    return at;
  }

  // Appends value's decimal digits, at most 3, to line from at, up to kLineWidth
  // characters, and returns the position after them.
  static size_t append_number(char* line, size_t at, uint8_t value) {
    char digits[3];
    size_t count = 0;
    unsigned rest = value;
    do {
      digits[count] = static_cast<char>('0' + rest % 10u);
      ++count;
      rest /= 10u;
    } while (rest != 0u);
    while (count > 0 && at < kLineWidth) {
      --count;
      line[at] = digits[count];
      ++at;
    }
    return at;
  }

  uint8_t round_ = 0;
  Phase phase_ = Phase::kReady;
  uint8_t winner_ = kNoWinner;
  // The knob's seconds read as the winner was chosen, and the clock at that moment.
  uint8_t seconds_ = 0;
  uint32_t started_ms_ = 0;
};

}  // namespace answer_sticks

#endif  // ANSWER_STICKS_HOST_LOGIC_H
