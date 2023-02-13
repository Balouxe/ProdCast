#pragma once
#include "Core.h"
#include "Engine.h"
#include "AudioBackend.h"

#include "portaudio.h"

namespace ProdCast {

	class PortAudioBackend : public AudioBackend {
	public:
		PCError Init(AudioSettings* settings, ProdCastEngine* engine);
		PCError DeInit();
		static int RenderPortAudio(const void* /*input*/,
			void* output,
			unsigned long frameCount,
			const PaStreamCallbackTimeInfo* /*timeInfo*/,
			PaStreamCallbackFlags /*statusFlags*/,
			void* userData);
	private:
		PaStream* m_stream;
		PaStreamParameters m_outputParameters;
		PaStreamParameters m_inputParameters;
	};

}