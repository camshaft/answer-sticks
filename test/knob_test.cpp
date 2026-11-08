// Tests of the knob mapping: its two ends, its middle, its rounding to the nearest second, and
// readings above the 10-bit range.
#include <knob.h>

#include "test_suites.h"

namespace {

using answer_sticks::knob_seconds;
using answer_sticks_test::check_number;

int run() {
  int failures = 0;

  // The two ends of the knob.
  failures += check_number(knob_seconds(0), 3, "knob reading 0 maps to 3 s");
  failures += check_number(knob_seconds(1023), 60, "knob reading 1023 maps to 60 s");

  // The middle: the middle of 0 to 1023 is 511.5, and of 3 to 60 s is 31.5 s. 511 maps to
  // 31.47 s and 512 to 31.53 s.
  failures += check_number(knob_seconds(511), 31, "knob reading 511 maps to 31 s");
  failures += check_number(knob_seconds(512), 32, "knob reading 512 maps to 32 s");

  // Rounding to the nearest second: 8 maps to 3.45 s and 9 to 3.50 s, which a division that
  // cut off the fraction would show as 3 s.
  failures += check_number(knob_seconds(8), 3, "knob reading 8 maps to 3 s");
  failures += check_number(knob_seconds(9), 4, "knob reading 9 maps to 4 s");

  // Readings above 1023 count as 1023: the 12-bit and the 14-bit highest, and the highest
  // uint32_t, whose product with 57 would overflow.
  failures += check_number(knob_seconds(4095), 60, "knob reading 4095 maps to 60 s");
  failures += check_number(knob_seconds(16383), 60, "knob reading 16383 maps to 60 s");
  failures += check_number(knob_seconds(0xFFFFFFFFu), 60, "knob reading 0xFFFFFFFF maps to 60 s");

  return failures;
}

answer_sticks_test::AddSuite add_knob_suite("knob", run);

}  // namespace
