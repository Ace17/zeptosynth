#include <stdio.h>
#include <stdint.h>
#include <cassert>
#include "SDL.h" // SDL_Delay

#include "midi_input.h"
#include "audio_output.h"

namespace
{
const int PORT_NUMBER = 1;

double freq = 880;
double phase = 0;
double lfoPhase = 0;

static double synth(double phase)
{
  return sin(phase * 2.0 * M_PI);
  // return phase - floor(phase);
}

void audioCallback(float* samples, int count, void* userParam)
{
  for(int i = 0; i < count; ++i)
  {
    samples[i] = synth(phase) * 0.5;
    phase += freq / SAMPLERATE;
  }

  if(phase >= 1)
    phase -= 1;

  if(lfoPhase >= 1)
    lfoPhase -= 1;
}

void safeMain()
{
  auto input = createMidiInput(PORT_NUMBER);
  auto output = createAudioOutput(&audioCallback, nullptr);

  while(1)
  {
    input->read();
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

