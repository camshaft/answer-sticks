// Tests of the AnswerSticks library, built with the computer's own C++ compiler and run
// without a board. The program prints each check and exits with 1 where any check fails.
// Each other test file registers its suite as test_suites.h says, and main() runs them all.
#include <AnswerSticks.h>

#include <cstdio>

#include "test_suites.h"

int main() {
  int failures = 0;

  if (answer_sticks::kMaxSticks == 64) {
    std::printf("PASS: kMaxSticks is 64\n");
  } else {
    std::printf("FAIL: kMaxSticks is %u, expected 64\n",
                static_cast<unsigned>(answer_sticks::kMaxSticks));
    ++failures;
  }

  for (const answer_sticks_test::NamedSuite& suite : answer_sticks_test::suites()) {
    std::printf("Suite: %s\n", suite.name);
    failures += suite.run();
  }

  return failures == 0 ? 0 : 1;
}
