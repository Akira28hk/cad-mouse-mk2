#include "states/SleepState.h"

#include <Arduino.h>
#include <math.h>

#include "Config.h"
#include "Controllers.h"
#include "StateMachine.h"

void SleepState::enter() {
  ledController.dim();
  lastWakePollMs_ = millis();
}

void SleepState::update() {
  inputController.update();

  if (inputController.takeActivity() || knobMoved()) {
    stateMachine.changeState(&StateMachine::idleState);
    return;
  }
  
  // Flush neutral HID state so any motion/buttons from before sleep
  // do not stay latched on the host. After the round-robin finishes
  // (within 3 ticks) sendReports() early-returns and this is a no-op.
  hidController.sendNeutral(0);
}

void SleepState::exit() {}

bool SleepState::knobMoved() {
  const unsigned long now = millis();
  if ((now - lastWakePollMs_) < Config::SLEEP_WAKE_POLL_MS) {
    return false;
  }
  lastWakePollMs_ = now;

  float raw[9] = {};
  if (!sensorController.readRaw(raw)) {
    return false;
  }

  const float* baseline = sensorController.baseline();
  for (int i = 0; i < 9; i++) {
    if (fabsf(raw[i] - baseline[i]) > Config::SLEEP_WAKE_THRESHOLD_MT) {
      return true;
    }
  }
  return false;
}
