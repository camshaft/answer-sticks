// The two packets of the game's radio protocol, BEACON and PRESS, and their encoding, as
// plain C++ with no hardware calls. The game's design sets each packet's fields, their
// widths and its size, quoted beside each value below, and says: "Each field is a
// fixed-width integer, little-endian, packed; the library encodes and decodes them".
#ifndef ANSWER_STICKS_PACKETS_H
#define ANSWER_STICKS_PACKETS_H

#include <stddef.h>
#include <stdint.h>

#include "AnswerSticks.h"

namespace answer_sticks {

// The first byte of each packet, its `type u8`, says which packet it is. The design sets
// the field and its width, and doesn't set its values: these two are this library's.
constexpr uint8_t kBeaconType = 1;
constexpr uint8_t kPressType = 2;

// A round's phase, as a beacon carries it: "its phase (ready, deciding or answering)". The
// design lists the three phases and doesn't number them: these numbers are this library's.
enum class Phase : uint8_t {
  kReady = 0,
  kDeciding = 1,
  kAnswering = 2,
};

// The beacon's winner where no stick has won: "the winner's stick number or 255 for none".
constexpr uint8_t kNoWinner = 255;

// The bytes of a beacon's `heard[8]`: "a 64-bit mask with a bit set for each stick whose
// PRESS of this round the host has received", one bit for each of the kMaxSticks sticks.
constexpr size_t kHeardBytes = 8;
static_assert(kHeardBytes * 8 == kMaxSticks, "the heard mask holds one bit for each stick");

// "BEACON {type u8, round u8, phase u8, winner u8, heard[8]}, 12 bytes".
constexpr size_t kBeaconSize = 1 + 1 + 1 + 1 + kHeardBytes;
static_assert(kBeaconSize == 12, "the design sets a BEACON's size to 12 bytes");

// "PRESS {type u8, stick u8, round u8, age_us u32}, 7 bytes".
constexpr size_t kPressSize = 1 + 1 + 1 + 4;
static_assert(kPressSize == 7, "the design sets a PRESS's size to 7 bytes");

// The beacon the host sends to every stick once a frame.
struct Beacon {
  // "the round's number, counting up and wrapping at 255".
  uint8_t round = 0;
  Phase phase = Phase::kReady;
  // "the winner's stick number or 255 for none": a stick number, 0 to 63, or kNoWinner.
  uint8_t winner = kNoWinner;
  // Bit (stick % 8) of byte (stick / 8) is set for each stick the host has heard this
  // round, so the 8 bytes are the 64-bit mask in little-endian order. set_heard and
  // is_heard read and write it.
  uint8_t heard[kHeardBytes] = {};
};

// The press a stick sends to the host in the stick's own slot.
struct Press {
  // "the stick's number (0 to 63)".
  uint8_t stick = 0;
  // "the round of the beacon it received".
  uint8_t round = 0;
  // "the microseconds from its button's edge to the start of its slot".
  uint32_t age_us = 0;
};

// Sets stick's bit in beacon's heard mask. A number past the last stick doesn't have a bit,
// and leaves the mask as it was.
inline void set_heard(Beacon& beacon, uint8_t stick) {
  if (stick >= kMaxSticks) {
    return;
  }
  beacon.heard[stick / 8] = static_cast<uint8_t>(beacon.heard[stick / 8] | (1u << (stick % 8)));
}

// Whether stick's bit is set in beacon's heard mask; false for a number past the last stick.
inline bool is_heard(const Beacon& beacon, uint8_t stick) {
  if (stick >= kMaxSticks) {
    return false;
  }
  return ((beacon.heard[stick / 8] >> (stick % 8)) & 1u) != 0;
}

// Writes beacon's kBeaconSize bytes to out and returns kBeaconSize. Where capacity is less
// than kBeaconSize, it doesn't write any byte and returns 0.
inline size_t encode(const Beacon& beacon, uint8_t* out, size_t capacity) {
  if (capacity < kBeaconSize) {
    return 0;
  }
  out[0] = kBeaconType;
  out[1] = beacon.round;
  out[2] = static_cast<uint8_t>(beacon.phase);
  out[3] = beacon.winner;
  for (size_t i = 0; i < kHeardBytes; ++i) {
    out[4 + i] = beacon.heard[i];
  }
  return kBeaconSize;
}

// Writes press's kPressSize bytes to out, age_us little-endian, and returns kPressSize.
// Where capacity is less than kPressSize, it doesn't write any byte and returns 0.
inline size_t encode(const Press& press, uint8_t* out, size_t capacity) {
  if (capacity < kPressSize) {
    return 0;
  }
  out[0] = kPressType;
  out[1] = press.stick;
  out[2] = press.round;
  out[3] = static_cast<uint8_t>(press.age_us & 0xFFu);
  out[4] = static_cast<uint8_t>((press.age_us >> 8) & 0xFFu);
  out[5] = static_cast<uint8_t>((press.age_us >> 16) & 0xFFu);
  out[6] = static_cast<uint8_t>((press.age_us >> 24) & 0xFFu);
  return kPressSize;
}

// Reads the BEACON in in's length bytes into beacon and returns true. It returns false,
// and leaves beacon as it was, where the bytes aren't a BEACON this library reads: their
// length isn't kBeaconSize, their type isn't kBeaconType, their phase isn't one of the
// three, or their winner is neither a stick's number nor kNoWinner.
inline bool decode(const uint8_t* in, size_t length, Beacon& beacon) {
  if (length != kBeaconSize || in[0] != kBeaconType) {
    return false;
  }
  if (in[2] > static_cast<uint8_t>(Phase::kAnswering)) {
    return false;
  }
  if (in[3] >= kMaxSticks && in[3] != kNoWinner) {
    return false;
  }
  beacon.round = in[1];
  beacon.phase = static_cast<Phase>(in[2]);
  beacon.winner = in[3];
  for (size_t i = 0; i < kHeardBytes; ++i) {
    beacon.heard[i] = in[4 + i];
  }
  return true;
}

// Reads the PRESS in in's length bytes into press and returns true. It returns false, and
// leaves press as it was, where the bytes aren't a PRESS this library reads: their length
// isn't kPressSize, their type isn't kPressType, or their stick's number is past the last
// stick.
inline bool decode(const uint8_t* in, size_t length, Press& press) {
  if (length != kPressSize || in[0] != kPressType) {
    return false;
  }
  if (in[1] >= kMaxSticks) {
    return false;
  }
  press.stick = in[1];
  press.round = in[2];
  press.age_us = static_cast<uint32_t>(in[3]) | (static_cast<uint32_t>(in[4]) << 8) |
                 (static_cast<uint32_t>(in[5]) << 16) | (static_cast<uint32_t>(in[6]) << 24);
  return true;
}

}  // namespace answer_sticks

#endif  // ANSWER_STICKS_PACKETS_H
