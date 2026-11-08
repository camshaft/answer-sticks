// The knob that sets the answer time on the host: a potentiometer whose wiper the host reads
// with analogRead(). Plain C++ with no hardware calls, so that the tests build and run it with
// the computer's compiler.
#ifndef ANSWER_STICKS_KNOB_H
#define ANSWER_STICKS_KNOB_H

#include <stdint.h>

namespace answer_sticks {

// The highest knob reading: the core's analogRead() reads 10 bits unless a sketch sets another
// resolution, 0 to 1023 [core wiring_analog_nRF52.c:39, `readResolution = 10`].
constexpr uint32_t kKnobMaxReading = 1023;

// The answer time the knob sets, in whole seconds: 3 at one end of the knob and 60 at the
// other (the game's design; no data sheet sets them).
constexpr uint8_t kKnobMinSeconds = 3;
constexpr uint8_t kKnobMaxSeconds = 60;

// Maps a knob reading linearly to whole seconds: 0 to kKnobMinSeconds and kKnobMaxReading to
// kKnobMaxSeconds, rounding to the nearest second. A reading above kKnobMaxReading counts as
// kKnobMaxReading, so the answer stays in range whatever analogRead() returns.
//
// The rounding adds half of kKnobMaxReading, rounded down, before the division. A reading
// never maps to a whole second and a half: that would need 2 * reading * 57 to equal 1023
// times an odd number, and the first is even while the second is odd.
inline uint8_t knob_seconds(uint32_t reading) {
  if (reading > kKnobMaxReading) {
    reading = kKnobMaxReading;
  }
  const uint32_t span = kKnobMaxSeconds - kKnobMinSeconds;
  return static_cast<uint8_t>(kKnobMinSeconds +
                              (reading * span + kKnobMaxReading / 2) / kKnobMaxReading);
}

}  // namespace answer_sticks

#endif  // ANSWER_STICKS_KNOB_H
