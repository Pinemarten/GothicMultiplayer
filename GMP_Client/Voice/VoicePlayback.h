#pragma once

#include <SDL.h>
#include <deque>
#include <thread>

class VoicePlayback
{
public:
  VoicePlayback();
  ~VoicePlayback();

  void PlayVoice(char*, size_t);
  bool StartPlayback();

private:
  struct VoiceBuffer
  {
    char* data;
    size_t size;
  };

  std::deque<VoiceBuffer> voiceBuffers;
  std::thread loopThread;

  SDL_AudioDeviceID out;
  SDL_AudioSpec audioSpec;

  void ClearBuffers();
  void Loop();
};