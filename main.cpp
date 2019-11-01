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

struct Voice
{
  double pitch = 0;
  double freq = 0;
  double vol = 0;
  double phase = 0;
};

Voice voices[8];
uint8_t status;
uint8_t noteToVoice[128];
double pitchBendDelta = 0;

static double synth(double phase)
{
  if(0)
    return sin(phase * 2.0 * M_PI);

  return phase - floor(phase);
}

double pitchToFreq(double pitch)
{
  static double const LN_2 = log(2.0);
  return exp(pitch * LN_2 / 12.0);
}

void audioCallback(float* samples, int count, void* userParam)
{
  for(int i = 0; i < count; ++i)
    samples[i] = 0;

  for(auto& voice : voices)
  {
    if(voice.vol == 0)
      continue;

    auto freq = pitchToFreq(voice.pitch + pitchBendDelta - 69) * 440.0 / SAMPLERATE;

    for(int i = 0; i < count; ++i)
    {
      samples[i] += synth(voice.phase) * voice.vol;
      voice.phase += freq;
    }

    if(voice.phase >= 1)
      voice.phase -= 1;
  }

  for(int i = 0; i < count; ++i)
    samples[i] = atan(samples[i]) / 1.5;
}

void processMidiEvent(const uint8_t* data, int len)
{
  if(data[0] & 0x80)
  {
    status = data[0];
    ++data;
    --len;
  }

  if(status == 0xFE)
    return; // active sensing: ignore

  if(status == 0x90 && data[1] > 0)
  {
    int note = data[0];

    int i = 0;

    while(voices[i].vol > 0 && i < 16 - 1)
      ++i;

    noteToVoice[note] = i;
    auto& voice = voices[i];

    voice.pitch = data[0];
    voice.vol = 1.0;
    fprintf(stderr, "NOTE-ON: %d\n", data[0]);
  }
  else if(status == 0x80 || (status == 0x90 && data[1] == 0))
  {
    int note = data[0];
    auto& voice = voices[noteToVoice[note]];
    voice.vol = 0;
    fprintf(stderr, "NOTE-OFF: %d\n", note);
  }
  else if(status == 0xE0)
  {
    double pos = (data[1] - 64) / 64.0;
    pitchBendDelta = 2.0 * pos; // [-2, 2]
    fprintf(stderr, "PITCH-BEND: %d (pos=%+.1f, factor=%.2f)\n", data[1], pos, pitchBendDelta);
  }
  else if(1)
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

