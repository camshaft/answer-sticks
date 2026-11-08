// The stick's rules for each beacon it receives, and its guards for lost beacons, as plain
// C++ with no hardware calls. The sketch turns the receiver on, reads TIMER3, sends the
// PRESS, drives the light and puts the chip to sleep; StickLogic says which of these to do.
// The game's design states each rule and each value, quoted beside it below.
#ifndef ANSWER_STICKS_STICK_LOGIC_H
#define ANSWER_STICKS_STICK_LOGIC_H

#include <stdint.h>

#include "AnswerSticks.h"
#include "packets.h"

namespace answer_sticks {

// "As a guard for a host that is switched off, a stick waiting in deciding turns its light
// off and sleeps after 5 frames in a row without a beacon."
constexpr uint8_t kMissedBeaconsBeforeSleep = 5;

// "The winner in answering keeps its light on through lost beacons for up to 61 s without a
// beacon, the knob's longest answer time of 60 s and 1 s more, since its light is the
// player's cue to answer".
constexpr uint32_t kWinnerLightUs = 61000000;

// "The PRESS's `age_us` is TIMER3's count at the start of the slot, divided by 16."
constexpr uint32_t kTimerCountsPerUs = 16;

// The PRESS's age_us from TIMER3's count at the start of the stick's slot.
inline uint32_t age_us_from_timer(uint32_t timer_count) {
  return timer_count / kTimerCountsPerUs;
}

// What the stick does once StickLogic has read a beacon, or a frame without one. Where
// neither send_press nor sleep is set, the stick keeps receiving beacons.
struct StickAction {
  // Send StickLogic::press in the stick's slot of this frame.
  bool send_press = false;
  // Whether the stick's light is on.
  bool light = false;
  // Turn the light off and sleep.
  bool sleep = false;
};

// One stick's decisions from the beacons it receives, from its wake to its sleep. The
// sketch makes a new StickLogic on each wake.
class StickLogic {
 public:
  // stick is this stick's number, 0 to kMaxSticks - 1.
  explicit StickLogic(uint8_t stick) : stick_(stick) {}

  // "On wake, the stick turns its receiver on at the beacon address and acts on each beacon
  // it receives". now_us is a microsecond clock, which may wrap, read at the beacon's receipt.
  StickAction on_beacon(const Beacon& beacon, uint32_t now_us) {
    if (asleep_) {
      return action();
    }
    missed_ = 0;
    last_beacon_us_ = now_us;
    // "a round other than the one its PRESS named: light off, sleep". This rule comes first,
    // so that a stick doesn't press again in a round its button didn't press in.
    if (pressed_ && beacon.round != press_round_) {
      return go_to_sleep();
    }
    switch (beacon.phase) {
      case Phase::kReady:
      case Phase::kDeciding:
        // The winner's light: "any beacon that shows another phase or winner turns it off at
        // once".
        if (light_) {
          return go_to_sleep();
        }
        // "ready or deciding, with its bit clear in `heard`: send PRESS in its slot of this
        // frame". A PRESS the host didn't receive is sent again in the next frame.
        if (!is_heard(beacon, stick_)) {
          pressed_ = true;
          press_round_ = beacon.round;
          StickAction pressing = action();
          pressing.send_press = true;
          return pressing;
        }
        // "deciding, with its bit set: keep receiving beacons". A ready beacon with the bit
        // set is read the same way.
        return action();
      case Phase::kAnswering:
        // "answering, with itself as winner, in its round: light on, keep receiving beacons".
        // The round is the PRESS's round, since the check above comes first.
        if (pressed_ && beacon.winner == stick_) {
          light_ = true;
          return action();
        }
        // "answering with another winner, answering before it sent a PRESS": light off,
        // sleep. kNoWinner counts as another winner here.
        return go_to_sleep();
    }
    return action();
  }

  // Acts on a frame whose beacon the stick didn't receive, at now_us on the same clock.
  StickAction on_missed_beacon(uint32_t now_us) {
    if (light_) {
      // The winner keeps its light on "for up to 61 s without a beacon".
      if (static_cast<uint32_t>(now_us - last_beacon_us_) >= kWinnerLightUs) {
        return go_to_sleep();
      }
      return action();
    }
    // "after 5 frames in a row without a beacon". The design states this guard for deciding;
    // it holds here in each wait but the winner's: before the first beacon, and in ready.
    if (missed_ < kMissedBeaconsBeforeSleep) {
      ++missed_;
    }
    if (missed_ >= kMissedBeaconsBeforeSleep) {
      return go_to_sleep();
    }
    return action();
  }

  // The PRESS that on_beacon asked to send, from TIMER3's count at the start of the slot.
  Press press(uint32_t timer_count) const {
    Press packet;
    packet.stick = stick_;
    packet.round = press_round_;
    packet.age_us = age_us_from_timer(timer_count);
    return packet;
  }

  uint8_t stick() const { return stick_; }
  bool pressed() const { return pressed_; }
  uint8_t press_round() const { return press_round_; }
  bool light() const { return light_; }
  bool asleep() const { return asleep_; }

 private:
  StickAction action() const {
    StickAction current;
    current.light = light_;
    current.sleep = asleep_;
    return current;
  }

  // "light off, sleep".
  StickAction go_to_sleep() {
    light_ = false;
    asleep_ = true;
    return action();
  }

  uint8_t stick_;
  // Whether the stick has sent a PRESS since its wake, and the round that PRESS named.
  bool pressed_ = false;
  uint8_t press_round_ = 0;
  bool light_ = false;
  bool asleep_ = false;
  // The frames in a row without a beacon, and the clock at the last beacon received.
  uint8_t missed_ = 0;
  uint32_t last_beacon_us_ = 0;
};

}  // namespace answer_sticks

#endif  // ANSWER_STICKS_STICK_LOGIC_H
