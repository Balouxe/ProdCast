#pragma once
#include "ProdCast/Core.h"
#include "Prodcast/Engine.h"
#include "ProdCast/backends/AudioBackend.h"

#include "SDL.h"

namespace ProdCast {

	class SDLBackend : public AudioBackend {
	public:
		PCError Init(AudioSettings* settings, ProdCastEngine* engine);
		PCError DeInit();
		static PCError RenderSDL(void* userdata, Uint8* stream, int len);

	private:
		SDL_AudioSpec audioSpec;
	};

}