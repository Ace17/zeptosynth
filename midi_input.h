#pragma once

#include "unique.h"

struct Exception
{
  const char* msg;
};

struct IMidiInput
{
  virtual ~IMidiInput() = default;
  virtual void read() = 0;
};

Unique<IMidiInput> createMidiInput(int port);

