#include "AudioIO.h"
#include <string>

AudioSettings::AudioSettings(){}

AudioEngine::AudioEngine() {
	static PaError InitErr = Pa_Initialize();
	if (InitErr != paNoError) {
		std::string error = "Error initializing PortAudio : " + std::string(Pa_GetErrorText(InitErr));
		PC_ERROR(error);
	}
	
}
AudioEngine::~AudioEngine() {
}