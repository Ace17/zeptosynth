#pragma once

#include <stddef.h>

#include "fifo.h"
#include "osc.h"

const int MaxVoices = 16;

struct Command
{
  enum
  {
    NoteOn,
    NoteOff,
    ConfigChange,
  };

  int type;
  int value1 = 0; // note, cc
  double value2 = 0; // velocity, value
};

struct Config
{
  double Volume = 1;
  double LfoAmount = 0;
  double LfoRate = 0.5;
  double PitchBendDelta = 0;
  double OscType = 5;
  double PWM = 0.75;
};

struct ConfigVarTypeInfo
{
  int midiCC;
  const char* name;
  size_t offset;
};

constexpr ConfigVarTypeInfo ConfigTypeInfo[] = {
      {1, "LfoAmount", offsetof(Config, LfoAmount)},
      {2, "PitchBendDelta", offsetof(Config, PitchBendDelta)},
      {3, "OscType", offsetof(Config, OscType)},
      {4, "PWM", offsetof(Config, PWM)},
      {7, "Volume", offsetof(Config, Volume)},
      {5, "LfoRate", offsetof(Config, LfoRate)},
};

struct Synth
{
  public:
  // called from audio thread
  void run(float* samples, int count);

  // called from input thread
  void pushCommand(const Command& cmd);

  private:
  void processPendingCommands();
  void noteOn(int note);
  void noteOff(int note);

  // synth config
  Config config{};

  // synth state
  struct Voice
  {
    double pitch = 0;
    double vol = 0;

    Osc osc;
  };

  Voice voices[MaxVoices]{};
  char noteToVoice[128]{};
  double lfoPhase = 0;

  // command queue: filled b the input thread, consumed by the audio thread
  Fifo<Command> commandQueue;
};
