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
  if(0)
    return sin(phase * 2.0 * M_PI);
  return phase - floor(phase);
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
  return exp((pitch-69) * LN_2 / 12.0) * 440;
}

void processMidiEvent(const uint8_t* data, int len)
{
  static uint8_t status;
  if(data[0] & 0x80)
  {
    status = data[0];
    ++data;
    --len;
  }

  if(status == 0x90 && data[1] > 0)
  {
    freq = pitchToFreq(data[0]);
    fprintf(stderr, "NOTE-ON: %d (%.2f)\n", data[0], freq);
    vol = 0.5;
  }
  else if(status == 0x80 || (status == 0x90 && data[1] == 0))
  {
    fprintf(stderr, "NOTE-OFF: %d\n", data[0]);
    vol = 0;
  }

  if(1)
  {
    fprintf(stderr, "[%.2X] ", status);

    for(int i = 0; i < len; ++i)
      fprintf(stderr, "%.2X ", data[i]);

    fprintf(stderr, "\n");
    fflush(stderr);
  }
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
      processMidiEvent(buffer, len);

    SDL_Delay(1);
  }
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

