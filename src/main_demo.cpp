#include <cassert>
#include <functional>
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

void playSong(Synth& synth, std::function<void(int ms)> wait)
{
  if(0)
  {
    synth.pushCommand({Command::ConfigChange, 3, (double)5});
    synth.pushCommand({Command::NoteOn, 40, 1});
    wait(1000);
    synth.pushCommand({Command::NoteOff, 40, 1});
    wait(10);
    synth.pushCommand({Command::ConfigChange, 3, (double)1});
    synth.pushCommand({Command::NoteOn, 40, 1});
    wait(1000);
    synth.pushCommand({Command::NoteOff, 40, 1});
    wait(10);
    return;
  }

  for(int oscType : {1, 3, 5})
  {
    synth.pushCommand({Command::ConfigChange, 3, (double)oscType});
    for(int i = 0; i < 8; ++i)
    {
      const int note = 90 + i * 2;
      synth.pushCommand({Command::NoteOn, note, 1});
      wait(100);
      synth.pushCommand({Command::NoteOff, note, 1});
      wait(100);

      if(i == 5)
        synth.pushCommand({Command::ConfigChange, 1, 0.5});
    }
  }

  wait(1000);
}

void safeMain(int argc, char* argv[])
{
  // dump MINBLEP
  {
    printf("MinBLEP size: %d samples\n", (int)MINBLEP.len);
    FILE* fp = fopen("minblep.data", "wb");
    std::vector<float> m(MINBLEP.len);
    m.assign(MINBLEP.begin(), MINBLEP.end());
    for(auto& v : m)
      v *= 0.5;
    fwrite(m.data(), 1, m.size() * sizeof(float), fp);
    fclose(fp);
  }

  Synth synth;

  if(argc == 1)
  {
    auto output = createAudioOutput(&synthesize, &synth);
    playSong(synth, &SDL_Delay);
  }
  else
  {
    FILE* fp = fopen("output.data", "wb");
    int totalMs = 0;
    int totalSamples = 0;
    auto write = [&](int ms) {
      int samples = (SAMPLERATE * ms) / 1000;
      std::vector<float> buffer(samples);
      synth.run(buffer.data(), buffer.size());
      fwrite(buffer.data(), 1, buffer.size() * sizeof(float), fp);

      totalMs += ms;
      totalSamples += samples;
    };
    playSong(synth, write);
    fprintf(stderr, "Duration: %d ms (%d samples)\n", totalMs, totalSamples);
    fclose(fp);
  }
}
}

int main(int argc, char* argv[])
{
  try
  {
    safeMain(argc, argv);
    return 0;
  }
  catch(Exception const& e)
  {
    fprintf(stderr, "Fatal: %s\n", e.msg);
    return 1;
  }
}
