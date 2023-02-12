#include "Engine.h"
#include "Utils/Logger.h"
#include "backends/sdl/SDLBackend.h"
#include "backends/portaudio/PortAudioBackend.h"

#include <iostream>

namespace ProdCast {

	ProdCastEngine::ProdCastEngine(unsigned int bufferSize, unsigned int sampleRate, Backends backend) {
		Logger::Init();
		m_audioSettings.audioBackend = backend;
		m_audioSettings.bufferSize = bufferSize;
		m_audioSettings.sampleRate = sampleRate;
		m_masterGain = 1.0f;

		switch (m_audioSettings.audioBackend) {
		case(BE_SDL):
			m_backend = new SDLBackend();
			break;
		case(BE_PORTAUDIO):
			m_backend = new PortAudioBackend();
			break;
		case(BE_RTAUDIO):
			// not implemented
			break;
		case(BE_NONE):
			// not implemented
			break;
		default:
			m_backend = new PortAudioBackend();
			break;
		}
		m_backend->Init(&m_audioSettings, this);

		m_ringBuffer = new RingBuffer(&m_audioSettings);

		m_masterBus = new AudioBus(this, true);

		m_threadPool = new ThreadPool();
		m_threadPool->InitPool(1);

		m_buffer = new float[m_audioSettings.bufferSize * m_audioSettings.outputChannels];
		for (int i = 0; i < m_audioSettings.bufferSize * m_audioSettings.outputChannels; i++) {
			m_buffer[i] = 0.0f;
		}

		isInit = true;
	}

	ProdCastEngine::~ProdCastEngine() {
		m_threadPool->DeInitPool();
		delete m_threadPool;
		m_backend->DeInit();
		delete m_backend;

		delete m_ringBuffer;
	}

	int ProdCastEngine::AudioCallback(float* outputBuffer, float* inputBuffer, unsigned long framesCount) {
		if (!isInit)
			return 0;
		float* out = (float*)outputBuffer;
		float* in = (float*)inputBuffer;
		
		m_ringBuffer->Read(out);

		m_threadPool->addJob(m_masterBus);
		
		return 0;
	}

	void ProdCastEngine::RenderBuffer() {
		unsigned long i;

		float* buffer = m_masterBus->GetBuffer();

		for (i = 0; i < m_audioSettings.bufferSize * m_audioSettings.outputChannels; i++) {
			m_buffer[i] = std::min(std::max(buffer[i] * m_masterGain, -1.0f), 1.0f);
		}

		m_ringBuffer->Write(m_buffer);
	}

	// Setters / Getters

	void ProdCastEngine::setMasterGain(float gain) {
		lockAudioMutex();
		m_masterGain = gain; 
		unlockAudioMutex();
	}
}