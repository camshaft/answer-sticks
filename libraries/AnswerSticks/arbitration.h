// The time slots of the game's radio frame, the time of each press, and the host's choice of
// the first press of a round, as plain C++ with no hardware calls. The game's design sets
// each timing and rule below, quoted beside it; a value computed from them says so.
#ifndef ANSWER_STICKS_ARBITRATION_H
#define ANSWER_STICKS_ARBITRATION_H

#include <stdint.h>

#include "AnswerSticks.h"
#include "packets.h"

namespace answer_sticks {

// "A frame is a beacon slot of 600 us followed by 64 stick slots of 400 us, 26.2 ms in all."
constexpr uint32_t kBeaconSlotUs = 600;
constexpr uint32_t kStickSlotUs = 400;
constexpr uint32_t kFrameUs = kBeaconSlotUs + kStickSlotUs * kMaxSticks;
static_assert(kFrameUs == 26200, "the design sets a frame to 26.2 ms");

// "TIMER3 counts at 16 MHz, 62.5 ns a count, with PRESCALER 0 [PS 6.30 p774, 6.30.5 p783]",
// so the timer counts 16 times in a microsecond.
constexpr uint32_t kCountsPerUs = 16;

// The microseconds from the end of the beacon a stick received to the start of its slot: "A
// stick's slot starts 600 + 400 x stick microseconds after the end of the beacon it
// received." stick is a stick's number, 0 to kMaxSticks - 1.
constexpr uint32_t slot_offset_us(uint8_t stick) {
  return kBeaconSlotUs + kStickSlotUs * stick;
}
static_assert(slot_offset_us(kMaxSticks - 1) + kStickSlotUs == kFrameUs,
              "the last stick's slot ends at the end of the frame (computed)");

// The same offset in the timer's counts (computed).
constexpr uint32_t slot_offset_counts(uint8_t stick) {
  return slot_offset_us(stick) * kCountsPerUs;
}

// The timer's count at the start of stick's slot, where beacon_end is the count the timer
// captured at the beacon's END event. The design says that "both sketches set TIMER3's
// BITMODE register to 32 bits", so a count is 32 bits wide, and the sum wraps as the timer
// does.
constexpr uint32_t slot_start(uint32_t beacon_end, uint8_t stick) {
  return beacon_end + slot_offset_counts(stick);
}

// A press's time, in the timer's counts: "the start of that stick's slot in the current
// frame, minus `age_us`", where beacon_end is "the host's capture of its own beacon's END on
// TIMER3" in the frame in which the host received the PRESS. The difference wraps as the
// timer does.
constexpr uint32_t press_time(uint32_t beacon_end, uint8_t stick, uint32_t age_us) {
  return slot_start(beacon_end, stick) - age_us * kCountsPerUs;
}

// Whether count a comes before count b. The 32-bit timer wraps "after about 268 s", so a
// comes first where b is less than 2^31 counts, about 134 s, after a, counted forward from a
// across the wrap (computed). The presses of one round are a few frames apart.
constexpr bool before(uint32_t a, uint32_t b) {
  return a != b && ((b - a) & 0x80000000u) == 0;
}

// The host's round of presses: which PRESS packets it takes, the heard mask, the move to
// deciding, and the choice of the winner. The round's number, phase, winner and heard mask
// are the fields of the beacon the host sends next.
class Arbiter {
 public:
  // A round numbered round, in ready, with no stick heard and no winner.
  explicit Arbiter(uint8_t round = 0) { beacon_.round = round; }

  // The round's number, phase, winner and heard mask, as the host's next beacon carries
  // them.
  const Beacon& beacon() const { return beacon_; }

  // Takes press, which the host received in the frame whose beacon's END it captured at
  // beacon_end, and returns true; returns false, and changes nothing, where the host doesn't
  // take it. "the host takes it only where that is the current round and the phase is ready
  // or deciding", and "the host counts a stick once in a round however many of its PRESS
  // packets it receives", so a second PRESS of a stick in the round isn't taken. Taking a
  // PRESS sets the stick's bit in the heard mask: "a bit set for each stick whose PRESS of
  // this round the host has received". "the first PRESS of a ready round, received in frame
  // F, moves the host to deciding".
  bool take(const Press& press, uint32_t beacon_end) {
    if (beacon_.phase == Phase::kAnswering || press.round != beacon_.round ||
        press.stick >= kMaxSticks || is_heard(beacon_, press.stick)) {
      return false;
    }
    set_heard(beacon_, press.stick);
    const uint32_t time = press_time(beacon_end, press.stick, press.age_us);
    if (beacon_.phase == Phase::kReady) {
      beacon_.phase = Phase::kDeciding;
      frames_ended_ = 0;
      best_stick_ = press.stick;
      best_time_ = time;
    } else if (before(time, best_time_) || (time == best_time_ && press.stick < best_stick_)) {
      // "ties going to the lower stick number".
      best_stick_ = press.stick;
      best_time_ = time;
    }
    return true;
  }

  // Ends the current frame, and returns true where this end chose the winner. "At the end of
  // frame F+1 the host picks the PRESS with the earliest computed time, ties going to the
  // lower stick number, sets `winner` and moves to answering", where F is the frame in which
  // the round's first PRESS arrived: the second frame that ends in deciding.
  bool end_frame() {
    if (beacon_.phase != Phase::kDeciding) {
      return false;
    }
    ++frames_ended_;
    if (frames_ended_ < 2) {
      return false;
    }
    beacon_.winner = best_stick_;
    beacon_.phase = Phase::kAnswering;
    return true;
  }

  // Opens the next round: "the host opens the next round, its number one higher, in ready",
  // with no stick heard and no winner. Its number wraps from 255 to 0: "counting up and
  // wrapping at 255".
  void open_next() {
    const uint8_t next = static_cast<uint8_t>(beacon_.round + 1);
    *this = Arbiter(next);
  }

 private:
  Beacon beacon_;
  // The frames that ended since the round's first PRESS: 1 at the end of frame F, 2 at the
  // end of frame F+1.
  uint8_t frames_ended_ = 0;
  // The stick of the earliest press the host took in the round, and its time.
  uint8_t best_stick_ = kNoWinner;
  uint32_t best_time_ = 0;
};

}  // namespace answer_sticks

#endif  // ANSWER_STICKS_ARBITRATION_H
