#include "synth.h"

#include <cmath>

#include "audio_output.h" // SAMPLERATE

#if defined(__arm__)
__asm__(".symver exp,exp@GLIBC_2.4");
#endif

inline double clamp(double val, double min, double max)
{
  if(val < min)
    return min;

  if(val > max)
    return max;

  return val;
}

inline double pitchToFreq(double pitch)
{
  static double const LN_2 = log(2.0);
  return exp(pitch * LN_2 / 12.0);
}

const int CHUNK_PERIOD = 16;

void Synth::run(float* samples, int count)
{
  processPendingCommands();

  for(int i = 0; i < count; ++i)
    samples[i] = 0;

  while(count > 0)
  {
    if(remainingInChunk <= 0)
    {
      // start a new chunk
      remainingInChunk += CHUNK_PERIOD;
      updateParamsForNextChunk();
    }

    // process chunk
    int subCount = std::min(remainingInChunk, count);
    runChunk(samples, subCount);

    samples += subCount;
    count -= subCount;
    remainingInChunk -= subCount;
  }
}

void Synth::updateParamsForNextChunk()
{
  for(auto& voice : voices)
  {
    if(voice.vol == 0)
      continue;

    double pitch = voice.pitch + config.PitchBendDelta;
    pitch += sin(lfoPhase * 2.0 * M_PI) * config.LfoAmount;

    voice.aux_freq = pitchToFreq(pitch - 69) * 440.0 / SAMPLERATE;
    voice.osc.pwm = clamp(config.PWM, 0.0, 1.0);
  }

  lfoPhase += 20.0 * config.LfoRate * CHUNK_PERIOD / SAMPLERATE;

  if(lfoPhase >= 1)
    lfoPhase -= 1;
}

void Synth::runChunk(float* samples, int count)
{
  for(auto& voice : voices)
  {
    if(voice.vol == 0)
      continue;

    const int oscType = floor(config.OscType);

    for(int i = 0; i < count; ++i)
      samples[i] += voice.osc.work(oscType, voice.aux_freq) * voice.vol;
  }

  for(int i = 0; i < count; ++i)
  {
    float s = samples[i];

    s = s * (config.Volume * 0.1);

    // slight saturation
    s = atan(s) / (M_PI * 0.5);

    samples[i] = clamp(s, -1.0, 1.0);
  }
}

void Synth::pushCommand(const Command& cmd) { commandQueue.push(cmd); }

void Synth::processPendingCommands()
{
  Command cmd;
  while(commandQueue.pop(cmd))
  {
    switch(cmd.type)
    {
    case Command::NoteOn:
      noteOn(cmd.value1);
      break;
    case Command::NoteOff:
      noteOff(cmd.value1);
      break;
    case Command::ConfigChange:
    {
      auto& info = ConfigTypeInfo[cmd.value1];
      *((double*)(((uint8_t*)&config) + info.offset)) = cmd.value2;
    }
    break;
    }
  }
}

void Synth::noteOn(int note)
{
  int i = 0;

  // ignore invalid notes
  if(note < 0 || note >= 128)
    return;

  // find free voice, steal the last one in the worst case
  while(voices[i].vol > 0 && i < MaxVoices - 1)
    ++i;

  noteToVoice[note] = i;
  auto& voice = voices[i];
  voice.pitch = note;
  voice.vol = 1.0;

  voice.osc.phase = 0; // retrigger
}

void Synth::noteOff(int note)
{
  int i = noteToVoice[note];
  auto& voice = voices[i];
  voice.vol = 0;
}
