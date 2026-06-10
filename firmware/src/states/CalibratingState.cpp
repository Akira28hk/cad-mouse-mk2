#include "states/CalibratingState.h"

#include <Arduino.h>

#include "Config.h"
#include "Controllers.h"
#include "StateMachine.h"

void CalibratingState::enter() {
  sensorController.beginCalibration();
  motionController.reset();
  ledController.startSpinner(Config::LED_CALIBRATING_COLOR);
}

void CalibratingState::update() {
  inputController.update();
  ledController.updateSpinner();
  sensorController.updateCalibration();

  // Push a neutral HID state to the host every tick so the 3-second
  // chord that triggered calibration does not leave the buttons
  // latched as pressed for the full 2-second sample window, and so
  // any live motion from the previous Idle frame is cleared.
  hidController.sendNeutral(0);

  if (sensorController.calibrationDone()) {
    stateMachine.changeState(&StateMachine::idleState);
  }
}

void CalibratingState::exit() {}
