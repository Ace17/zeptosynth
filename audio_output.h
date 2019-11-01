#pragma once

#include "unique.h"

constexpr auto SAMPLERATE = 48000;

struct IAudioOutput
{
  virtual ~IAudioOutput() = default;
};

using AudioCallback = void (float*, int samples, void* userParam);

Unique<IAudioOutput> createAudioOutput(AudioCallback* callback, void* userParam);

