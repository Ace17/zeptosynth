#pragma once

#include "fifo.h"

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
  union
  {
    struct
    {
      double Volume = 1;
      double LfoAmount = 0;
      double PitchBendDelta = 0;
    };
    double values[3];
  };
};

const Config DefaultConfig;

static_assert(&DefaultConfig.values[0] == &DefaultConfig.Volume);
static_assert(&DefaultConfig.values[1] == &DefaultConfig.LfoAmount);
static_assert(&DefaultConfig.values[2] == &DefaultConfig.PitchBendDelta);

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
  Config config {};

  // synth state
  struct Voice
  {
    double pitch = 0;
    double freq = 0;
    double vol = 0;
    double phase = 0;
  };

  Voice voices[MaxVoices] {};
  char noteToVoice[128] {};
  double lfoPhase = 0;

  // command queue: filled b the input thread, consumed by the audio thread
  Fifo<Command> commandQueue;
};

