// AnswerSticks: the game logic that the host and stick sketches share, written as plain
// C++ with no hardware calls, so that the tests build and run it with the computer's own
// compiler. This first version holds only the number of sticks one host plays with.
#ifndef ANSWER_STICKS_H
#define ANSWER_STICKS_H

#include <stdint.h>

namespace answer_sticks {

// The most sticks one host plays with: each stick sends in a time slot of its own.
constexpr uint8_t kMaxSticks = 64;

}  // namespace answer_sticks

#endif  // ANSWER_STICKS_H
