#include <stdio.h>
#include <time.h>

#include "minblep.h"
#include "monotime.h"
#include "synth.h"

int main()
{
  fprintf(stderr, "MINBLEP size: %.2fkB\n", (MINBLEP.len * sizeof(float)) / 1024.0);
  Synth synth;

  const int N = 4096 * 256;
  const int blockSize = 256;

  const auto t0 = get_monotonic_time();

  int note = 60;
  int sampleCount = N;
  while(sampleCount > 0)
  {
    float buff[blockSize];
    synth.pushCommand({Command::NoteOn, note % 128, 1});
    synth.run(buff, blockSize);
    sampleCount -= blockSize;
    ++note;
  }

  const auto t1 = get_monotonic_time();

  fprintf(stderr, "Speed: %.3f kilosample/s\n", N / ((t1 - t0) * 1000.0));

  return 0;
}
