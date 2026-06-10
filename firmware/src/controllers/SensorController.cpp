#include "controllers/SensorController.h"

#include "Config.h"

using namespace ifx::tlx493d;

SensorController::SensorController()
    : mag1Sensor_(Wire, TLx493D_IIC_ADDR_A0_e),
      mag2Sensor_(Wire, TLx493D_IIC_ADDR_A0_e),
      mag3Sensor_(Wire, TLx493D_IIC_ADDR_A0_e) {}

void SensorController::powerOff(int pin) { digitalWrite(pin, HIGH); }

void SensorController::powerOn(int pin) {
  digitalWrite(pin, LOW);
  delay(5);
}

void SensorController::begin() {

  pinMode(Config::PIN_MAG1_LS, OUTPUT);
  pinMode(Config::PIN_MAG2_LS, OUTPUT);
  pinMode(Config::PIN_MAG3_LS, OUTPUT);

  // All three rails are pulled high in hardware, so force them all off first
  // before bringing sensors up one-by-one for address assignment.

  powerOff(Config::PIN_MAG1_LS);
  powerOff(Config::PIN_MAG2_LS);
  powerOff(Config::PIN_MAG3_LS);
  delay(5);

  Wire.begin();
  Wire.setClock(400000);

  powerOn(Config::PIN_MAG1_LS);
  mag1Sensor_.begin(true, false, false, true);
  mag1Sensor_.setIICAddress(TLx493D_IIC_ADDR_A2_e);
  mag1Sensor_.setSensitivity(TLx493D_EXTRA_SHORT_RANGE_e);
  delay(10);

  powerOn(Config::PIN_MAG2_LS);
  mag2Sensor_.begin(true, false, false, true);
  mag2Sensor_.setIICAddress(TLx493D_IIC_ADDR_A1_e);
  mag2Sensor_.setSensitivity(TLx493D_EXTRA_SHORT_RANGE_e);
  delay(10);

  powerOn(Config::PIN_MAG3_LS);
  mag3Sensor_.begin(true, false, false, true);
  mag3Sensor_.setSensitivity(TLx493D_EXTRA_SHORT_RANGE_e);
  delay(10);
}

void SensorController::readRaw(float out[9]) {
  double mag1x = 0, mag1y = 0, mag1z = 0, temp1 = 0;
  double mag2x = 0, mag2y = 0, mag2z = 0, temp2 = 0;
  double mag3x = 0, mag3y = 0, mag3z = 0, temp3 = 0;

  mag1Sensor_.getMagneticFieldAndTemperature(&mag1x, &mag1y, &mag1z, &temp1);
  mag2Sensor_.getMagneticFieldAndTemperature(&mag2x, &mag2y, &mag2z, &temp2);
  mag3Sensor_.getMagneticFieldAndTemperature(&mag3x, &mag3y, &mag3z, &temp3);

  // MAG1 = bottom, MAG2 = top left, MAG3 = top right.
  out[0] = mag1x;
  out[1] = mag1y;
  out[2] = mag1z;
  out[3] = mag2x;
  out[4] = mag2y;
  out[5] = mag2z;
  out[6] = mag3x;
  out[7] = mag3y;
  out[8] = mag3z;
}

void SensorController::beginCalibration() {
  calibrationActive_ = true;
  calibrationDone_ = false;
  calibrationSamples_ = 0;
  lastCalibrationSampleMs_ = 0;
  for (int i = 0; i < 9; i++) {
    calibrationSum_[i] = 0.0;
  }
}

void SensorController::updateCalibration() {
  if (!calibrationActive_) {
    return;
  }

  const unsigned long now = millis();
  if (lastCalibrationSampleMs_ != 0 &&
      (now - lastCalibrationSampleMs_) < 10) {
    return;
  }
  lastCalibrationSampleMs_ = now;

  float raw[9] = {};
  readRaw(raw);

  for (int i = 0; i < 9; i++) {
    calibrationSum_[i] += raw[i];
  }

  calibrationSamples_++;
  if (calibrationSamples_ < Config::ZERO_SAMPLES) {
    return;
  }

  for (int i = 0; i < 9; i++) {
    baseline_[i] = calibrationSum_[i] / Config::ZERO_SAMPLES;
  }

  calibrationActive_ = false;
  calibrationDone_ = true;
}

bool SensorController::calibrationDone() const { return calibrationDone_; }

const float* SensorController::baseline() const { return baseline_; }
