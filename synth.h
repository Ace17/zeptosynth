#pragma once

struct Synth
{
  struct Voice
  {
    double pitch = 0;
    double freq = 0;
    double vol = 0;
    double phase = 0;
  };

  Voice voices[8] {};
  char noteToVoice[128] {};
  double pitchBendDelta = 0;
  double lfoPhase = 0;
  double lfoAmount = 0;

  void run(float* samples, int count);
};

