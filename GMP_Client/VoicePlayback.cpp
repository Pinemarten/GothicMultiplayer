#include "VoicePlayback.h"
#include <mutex>
#include <spdlog/spdlog.h>

std::mutex voicePlaybackBufferMutex;
std::atomic_bool stopPlaybackThread = false;

using namespace std;
VoicePlayback::VoicePlayback()
{
  out = NULL;
  SDL_zero(audioSpec);
}

VoicePlayback::~VoicePlayback()
{
  stopPlaybackThread = true;
  SDL_PauseAudioDevice(out, SDL_TRUE);
  SDL_CloseAudioDevice(out);
  ClearBuffers();
}

bool VoicePlayback::StartPlayback()
{
  out = SDL_OpenAudioDevice(0, SDL_FALSE, &audioSpec, &audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if (out <= 0) {
    SPDLOG_ERROR("Failed to open playback device, error: {}", SDL_GetError());
    return false;
  }
  SDL_PauseAudioDevice(out, SDL_FALSE);
  if (out > 0) {
    stopPlaybackThread = false;
    loopThread = thread(&VoicePlayback::Loop, this);
    loopThread.detach();
  }
  return out > 0;
}

void VoicePlayback::PlayVoice(char* buffer, size_t size)
{
  const lock_guard<mutex> lock(voicePlaybackBufferMutex);
  voiceBuffers.push_back(VoiceBuffer{buffer, size});
}

void VoicePlayback::Loop()
{
  do {
    voicePlaybackBufferMutex.lock();
    if (voiceBuffers.empty()) {
      voicePlaybackBufferMutex.unlock();
      SDL_Delay(16);
      continue;
    }
    auto voiceBuffer = voiceBuffers.front();
    voiceBuffers.pop_front();
    voicePlaybackBufferMutex.unlock();
    
    SDL_QueueAudio(out, voiceBuffer.data, voiceBuffer.size);
    delete[] voiceBuffer.data;
  } while (!stopPlaybackThread);
}

void VoicePlayback::ClearBuffers()
{
  const lock_guard<mutex> lock(voicePlaybackBufferMutex);
  while (!voiceBuffers.empty()) {
    auto voiceBuffer = voiceBuffers.front();
    voiceBuffers.pop_front();
    delete[] voiceBuffer.data;
  }
}