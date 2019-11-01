#include <stdio.h>
#include <stdint.h>
#include <cassert>
#include "SDL.h" // SDL_Delay

#include "synth.h"
#include "profiler.h"
#include "midi_input.h"
#include "audio_output.h"

namespace
{
const int PORT_NUMBER = 1;

void synthesize(float* samples, int count, void* userParam)
{
  ProfileScope scope;
  auto synth = (Synth*)userParam;
  synth->run(samples, count);
}

uint8_t status;

void processMidiEvent(Synth* synth, const uint8_t* data, int len)
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
    const int note = data[0];

    synth->noteOn(note);
    fprintf(stderr, "NOTE-ON: %d\n", note);
  }
  else if(status == 0x80 || (status == 0x90 && data[1] == 0))
  {
    int note = data[0];
    synth->noteOff(note);
    fprintf(stderr, "NOTE-OFF: %d\n", note);
  }
  else if(status == 0xB0)
  {
    if(data[0] == 1) // mod wheel
    {
      synth->lfoAmount = data[1] / 128.0;
    }
    else
    {
      fprintf(stderr, "Unknown CC: %d %d\n", data[0], data[1]);
    }
  }
  else if(status == 0xE0)
  {
    double pos = (data[1] - 64) / 64.0;
    synth->pitchBendDelta = 2.0 * pos; // [-2, 2]
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
  Synth synth;

  auto input = createMidiInput(PORT_NUMBER);
  auto output = createAudioOutput(&synthesize, &synth);

  while(1)
  {
    uint8_t buffer[IMidiInput::MAX_SIZE];
    int len;

    while((len = input->read(buffer)) > 0)
      processMidiEvent(&synth, buffer, len);

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

