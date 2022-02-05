#pragma once

#include "unique.h"
#include <cstdint>

struct IMidiInput
{
  static constexpr int MAX_SIZE = 4096;

  virtual ~IMidiInput() = default;
  virtual int read(uint8_t* buffer, double& timestamp) = 0;
};

Unique<IMidiInput> createMidiInput(int port);

