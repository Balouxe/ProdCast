#pragma once
#include<iostream>
#include "PortAudio.h"

struct AudioSettings {
	uint32_t sampleRate = 48000;
	uint16_t framesPerBuffer = 128;
	uint8_t channelCount = 2;
	PaSampleFormat sampleFormat = paFloat32;
	PaDeviceIndex deviceIndex;
	const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
	void* apiSpecificInfo = NULL;
	AudioSettings();
	
};

class AudioEngine {
private:

public:
	AudioEngine();
	~AudioEngine();
};