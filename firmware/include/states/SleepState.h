#pragma once

#include <Arduino.h>

#include "State.h"

class SleepState : public State {
 public:
  void enter() override;
  void update() override;
  void exit() override;

 private:
  bool knobMoved();

  unsigned long lastWakePollMs_ = 0;
};
