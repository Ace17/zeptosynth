#include "osc.h"

#include <cmath>

#if defined(__arm__)
__asm__(".symver pow,pow@GLIBC_2.4");
#endif

float Osc::work(int type, double phaseIncrement)
{
  switch(type)
  {
  case 0:
    return work_naive_sine(phaseIncrement);
  case 1:
    return work_naive_square(phaseIncrement);
  case 2:
    return work_naive_sawtooth(phaseIncrement);
  case 3:
    return work_feedbackfm_square(phaseIncrement);
  case 4:
    return work_feedbackfm_sawtooth(phaseIncrement);
  case 5:
    return work_minblep_square(phaseIncrement);
  case 6:
    return work_minblep_sawtooth(phaseIncrement);
  default:
    return work_naive_sine(phaseIncrement);
  }
}

float Osc::work_naive_sine(double phaseIncrement)
{
  phase += phaseIncrement;

  while(phase >= 1)
    phase -= 1;

  return sin(phase * 2.0 * M_PI);
}

float Osc::work_naive_sawtooth(double phaseIncrement)
{
  phase += phaseIncrement;

  while(phase >= 1)
    phase -= 1;

  return phase - floor(phase);
}

float Osc::work_naive_square(double phaseIncrement)
{
  phase += phaseIncrement;

  while(phase >= 1)
    phase -= 1;

  return phase - floor(phase) < 0.5 ? -0.7 : +0.7;
}

double pow6(double value)
{
  auto v = value * value;
  return v * v * v;
}

float Osc::work_feedbackfm_sawtooth(double phaseIncrement)
{
  phase += phaseIncrement;
  while(phase >= 1.0)
    phase -= 1.0;

  double scaling = 54.0 * pow6(0.5 - phaseIncrement);

  // alternative scaling: scaling = 13.0*pow((0.5-phaseIncrement), 4.0);

  double temp = (internalHistory1 + sin(M_PI * (((2.0 * phase) - 1.0) + internalHistory1 * scaling))) * 0.5f;
  // compensate HF rolloff
  double const A0 = 2.5; // precalculated coeffs
  double const A1 = -1.5; // for HF compensation
  double output = (A0 * temp) + (A1 * internalHistory1);
  double DC = 0.376 - phaseIncrement * 0.752; // calculate DC compensation
  double norm = 1.0 - (2.0 * phaseIncrement); // calculate normalization (frequency dependent amplitude compensation)
  output = (output + DC) * norm;
  internalHistory1 = temp;
  return output;
}

float Osc::work_feedbackfm_square(double phaseIncrement)
{
  static const double squareAmount = 1.0; // 0.0 = sawtooth, 1.0 = square
  static const double pulseWidth = 1.0; // 1.0 = perfect square

  phase += phaseIncrement;
  while(phase >= 1.0)
    phase -= 1.0;

  double scaling = 54.0 * pow6(0.5 - phaseIncrement); // calculate scaling
  double temp = (internalHistory1 + sin(M_PI * (((2.0 * phase) - 1.0) + internalHistory1 * scaling))) * 0.5f;
  double temp2 = squareAmount *
        (internalHistory2 + sin(M_PI * (((2.0 * phase) - 1.0) + (internalHistory2 * scaling) + pulseWidth))) * 0.5f;
  double square = temp - temp2; // you can make a square by subtracting two out of phase sawtooth waves
                                // compensate HF rolloff
  double const A0 = 2.5; // precalculated coeffs
  double const A1 = -1.5; // for HF compensation
  double output = (A0 * square) + (A1 * (internalHistory1 - internalHistory2));
  double norm = 1.0 - (2.0 * phaseIncrement); // calculate normalization (frequency dependent amplitude compensation)
  output = output * norm;
  internalHistory1 = temp;
  internalHistory2 = temp2;
  return output;
}

static float lerp(float alpha, float a, float b) { return a + (b - a) * alpha; }

float Osc::work_minblep_sawtooth(double phaseIncrement)
{
  phase += phaseIncrement;
  index = (index + 1) % RING_BUFFER_SIZE;
  while(phase >= 1.0)
  {
    phase -= 1.0;
    double exactCrossTime = 1.0 - ((phaseIncrement - phase) / phaseIncrement);
    for(int i = 0; i < 47; i++)
    {
      double tempIndex = (exactCrossTime * 8.0) + (i * 8.0);
      double tempFraction = tempIndex - floor(tempIndex);
      const auto a = MINBLEP[(int)floor(tempIndex)];
      const auto b = MINBLEP[(int)ceil(tempIndex)];
      circularBuffer[(index + i) % RING_BUFFER_SIZE] += (1.0 - lerp(tempFraction, a, b));
    }
  }

  circularBuffer[index] += (phase - phaseIncrement);
  double output = circularBuffer[index];
  circularBuffer[index] = 0.0;
  return output;
}

float Osc::work_minblep_square(double phaseIncrement)
{
  phase += phaseIncrement;
  index = (index + 1) % RING_BUFFER_SIZE;
  if(phase >= 0.5)
  {
    phase -= 0.5;
    sign = -sign;

    double exactCrossTime = phase / phaseIncrement;

    double tempIndex = (exactCrossTime * 8.0);

    for(int i = 0; i < MINBLEP.len / 8; ++i)
    {
      const auto f = floor(tempIndex);
      const auto frac = tempIndex - f;

      const auto a = MINBLEP[(int)f + 0];
      const auto b = MINBLEP[(int)f + 1];

      const auto val = (1.0 - lerp(frac, a, b));

      circularBuffer[(index + i) % RING_BUFFER_SIZE] += -sign * 2 * val;

      tempIndex += 8;
    }
  }

  double output = circularBuffer[index] + sign;
  circularBuffer[index] = 0.0;
  return output;
}
