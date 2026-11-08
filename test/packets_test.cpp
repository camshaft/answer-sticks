// Tests of the BEACON and PRESS packets and their encoding, built with the computer's own
// C++ compiler and run without a board. The checks run from a static object's constructor,
// before main, so that this file adds its cases without a change to the main of
// answer_sticks_test.cpp: the constructor prints each check, and where any check fails it
// exits with 1, as main does for its own checks.
#include <packets.h>

#include <cstdio>
#include <cstdlib>

namespace {

using answer_sticks::Beacon;
using answer_sticks::Phase;
using answer_sticks::Press;
using answer_sticks::decode;
using answer_sticks::encode;
using answer_sticks::is_heard;
using answer_sticks::kBeaconSize;
using answer_sticks::kNoWinner;
using answer_sticks::kPressSize;
using answer_sticks::set_heard;

int failures = 0;

void check(bool ok, const char* what) {
  std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) {
    ++failures;
  }
}

bool same_bytes(const uint8_t* a, const uint8_t* b, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

bool same_beacon(const Beacon& a, const Beacon& b) {
  return a.round == b.round && a.phase == b.phase && a.winner == b.winner &&
         same_bytes(a.heard, b.heard, sizeof a.heard);
}

bool same_press(const Press& a, const Press& b) {
  return a.stick == b.stick && a.round == b.round && a.age_us == b.age_us;
}

void sizes() {
  check(kBeaconSize == 12, "a BEACON is 12 bytes");
  check(kPressSize == 7, "a PRESS is 7 bytes");
  uint8_t out[32] = {};
  check(encode(Beacon(), out, sizeof out) == 12, "encode writes 12 bytes of a BEACON");
  check(encode(Press(), out, sizeof out) == 7, "encode writes 7 bytes of a PRESS");
}

void heard_bits() {
  Beacon beacon;
  set_heard(beacon, 0);
  const uint8_t stick_0[8] = {0x01, 0, 0, 0, 0, 0, 0, 0};
  check(same_bytes(beacon.heard, stick_0, 8), "stick 0's bit is bit 0 of heard[0]");
  check(is_heard(beacon, 0), "stick 0's bit reads as set");
  check(!is_heard(beacon, 1), "stick 1's bit reads as clear beside stick 0's");
  check(!is_heard(beacon, 63), "stick 63's bit reads as clear beside stick 0's");

  set_heard(beacon, 63);
  const uint8_t sticks_0_63[8] = {0x01, 0, 0, 0, 0, 0, 0, 0x80};
  check(same_bytes(beacon.heard, sticks_0_63, 8), "stick 63's bit is bit 7 of heard[7]");
  check(is_heard(beacon, 63), "stick 63's bit reads as set");
  check(!is_heard(beacon, 62), "stick 62's bit reads as clear beside stick 63's");

  set_heard(beacon, 64);
  check(same_bytes(beacon.heard, sticks_0_63, 8), "a number past stick 63 leaves heard as it was");
  check(!is_heard(beacon, 64), "a number past stick 63 reads as not heard");
}

void beacon_both_directions() {
  Beacon beacon;
  beacon.round = 200;
  beacon.phase = Phase::kDeciding;
  beacon.winner = 63;
  set_heard(beacon, 0);
  set_heard(beacon, 63);
  const uint8_t bytes[12] = {1, 200, 1, 63, 0x01, 0, 0, 0, 0, 0, 0, 0x80};

  uint8_t out[12] = {};
  check(encode(beacon, out, sizeof out) == 12 && same_bytes(out, bytes, 12),
        "a BEACON encodes as type, round, phase, winner and heard[8]");

  Beacon read;
  check(decode(bytes, sizeof bytes, read) && same_beacon(read, beacon),
        "a BEACON's 12 bytes decode to its fields");

  Beacon none;
  none.phase = Phase::kAnswering;
  check(encode(none, out, sizeof out) == 12 && out[3] == 255,
        "a BEACON without a winner encodes winner 255");
  Beacon none_read;
  none_read.winner = 0;
  check(decode(out, sizeof out, none_read) && none_read.winner == kNoWinner,
        "winner 255 decodes as no winner");
}

void phases() {
  const Phase all[3] = {Phase::kReady, Phase::kDeciding, Phase::kAnswering};
  for (uint8_t value = 0; value < 3; ++value) {
    Beacon beacon;
    beacon.phase = all[value];
    uint8_t out[12] = {};
    encode(beacon, out, sizeof out);
    Beacon read;
    read.phase = all[(value + 1) % 3];
    char what[64];
    std::snprintf(what, sizeof what, "phase %u encodes as %u and decodes back",
                  static_cast<unsigned>(value), static_cast<unsigned>(value));
    check(out[2] == value && decode(out, sizeof out, read) && read.phase == all[value], what);
  }
}

void press_both_directions() {
  Press press;
  press.stick = 63;
  press.round = 255;
  press.age_us = 0x12345678u;
  const uint8_t bytes[7] = {2, 63, 255, 0x78, 0x56, 0x34, 0x12};

  uint8_t out[7] = {};
  check(encode(press, out, sizeof out) == 7 && same_bytes(out, bytes, 7),
        "a PRESS encodes as type, stick, round and age_us little-endian");

  Press read;
  check(decode(bytes, sizeof bytes, read) && same_press(read, press),
        "a PRESS's 7 bytes decode to its fields");

  Press first;
  first.age_us = 4100;
  check(encode(first, out, sizeof out) == 7 && decode(out, sizeof out, read) &&
            same_press(read, first),
        "a PRESS of stick 0 decodes back to its fields");
}

void short_buffers() {
  uint8_t out[12] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
  const uint8_t untouched[12] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
  check(encode(Beacon(), out, 11) == 0 && same_bytes(out, untouched, 12),
        "encode doesn't write a BEACON into 11 bytes");
  check(encode(Press(), out, 6) == 0 && same_bytes(out, untouched, 12),
        "encode doesn't write a PRESS into 6 bytes");
}

void refused_beacons() {
  const uint8_t good[13] = {1, 9, 2, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  Beacon before;
  before.round = 5;
  set_heard(before, 3);
  Beacon read = before;

  check(decode(good, 12, read) && read.round == 9, "the BEACON the refusals change decodes");
  read = before;
  check(!decode(good, 11, read) && same_beacon(read, before),
        "11 bytes aren't a BEACON, and leave the beacon as it was");
  check(!decode(good, 13, read) && same_beacon(read, before), "13 bytes aren't a BEACON");

  uint8_t bytes[12];
  for (size_t i = 0; i < 12; ++i) {
    bytes[i] = good[i];
  }
  bytes[0] = 2;
  check(!decode(bytes, 12, read) && same_beacon(read, before), "a PRESS's type isn't a BEACON");
  bytes[0] = 1;
  bytes[2] = 3;
  check(!decode(bytes, 12, read) && same_beacon(read, before), "phase 3 isn't a BEACON");
  bytes[2] = 2;
  bytes[3] = 64;
  check(!decode(bytes, 12, read) && same_beacon(read, before),
        "winner 64 is neither a stick nor none");
  bytes[3] = 254;
  check(!decode(bytes, 12, read) && same_beacon(read, before),
        "winner 254 is neither a stick nor none");
  bytes[3] = 63;
  check(decode(bytes, 12, read) && read.winner == 63, "winner 63 decodes");
}

void refused_presses() {
  const uint8_t good[8] = {2, 63, 4, 1, 0, 0, 0, 0};
  Press before;
  before.stick = 5;
  before.age_us = 77;
  Press read = before;

  check(decode(good, 7, read) && read.stick == 63, "the PRESS the refusals change decodes");
  read = before;
  check(!decode(good, 6, read) && same_press(read, before),
        "6 bytes aren't a PRESS, and leave the press as it was");
  check(!decode(good, 8, read) && same_press(read, before), "8 bytes aren't a PRESS");

  uint8_t bytes[7];
  for (size_t i = 0; i < 7; ++i) {
    bytes[i] = good[i];
  }
  bytes[0] = 1;
  check(!decode(bytes, 7, read) && same_press(read, before), "a BEACON's type isn't a PRESS");
  bytes[0] = 2;
  bytes[1] = 64;
  check(!decode(bytes, 7, read) && same_press(read, before), "stick 64 isn't a PRESS");
}

struct PacketTests {
  PacketTests() {
    sizes();
    heard_bits();
    beacon_both_directions();
    phases();
    press_both_directions();
    short_buffers();
    refused_beacons();
    refused_presses();
    if (failures != 0) {
      std::printf("FAIL: %d packet checks failed\n", failures);
      std::exit(1);
    }
  }
};

PacketTests packet_tests;

}  // namespace
