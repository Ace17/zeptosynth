// MinBLEP Generation Code
// Based on code by Daniel Werner

#include "minblep.h"

#include <array>
#include <cstdio>
#include <math.h>

#if defined(__arm__)
__asm__(".symver exp,exp@GLIBC_2.4");
__asm__(".symver expf,expf@GLIBC_2.4");
__asm__(".symver logf,logf@GLIBC_2.4");
#endif

namespace
{
constexpr auto PI = M_PI;

using real = double;

constexpr real SINC(real x)
{
  if(x == 0.0f)
    return 1.0f;

  const real pix = PI * x;
  return sin(pix) / pix;
}

// Generate Blackman Window
constexpr void BlackmanWindow(int n, real* w)
{
  int m = n - 1;

  real fm = (real)m;
  for(int i = 0; i <= m; i++)
  {
    real f1 = (2.0f * PI * (real)i) / fm;
    real f2 = 2.0f * f1;
    w[i] = 0.42f - (0.5f * cosf(f1)) + (0.08f * cosf(f2));
  }
}

// Discrete Fourier Transform
constexpr void DFT(int n, real* realTime, real* imagTime, real* realFreq, real* imagFreq)
{
  for(int k = 0; k < n; k++)
  {
    realFreq[k] = 0.0f;
    imagFreq[k] = 0.0f;
  }

  for(int k = 0; k < n; k++)
  {
    for(int i = 0; i < n; i++)
    {
      real p = (2.0f * PI * (real)(k * i)) / n;
      real sr = cosf(p);
      real si = -sinf(p);
      realFreq[k] += (realTime[i] * sr) - (imagTime[i] * si);
      imagFreq[k] += (realTime[i] * si) + (imagTime[i] * sr);
    }
  }
}

// Inverse Discrete Fourier Transform
constexpr void InverseDFT(int n, real* realTime, real* imagTime, real* realFreq, real* imagFreq)
{
  for(int k = 0; k < n; k++)
    realTime[k] = 0.0f;

  for(int k = 0; k < n; k++)
    imagTime[k] = 0.0f;

  for(int k = 0; k < n; k++)
  {
    for(int i = 0; i < n; i++)
    {
      real p = (2.0f * PI * (real)(k * i)) / n;
      real sr = cosf(p);
      real si = -sinf(p);
      realTime[k] += (realFreq[i] * sr) + (imagFreq[i] * si);
      imagTime[k] += (realFreq[i] * si) - (imagFreq[i] * sr);
    }
    realTime[k] /= n;
    imagTime[k] /= n;
  }
}

// Complex Absolute Value
constexpr real cabs(real x, real y) { return sqrtf((x * x) + (y * y)); }

// Complex Exponential
constexpr void cexp(real x, real y, real* zx, real* zy)
{
  real expx = exp(x);
  *zx = expx * cos(y);
  *zy = expx * sin(y);
}

// Compute Real Cepstrum Of Signal
constexpr void RealCepstrum(int n, real* signal, real* realCepstrum)
{
  real realTime[n]{};
  real imagTime[n]{};
  real realFreq[n]{};
  real imagFreq[n]{};

  // Compose Complex FFT Input

  for(int i = 0; i < n; i++)
  {
    realTime[i] = signal[i];
    imagTime[i] = 0.0f;
  }

  // Perform DFT

  DFT(n, realTime, imagTime, realFreq, imagFreq);

  // Calculate Log Of Absolute Value

  for(int i = 0; i < n; i++)
  {
    realFreq[i] = logf(cabs(realFreq[i], imagFreq[i]));
    imagFreq[i] = 0.0f;
  }

  // Perform Inverse FFT

  InverseDFT(n, realTime, imagTime, realFreq, imagFreq);

  // Output Real Part Of FFT
  for(int i = 0; i < n; i++)
    realCepstrum[i] = realTime[i];
}

// Compute Minimum Phase Reconstruction Of Signal
constexpr void MinimumPhase(int n, real* realCepstrum, real* minimumPhase)
{
  int nd2 = n / 2;
  real realTime[n]{};
  real imagTime[n]{};
  real realFreq[n]{};
  real imagFreq[n]{};

  if((n % 2) == 1)
  {
    realTime[0] = realCepstrum[0];
    for(int i = 1; i < nd2; i++)
      realTime[i] = 2.0f * realCepstrum[i];
    for(int i = nd2; i < n; i++)
      realTime[i] = 0.0f;
  }
  else
  {
    realTime[0] = realCepstrum[0];
    for(int i = 1; i < nd2; i++)
      realTime[i] = 2.0f * realCepstrum[i];
    realTime[nd2] = realCepstrum[nd2];
    for(int i = nd2 + 1; i < n; i++)
      realTime[i] = 0.0f;
  }

  for(int i = 0; i < n; i++)
    imagTime[i] = 0.0f;

  DFT(n, realTime, imagTime, realFreq, imagFreq);

  for(int i = 0; i < n; i++)
    cexp(realFreq[i], imagFreq[i], &realFreq[i], &imagFreq[i]);

  InverseDFT(n, realTime, imagTime, realFreq, imagFreq);

  for(int i = 0; i < n; i++)
    minimumPhase[i] = realTime[i];
}

// https://www.kvraudio.com/forum/viewtopic.php?f=33&t=364256&sid=d2cd7e620946c6c9f69a79619aa5f02b&start=15
const int zeroCrossings = 48; // 24 is "more than enough"
const int overSampling = 8;
const int n = (zeroCrossings * 2 * overSampling) + 1;

// Generate MinBLEP And Return It In An Array Of Floating Point Values
std::array<float, n> GenerateMinBLEP()
{
  fprintf(stderr, "Precomputing MinBLEP ... ");
  fflush(stderr);
  real buffer1[n]{};
  real buffer2[n]{};

  // Generate Sinc
  real a = (real)-zeroCrossings;
  real b = (real)zeroCrossings;
  for(int i = 0; i < n; i++)
  {
    real r = ((real)i) / ((real)(n - 1));
    buffer1[i] = SINC(a + (r * (b - a)));
  }

  // Window Sinc
  BlackmanWindow(n, buffer2);
  for(int i = 0; i < n; i++)
    buffer1[i] *= buffer2[i];

  // Minimum Phase Reconstruction
  RealCepstrum(n, buffer1, buffer2); // slow
  MinimumPhase(n, buffer2, buffer1); // slow

  // Integrate Into MinBLEP
  std::array<float, n> minBLEP{};
  a = 0.0f;
  for(int i = 0; i < n; i++)
  {
    a += buffer1[i];
    minBLEP[i] = a;
  }

  // Normalize
  const float ratio = 1.0 / minBLEP[n - 1];
  for(int i = 0; i < n; i++)
    minBLEP[i] *= ratio;

  fprintf(stderr, "OK\n");
  fflush(stderr);
  return minBLEP;
}
}

const std::array<float, n> MINBLEP_data = GenerateMinBLEP();
extern const Span<const float> MINBLEP{MINBLEP_data.data(), MINBLEP_data.size()};
