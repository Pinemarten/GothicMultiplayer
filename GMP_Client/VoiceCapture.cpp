#include "VoiceCapture.h"
#include <mutex>
#include <spdlog/spdlog.h>

const int AUDIO_CHANNELS = 2;
std::mutex voiceCaptureBufferMutex;
std::atomic_bool stopCaptureThread = false;

using namespace std;
VoiceCapture::VoiceCapture()
{
  in = NULL;
  voiceBufferSize = 0;
  voiceBuffer = new char[480 * sizeof(float) * AUDIO_CHANNELS * 4096];
  memset(voiceBuffer, 0, 480 * sizeof(float) * AUDIO_CHANNELS * 4096);
  Init();
}

VoiceCapture::~VoiceCapture()
{
  stopCaptureThread = true;
  SDL_PauseAudioDevice(in, SDL_TRUE);
  SDL_CloseAudioDevice(in);
  delete[] voiceBuffer;
}

void VoiceCapture::Init()
{
  SDL_zero(wantedAudioSpec);
  wantedAudioSpec.freq = 48000;
  wantedAudioSpec.format = AUDIO_F32SYS;
  wantedAudioSpec.channels = AUDIO_CHANNELS;
  wantedAudioSpec.samples = 4096;
  wantedAudioSpec.callback = NULL;
  SDL_zero(audioSpec);
}

vector<string> VoiceCapture::GetInputDevices()
{
  vector<string> result;
  for (int i = 0; i < SDL_GetNumAudioDevices(SDL_TRUE); i++) {
    result.push_back(SDL_GetAudioDeviceName(i, SDL_TRUE));
  }
  return result;
}

bool VoiceCapture::StartCapture()
{
  in = SDL_OpenAudioDevice(0, SDL_TRUE, &wantedAudioSpec, &audioSpec, 0);
  if (in <= 0) {
    SPDLOG_ERROR("Failed to open capture device, error: {}", SDL_GetError());
    return false;
  }
  SDL_PauseAudioDevice(in, SDL_FALSE);
  if (in > 0) {
    stopCaptureThread = false;
    loopThread = thread(&VoiceCapture::Loop, this);
    loopThread.detach();
  }
  return in > 0;
}

void VoiceCapture::Loop()
{
  do {
    Uint8 buf[480 * sizeof(float) * AUDIO_CHANNELS];
    const Uint32 br = SDL_DequeueAudio(in, buf, sizeof(buf));

    voiceCaptureBufferMutex.lock();
    memcpy(voiceBuffer + voiceBufferSize, buf, br);
    voiceBufferSize += br;

    if (br < sizeof(buf)) {
      voiceCaptureBufferMutex.unlock();
      SDL_Delay(16);
      continue;
    }
    voiceCaptureBufferMutex.unlock();

  } while (!stopCaptureThread);
}

bool VoiceCapture::GetAndFlushVoiceBuffer(char* buffer, int& size)
{
  const lock_guard<mutex> lock(voiceCaptureBufferMutex);
  if (voiceBufferSize == 0) {
    return false;
  }
  memcpy(buffer, voiceBuffer, voiceBufferSize);
  size = voiceBufferSize;
  memset(voiceBuffer, 0, 480 * sizeof(float) * AUDIO_CHANNELS * 4096);
  voiceBufferSize = 0;
  return true;
}

int VoiceCapture::GetNumberOfChannels() const
{
  return AUDIO_CHANNELS;
}