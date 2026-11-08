// The test suites of the library: each test file registers its suite with an AddSuite object,
// and main() in answer_sticks_test.cpp runs every suite registered, so that a new test file
// doesn't change main(). Each check prints PASS or FAIL with what it checked.
#ifndef ANSWER_STICKS_TEST_SUITES_H
#define ANSWER_STICKS_TEST_SUITES_H

#include <cstdio>
#include <cstring>
#include <vector>

namespace answer_sticks_test {

// A suite runs its checks and returns how many of them failed.
using Suite = int (*)();

struct NamedSuite {
  const char* name;
  Suite run;
};

// The suites registered so far, held in a function's static so that a test file's AddSuite
// finds it constructed whatever order the files' objects start in.
inline std::vector<NamedSuite>& suites() {
  static std::vector<NamedSuite> all;
  return all;
}

// Registers a suite as the program starts, before main() runs.
struct AddSuite {
  AddSuite(const char* name, Suite run) { suites().push_back({name, run}); }
};

// Checks a number: prints PASS, or FAIL with both values, and returns 1 where it failed.
inline int check_number(unsigned long actual, unsigned long expected, const char* what) {
  if (actual == expected) {
    std::printf("PASS: %s\n", what);
    return 0;
  }
  std::printf("FAIL: %s: got %lu, expected %lu\n", what, actual, expected);
  return 1;
}

// Checks a line of text: prints PASS, or FAIL with both lines in quotes so that their spaces
// show, and returns 1 where it failed.
inline int check_line(const char* actual, const char* expected, const char* what) {
  if (std::strcmp(actual, expected) == 0) {
    std::printf("PASS: %s\n", what);
    return 0;
  }
  std::printf("FAIL: %s: got \"%s\", expected \"%s\"\n", what, actual, expected);
  return 1;
}

}  // namespace answer_sticks_test

#endif  // ANSWER_STICKS_TEST_SUITES_H
