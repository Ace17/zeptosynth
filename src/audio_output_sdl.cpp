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
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    SDL_AudioSpec audio{};
    audio.freq = SAMPLERATE;
    audio.format = AUDIO_F32;
    audio.channels = 1;
    audio.samples = 128;
    audio.callback = mixAudio;
    audio.userdata = this;

    SDL_AudioSpec actual;

    m_deviceId = SDL_OpenAudioDevice(nullptr, 0, &audio, &actual, 0);
    if(!m_deviceId)
    {
      static char buffer[256];
      sprintf(buffer, "Can't open audio device: %s", SDL_GetError());
      throw Exception{buffer};
    }

    printf("[audio] %d Hz %d channels, 0x%.4X (%d samples)\n", actual.freq, actual.channels, actual.format,
          actual.samples);

    SDL_PauseAudioDevice(m_deviceId, 0);
  }

  ~SdlAudioOutput()
  {
    SDL_CloseAudioDevice(m_deviceId);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    printf("[audio] shutdown\n");
  }

  static void mixAudio(void* userData, Uint8* stream, int len)
  {
    auto pThis = (SdlAudioOutput*)userData;
    pThis->mixAudioFloat((float*)stream, len / sizeof(float));
  }

  void mixAudioFloat(float* buffer, int len) { m_callback(buffer, len, m_userParam); }

  AudioCallback* const m_callback;
  void* const m_userParam;
  SDL_AudioDeviceID m_deviceId;
};
}

Unique<IAudioOutput> createAudioOutput(AudioCallback* callback, void* userParam)
{
  return makeUnique<SdlAudioOutput>(callback, userParam);
}
