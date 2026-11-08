// Tests of the scoreboard: the order of the three highest scores, ties, fewer than three
// sticks with a score, and the lines the host shows on rows 2 to 4 of its LCD.
#include <scoreboard.h>

#include <stdint.h>

#include "test_suites.h"

namespace {

using answer_sticks::Scoreboard;
using answer_sticks::kLineChars;
using answer_sticks::kScoreRows;
using answer_sticks_test::check_line;
using answer_sticks_test::check_number;

// A line of 20 spaces, for a row without a stick to show.
const char kBlank[] = "                    ";

// Adds `points` points to one stick's score.
void add_points(Scoreboard& board, uint8_t stick, unsigned long points) {
  for (unsigned long i = 0; i < points; ++i) {
    board.add_point(stick);
  }
}

// Checks the line of one row against `expected`, which holds kLineChars characters.
int check_row(const Scoreboard& board, uint8_t row, const char* expected, const char* what) {
  char line[kLineChars + 1];
  board.format_row(row, line);
  return check_line(line, expected, what);
}

int run() {
  int failures = 0;

  // A new scoreboard: every score is 0, no stick leads, and every row is blank.
  {
    Scoreboard board;
    uint8_t sticks[kScoreRows] = {};
    failures += check_number(board.score(0), 0, "new board: stick 0 has 0");
    failures += check_number(board.score(63), 0, "new board: stick 63 has 0");
    failures += check_number(board.leaders(sticks), 0, "new board: no leaders");
    failures += check_row(board, 0, kBlank, "new board: row 0 is blank");
    failures += check_row(board, 1, kBlank, "new board: row 1 is blank");
    failures += check_row(board, 2, kBlank, "new board: row 2 is blank");
  }

  // The three highest scores, highest first, whatever order the sticks scored in; a fourth
  // stick with a lower score isn't shown.
  {
    Scoreboard board;
    add_points(board, 4, 12);
    add_points(board, 50, 7);
    add_points(board, 9, 30);
    add_points(board, 2, 5);
    uint8_t sticks[kScoreRows] = {};
    failures += check_number(board.score(4), 12, "order: stick 4 has 12");
    failures += check_number(board.leaders(sticks), 3, "order: three leaders of four scorers");
    failures += check_number(sticks[0], 9, "order: stick 9 (30) first");
    failures += check_number(sticks[1], 4, "order: stick 4 (12) second");
    failures += check_number(sticks[2], 50, "order: stick 50 (7) third");
    failures += check_row(board, 0, "1. Stick 9    30" "    ", "order: row 0");
    failures += check_row(board, 1, "2. Stick 4    12" "    ", "order: row 1");
    failures += check_row(board, 2, "3. Stick 50    7" "    ", "order: row 2");
  }

  // A tie at the top goes to the lower stick number, whatever order the sticks scored in.
  {
    Scoreboard board;
    add_points(board, 60, 5);
    add_points(board, 7, 5);
    add_points(board, 3, 5);
    add_points(board, 1, 5);
    uint8_t sticks[kScoreRows] = {};
    failures += check_number(board.leaders(sticks), 3, "tie of four: three leaders");
    failures += check_number(sticks[0], 1, "tie of four: stick 1 first");
    failures += check_number(sticks[1], 3, "tie of four: stick 3 second");
    failures += check_number(sticks[2], 7, "tie of four: stick 7 third");
    failures += check_row(board, 0, "1. Stick 1     5" "    ", "tie of four: row 0");
    failures += check_row(board, 1, "2. Stick 3     5" "    ", "tie of four: row 1");
    failures += check_row(board, 2, "3. Stick 7     5" "    ", "tie of four: row 2");
  }

  // A tie below the top goes to the lower stick number too.
  {
    Scoreboard board;
    add_points(board, 20, 4);
    add_points(board, 10, 9);
    add_points(board, 5, 4);
    uint8_t sticks[kScoreRows] = {};
    failures += check_number(board.leaders(sticks), 3, "tie below the top: three leaders");
    failures += check_number(sticks[0], 10, "tie below the top: stick 10 (9) first");
    failures += check_number(sticks[1], 5, "tie below the top: stick 5 (4) second");
    failures += check_number(sticks[2], 20, "tie below the top: stick 20 (4) third");
  }

  // Fewer than three sticks with a score: the rows past them are blank, and a stick without
  // points isn't listed.
  {
    Scoreboard board;
    uint8_t sticks[kScoreRows] = {};
    add_points(board, 0, 1);
    failures += check_number(board.leaders(sticks), 1, "one scorer: one leader");
    failures += check_number(sticks[0], 0, "one scorer: stick 0 first");
    failures += check_row(board, 0, "1. Stick 0     1" "    ", "one scorer: row 0");
    failures += check_row(board, 1, kBlank, "one scorer: row 1 is blank");
    failures += check_row(board, 2, kBlank, "one scorer: row 2 is blank");

    add_points(board, 63, 2);
    failures += check_number(board.leaders(sticks), 2, "two scorers: two leaders");
    failures += check_number(sticks[0], 63, "two scorers: stick 63 (2) first");
    failures += check_number(sticks[1], 0, "two scorers: stick 0 (1) second");
    failures += check_row(board, 0, "1. Stick 63    2" "    ", "two scorers: row 0");
    failures += check_row(board, 1, "2. Stick 0     1" "    ", "two scorers: row 1");
    failures += check_row(board, 2, kBlank, "two scorers: row 2 is blank");
  }

  // A stick number of 64 or more changes nothing.
  {
    Scoreboard board;
    uint8_t sticks[kScoreRows] = {};
    board.add_point(64);
    board.add_point(255);
    failures += check_number(board.score(64), 0, "stick 64: score is 0");
    failures += check_number(board.leaders(sticks), 0, "sticks 64 and 255: no leaders");
  }

  // Five-digit scores: one ends at the 16th character after a one-digit stick number, and one
  // moves right to keep a space after a two-digit stick number. A score stays at 65535
  // instead of going back to 0.
  {
    Scoreboard board;
    add_points(board, 4, 10000);
    add_points(board, 63, 65536);
    failures += check_number(board.score(63), 65535, "long scores: stick 63 stays at 65535");
    failures += check_row(board, 0, "1. Stick 63 65535" "   ", "long scores: row 0");
    failures += check_row(board, 1, "2. Stick 4 10000" "    ", "long scores: row 1");
  }

  // A reset sets every score back to 0.
  {
    Scoreboard board;
    uint8_t sticks[kScoreRows] = {};
    add_points(board, 12, 3);
    board.reset();
    failures += check_number(board.score(12), 0, "reset: stick 12 has 0");
    failures += check_number(board.leaders(sticks), 0, "reset: no leaders");
  }

  return failures;
}

answer_sticks_test::AddSuite add_scoreboard_suite("scoreboard", run);

}  // namespace
