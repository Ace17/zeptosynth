#pragma once

struct Exception
{
  const char* msg;
};

struct IMidiInput
{
  virtual ~IMidiInput() = default;
  virtual void read() = 0;
};

IMidiInput* createMidiInput(int port);

