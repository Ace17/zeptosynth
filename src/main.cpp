#include <cassert>
#include <stdint.h>
#include <stdio.h>

#include "SDL.h" // SDL_Delay
#include "audio_output.h"
#include "midi_input.h"
#include "profiler.h"
#include "synth.h"

namespace
{
const int PORT_NUMBER = 1;

void synthesize(float* samples, int count, void* userParam)
{
  ProfileScope scope;
  auto synth = (Synth*)userParam;
  synth->run(samples, count);
}

void processMidiEvent(Synth* synth, const uint8_t* data, int len)
{
  static uint8_t status;

  if(data[0] & 0x80)
  {
    status = data[0];
    ++data;
    --len;
  }

  if(status == 0xFE)
    return; // active sensing: ignore

  // ignore channel
  status &= 0xF0;

  if(status == 0x90 && data[1] > 0)
  {
    const int note = data[0];

    synth->pushCommand({Command::NoteOn, note, 1});
    fprintf(stderr, "NOTE-ON: %d\n", note);
  }
  else if(status == 0x80 || (status == 0x90 && data[1] == 0))
  {
    int note = data[0];
    synth->pushCommand({Command::NoteOff, note, 1});
    fprintf(stderr, "NOTE-OFF: %d\n", note);
  }
  else if(status == 0xB0)
  {
    int index = -1;
    for(auto& configVarInfo : ConfigTypeInfo)
      if(configVarInfo.midiCC == data[0])
        index = int(&configVarInfo - ConfigTypeInfo);

    if(index >= 0)
    {
      const double value = data[1] / 128.0;
      synth->pushCommand({Command::ConfigChange, index, value});
    }
    else
    {
      fprintf(stderr, "Unknown CC: %d %d\n", data[0], data[1]);
    }
  }
  else if(status == 0xE0)
  {
    double pos = (data[1] - 64) / 64.0;
    const double value = 2.0 * pos; // [-2, 2]

    // pitchbend delta
    synth->pushCommand({Command::ConfigChange, 1, value});
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
    double timestamp;
    int len;

    while((len = input->read(buffer, timestamp)) > 0)
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
