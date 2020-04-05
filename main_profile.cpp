#include <time.h>
#include <stdio.h>
#include "synth.h"

int main()
{
  Synth synth;

  int sampleCount = 1024 * 1024 * 128;
  const int blockSize = 256;

  auto t1 = clock();

  while(sampleCount > 0)
  {
    float buff[blockSize];
    synth.run(buff, blockSize);
    sampleCount -= blockSize;
  }

  auto t2 = clock();

  printf("time=%.2f\n", (t2 - t1) / double(CLOCKS_PER_SEC));
  return 0;
}

