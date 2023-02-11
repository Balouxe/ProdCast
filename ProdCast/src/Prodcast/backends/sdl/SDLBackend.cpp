#include "SDLBackend.h"

namespace ProdCast {
	PCError SDLBackend::Init(AudioSettings* settings, ProdCastEngine* engine) {
		audioSpec = SDL_AudioSpec();
		audioSpec.freq = settings->sampleRate;
		audioSpec.format = AUDIO_F32;
		audioSpec.channels = (Uint8)settings->outputChannels;
		audioSpec.samples = (Uint16)settings->bufferSize;


		return E_SUCCESS;
	}

	PCError SDLBackend::DeInit() {

		return E_SUCCESS;
	}

	PCError SDLBackend::RenderSDL(void* userdata, Uint8* stream, int len) {

		return E_SUCCESS;
	}
}