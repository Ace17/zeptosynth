#include "SDL.h"
#include "audio_output.h"

namespace
{
struct SdlAudioOutput : IAudioOutput
{
  SdlAudioOutput(AudioCallback* callback, void* userParam)
      : m_callback(callback)
      , m_userParam(userParam)
  {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_AudioSpec audio{};
    audio.freq = SAMPLERATE;
    audio.format = AUDIO_F32;
    audio.channels = 1;
    audio.samples = 1024;
    audio.callback = mixAudio;
    audio.userdata = this;

    SDL_AudioSpec actual;

    if(SDL_OpenAudio(&audio, &actual) < 0)
      throw Exception{"Can't open audio"};

    printf("[audio] %d Hz %d channels, 0x%.4X (%d samples)\n", actual.freq, actual.channels, actual.format,
          actual.samples);

    SDL_PauseAudio(0);
  }

  ~SdlAudioOutput()
  {
    SDL_CloseAudio();
    SDL_Quit();
  }

  static void mixAudio(void* userData, Uint8* stream, int len)
  {
    auto pThis = (SdlAudioOutput*)userData;
    pThis->mixAudioFloat((float*)stream, len / sizeof(float));
  }

  void mixAudioFloat(float* buffer, int len) { m_callback(buffer, len, m_userParam); }

  AudioCallback* const m_callback;
  void* const m_userParam;
};
}

Unique<IAudioOutput> createAudioOutput(AudioCallback* callback, void* userParam)
{
  return makeUnique<SdlAudioOutput>(callback, userParam);
}
