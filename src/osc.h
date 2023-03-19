#pragma once

#include "minblep.h"

struct Osc
{
  double phase = 0.0;
  double pwm = 0.10;

  float work(int type, double phaseIncrement);

  private:
  float work_naive_sine(double phaseIncrement);
  float work_naive_sawtooth(double phaseIncrement);
  float work_naive_square(double phaseIncrement);
  float work_feedbackfm_sawtooth(double phaseIncrement);
  float work_feedbackfm_square(double phaseIncrement);
  float work_minblep_sawtooth(double phaseIncrement);
  float work_minblep_pulse(double phaseIncrement);

  // feedback FM
  double internalHistory1 = 0.0;
  double internalHistory2 = 0.0;

  // minBlep
  int index = 0;
  int sign = 1;
  static constexpr int RING_BUFFER_SIZE = 1024;
  float circularBuffer[RING_BUFFER_SIZE]{};
};
