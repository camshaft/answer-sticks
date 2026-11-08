// The host's scoreboard: a score for each stick, kept in RAM from the moment the host's box is
// switched on, and the lines that show the three highest scores on rows 2 to 4 of the host's
// LCD. Plain C++ with no hardware calls, so that the tests build and run it with the
// computer's compiler.
#ifndef ANSWER_STICKS_SCOREBOARD_H
#define ANSWER_STICKS_SCOREBOARD_H

#include <stdint.h>

#include "AnswerSticks.h"

namespace answer_sticks {

// How many scores the LCD shows: the three highest, on rows 2 to 4 (the game's design).
constexpr uint8_t kScoreRows = 3;

// The characters of one LCD line: the host's LCD, the Newhaven NHD-0420H1Z-FL-GBW-33V3, is a
// 4x20 module, 4 lines of 20 characters.
constexpr uint8_t kLineChars = 20;

// The highest score a counter holds, the largest uint16_t: a score that reaches it stays there.
constexpr uint16_t kMaxScore = 0xFFFF;

// Where a score's last digit sits on its line, counted from 0, as in "1. Stick 4    12" (the
// game's design).
constexpr uint8_t kScoreEnd = 15;

class Scoreboard {
 public:
  Scoreboard() { reset(); }

  // Sets every stick's score to 0, as when the host's box is switched on.
  void reset() {
    for (uint8_t stick = 0; stick < kMaxSticks; ++stick) {
      scores_[stick] = 0;
    }
  }

  // Adds one to a stick's score. A stick number of kMaxSticks or more changes nothing, and a
  // score at kMaxScore stays there instead of going back to 0.
  void add_point(uint8_t stick) {
    if (stick < kMaxSticks && scores_[stick] < kMaxScore) {
      ++scores_[stick];
    }
  }

  // A stick's score, or 0 for a stick number of kMaxSticks or more.
  uint16_t score(uint8_t stick) const { return stick < kMaxSticks ? scores_[stick] : 0; }

  // Writes the sticks with the highest scores above 0 into `sticks`, highest first, a tie
  // going to the lower stick number, and returns how many it wrote: kScoreRows, or fewer where
  // fewer sticks have scored.
  uint8_t leaders(uint8_t sticks[kScoreRows]) const {
    uint8_t count = 0;
    while (count < kScoreRows) {
      uint8_t best = kMaxSticks;
      for (uint8_t stick = 0; stick < kMaxSticks; ++stick) {
        if (scores_[stick] == 0 || listed(sticks, count, stick)) {
          continue;
        }
        // Only a higher score replaces the best so far, and the sticks come in rising order,
        // so a tie keeps the lower stick number.
        if (best == kMaxSticks || scores_[stick] > scores_[best]) {
          best = stick;
        }
      }
      if (best == kMaxSticks) {
        break;
      }
      sticks[count] = best;
      ++count;
    }
    return count;
  }

  // Writes the line of one row into `line`: kLineChars characters and a NUL. Row 0 shows the
  // highest score, as "1. Stick 4    12": the rank, the stick number, and the score, whose last
  // digit sits at kScoreEnd, or further right where the score needs the room to keep a space
  // after the stick number, then spaces to the end of the line. A row without a stick to show
  // is all spaces.
  void format_row(uint8_t row, char line[kLineChars + 1]) const {
    for (uint8_t i = 0; i < kLineChars; ++i) {
      line[i] = ' ';
    }
    line[kLineChars] = '\0';
    uint8_t sticks[kScoreRows] = {};
    if (row >= leaders(sticks)) {
      return;
    }
    const char prefix[] = ". Stick ";
    uint8_t pos = 0;
    line[pos] = static_cast<char>('1' + row);
    ++pos;
    for (uint8_t i = 0; prefix[i] != '\0'; ++i) {
      line[pos] = prefix[i];
      ++pos;
    }
    pos = write_number(line, pos, sticks[row]);
    const uint16_t points = scores_[sticks[row]];
    uint8_t start = static_cast<uint8_t>(kScoreEnd + 1 - count_digits(points));
    if (start < pos + 1) {
      start = static_cast<uint8_t>(pos + 1);
    }
    write_number(line, start, points);
  }

 private:
  // Whether `stick` is among the first `count` entries of `sticks`.
  static bool listed(const uint8_t* sticks, uint8_t count, uint8_t stick) {
    for (uint8_t i = 0; i < count; ++i) {
      if (sticks[i] == stick) {
        return true;
      }
    }
    return false;
  }

  // How many decimal digits `value` has.
  static uint8_t count_digits(uint16_t value) {
    uint8_t digits = 1;
    while (value >= 10) {
      value = static_cast<uint16_t>(value / 10);
      ++digits;
    }
    return digits;
  }

  // Writes `value` in decimal at line[pos] and returns the position after its last digit.
  static uint8_t write_number(char* line, uint8_t pos, uint16_t value) {
    const uint8_t digits = count_digits(value);
    for (uint8_t i = digits; i > 0; --i) {
      line[pos + i - 1] = static_cast<char>('0' + value % 10);
      value = static_cast<uint16_t>(value / 10);
    }
    return static_cast<uint8_t>(pos + digits);
  }

  uint16_t scores_[kMaxSticks];
};

}  // namespace answer_sticks

#endif  // ANSWER_STICKS_SCOREBOARD_H
