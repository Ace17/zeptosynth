#include <cassert>
#include <stdint.h>
#include <stdio.h>

#include "SDL.h" // SDL_Delay
#include "audio_output.h"
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


void safeMain()
{
  Synth synth;

  auto output = createAudioOutput(&synthesize, &synth);

  for(int i=0;i < 4;++i)
  {
    const int note = 48 + i * 2;
    synth.pushCommand({Command::NoteOn, note, 1});
    SDL_Delay(100);
    synth.pushCommand({Command::NoteOff, note, 1});
    SDL_Delay(100);
  }

  SDL_Delay(1000);
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

