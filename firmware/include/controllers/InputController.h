#pragma once

#include <Arduino.h>
#include <AceButton.h>

class InputController {
 public:
  InputController();
  void begin();
  void update();

  uint16_t buttonBits() const;
  bool takeCalibrationRequest();
  bool takeActivity();

 private:
  static void handleButtonEvent(ace_button::AceButton* button, uint8_t eventType,
                                uint8_t buttonState);
  void onButtonEvent(ace_button::AceButton* button, uint8_t eventType);
  bool areBothPressed() const;

  ace_button::AceButton leftBtn_;
  ace_button::AceButton rightBtn_;

  bool calibrationRequested_ = false;
  bool hadActivity_ = false;
  unsigned long bothHeldStartMs_ = 0;
  bool calibrationHoldFired_ = false;
  bool leftPressed_ = false;
  bool rightPressed_ = false;
  bool lockPanFired_ = false;

  static InputController* instance_;
};
