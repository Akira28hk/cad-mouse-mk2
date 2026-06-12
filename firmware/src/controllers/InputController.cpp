#include "controllers/InputController.h"
#include "controllers.h"

#include "Config.h"

using namespace ace_button;

namespace {
const long kCalibrationHoldMs = 3000;
const uint16_t leftButtonMask = 0x0001;
const uint16_t rightButtonMask = 0x0002;
}

InputController* InputController::instance_ = nullptr;

InputController::InputController()
    : leftBtn_(Config::PIN_LEFT_BTN), rightBtn_(Config::PIN_RIGHT_BTN) {}

void InputController::begin() {
  pinMode(Config::PIN_LEFT_BTN, INPUT_PULLUP);
  pinMode(Config::PIN_RIGHT_BTN, INPUT_PULLUP);

  ButtonConfig* cfg = ButtonConfig::getSystemButtonConfig();
  cfg->setEventHandler(handleButtonEvent);

  instance_ = this;
  calibrationRequested_ = false;
  hadActivity_ = false;
  bothHeldStartMs_ = 0;
  calibrationHoldFired_ = false;
  leftPressed_ = false;
  rightPressed_ = false;
}

void InputController::update() {
  const unsigned long now = millis();
  leftBtn_.check();
  rightBtn_.check();

  if (!areBothPressed()) {
    bothHeldStartMs_ = 0;
    calibrationHoldFired_ = false;
    lockPanFired_ = false;
    return;
  }

  if (bothHeldStartMs_ == 0) {
    bothHeldStartMs_ = now;
    return;
  }

  if (!lockPanFired_ && !calibrationHoldFired_ && (now - bothHeldStartMs_) >= 1000) {
    if (!Config::LOCKPAN) {
      lockPanFired_ = true;
      ledController.setSolid(Config::LED_LOCK_COLOR);
      Config::LOCKPAN = true;
    } else {
      lockPanFired_ = true;
      ledController.setSolid(Config::LED_IDLE_COLOR);
      Config::LOCKPAN = false;
    }  
  }
    
  if (!calibrationHoldFired_ && (now - bothHeldStartMs_) >= kCalibrationHoldMs) {
    calibrationRequested_ = true;
    hadActivity_ = true;
    calibrationHoldFired_ = true;
  }
}

uint16_t InputController::buttonBits() const {
  uint16_t bits = 0;
  if (leftPressed_) {
    bits |= leftButtonMask;
  }
  if (rightPressed_) {
    bits |= rightButtonMask;
  }
  return bits;
}

bool InputController::takeCalibrationRequest() {
  const bool out = calibrationRequested_;
  calibrationRequested_ = false;
  return out;
}

bool InputController::takeActivity() {
  const bool out = hadActivity_;
  hadActivity_ = false;
  return out;
}

void InputController::handleButtonEvent(AceButton* button, uint8_t eventType,
                                        uint8_t /*buttonState*/) {
  if (instance_ == nullptr) {
    return;
  }
  instance_->onButtonEvent(button, eventType);
}

void InputController::onButtonEvent(AceButton* button, uint8_t eventType) {
  if (eventType == AceButton::kEventPressed) {
    if (button->getPin() == Config::PIN_LEFT_BTN) {
      leftPressed_ = true;
    } else if (button->getPin() == Config::PIN_RIGHT_BTN) {
      rightPressed_ = true;
    } else {
      return;
    }
    hadActivity_ = true;
    return;
  }

  if (eventType == AceButton::kEventReleased) {
    if (button->getPin() == Config::PIN_LEFT_BTN) {
      leftPressed_ = false;
    } else if (button->getPin() == Config::PIN_RIGHT_BTN) {
      rightPressed_ = false;
    } else {
      return;
    }
    hadActivity_ = true;
  }
}

bool InputController::areBothPressed() const {
  return leftPressed_ && rightPressed_;
}
