// Tests of the AnswerSticks library, built with the computer's own C++ compiler and run
// without a board. The program prints each check and exits with 1 where any check fails.
#include <AnswerSticks.h>

#include <cstdio>

int main() {
  int failures = 0;

  if (answer_sticks::kMaxSticks == 64) {
    std::printf("PASS: kMaxSticks is 64\n");
  } else {
    std::printf("FAIL: kMaxSticks is %u, expected 64\n",
                static_cast<unsigned>(answer_sticks::kMaxSticks));
    ++failures;
  }

  return failures == 0 ? 0 : 1;
}
