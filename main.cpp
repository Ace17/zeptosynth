#include <stdio.h>
#include <stdint.h>
#include <cassert>
#include "SDL.h"

#include "midi_input.h"

namespace
{
const int PORT_NUMBER = 1;

void safeMain()
{
  auto input = createMidiInput(PORT_NUMBER);

  while(1)
  {
    input->read();
    SDL_Delay(1);
  }

  delete input;

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

