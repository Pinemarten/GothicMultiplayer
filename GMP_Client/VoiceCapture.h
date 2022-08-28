#pragma once

#include <SDL.h>
#include <vector>
#include <string>
#include <thread>

class VoiceCapture
{
public:
  VoiceCapture();
  ~VoiceCapture();

  bool StartCapture();
  bool GetAndFlushVoiceBuffer(char*, int&);
  int GetNumberOfChannels() const;

  std::vector<std::string> GetInputDevices();

private:
  SDL_AudioDeviceID in;
  char* voiceBuffer;
  size_t voiceBufferSize;
  void Init();
  void Loop();

  SDL_AudioSpec wantedAudioSpec;
  SDL_AudioSpec audioSpec;
  std::thread loopThread;
};