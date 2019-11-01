#include <stdio.h>
#include <stdint.h>
#include <cassert>
#include "SDL.h" // SDL_Delay

#include "midi_input.h"
#include "audio_output.h"

#if defined(__arm__)
__asm__ (".symver exp,exp@GLIBC_2.4");
#endif

namespace
{
const int PORT_NUMBER = 1;

double freq = 0;
double vol = 0;
double phase = 0;

static double synth(double phase)
{
  return sin(phase * 2.0 * M_PI);
  // return phase - floor(phase);
}

void audioCallback(float* samples, int count, void* userParam)
{
  for(int i = 0; i < count; ++i)
  {
    samples[i] = synth(phase) * vol;
    phase += freq / SAMPLERATE;
  }

  if(phase >= 1)
    phase -= 1;
}

double pitchToFreq(int pitch)
{
  static double const LN_2 = log(2.0);
  return exp(pitch * LN_2 / 12.0) * 440;
}

void processEvent(const uint8_t* data, int len)
{
  if(data[0] == 0x90)
  {
    freq = pitchToFreq(data[1] - 69);
    fprintf(stderr, "%.2f\n", freq);
    fflush(stderr);
    vol = 0.5;
  }
  else if(data[0] == 0x80)
  {
    vol = 0;
  }

  for(int i = 0; i < len; ++i)
    fprintf(stderr, "%.2X ", data[i]);

  fprintf(stderr, "\n");
  fflush(stderr);
}

void safeMain()
{
  auto input = createMidiInput(PORT_NUMBER);
  auto output = createAudioOutput(&audioCallback, nullptr);

  while(1)
  {
    uint8_t buffer[IMidiInput::MAX_SIZE];
    int len;

    while((len = input->read(buffer)) > 0)
    {
      processEvent(buffer, len);
    }

    SDL_Delay(1);
  }

  printf("OK\n");
}
}

int main()
{
  try
  {
    safeMain();
    return 0;
  }
  catch(Exception const& e)
  {
    fprintf(stderr, "Fatal: %s\n", e.msg);
    return 1;
  }
}

