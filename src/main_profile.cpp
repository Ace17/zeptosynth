#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "minblep.h"
#include "monotime.h"
#include "synth.h"

int VarIndex(const char* name)
{
  int i = 0;
  for(auto& var : ConfigTypeInfo)
  {
    if(strcmp(name, var.name) == 0)
      return i;
    ++i;
  }

  assert(false);
  return 0;
}

const int N = 8192 * 256;
const int blockSize = 256;

void measure(const char* name, void (*func)())
{
  fprintf(stderr, "%s: ", name);
  fflush(stderr);
  const auto t0 = get_monotonic_time();
  func();
  const auto t1 = get_monotonic_time();
  const auto freq = N / ((t1 - t0) * 1000.0);
  fprintf(stderr, "%.3f kHz (%.0fx) \n", freq, freq / 48.0);
}

template<int OscType>
void test_osc_type()
{
  Synth synth;

  synth.pushCommand({Command::ConfigChange, VarIndex("OscType"), OscType});

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
}

int main()
{
  fprintf(stderr, "MINBLEP size: %.2fkB\n", (MINBLEP.len * sizeof(float)) / 1024.0);

  measure("OscType=1", test_osc_type<1>);
  measure("OscType=2", test_osc_type<2>);
  measure("OscType=3", test_osc_type<3>);
  measure("OscType=4", test_osc_type<4>);
  measure("OscType=5", test_osc_type<5>);
  measure("OscType=6", test_osc_type<6>);

  return 0;
}
