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

		m_ringBuffer = new RingBuffer(m_audioSettings.bufferSize * m_audioSettings.outputChannels, m_audioSettings.sampleRate);

		m_threadPool = new ThreadPool();
		m_threadPool->InitPool(1);

		isInit = true;
	}

	ProdCastEngine::~ProdCastEngine() {
		m_threadPool->DeInitPool();
		delete m_threadPool;
		m_backend->DeInit();
		delete m_backend;

		delete m_ringBuffer;
	}

	void ProdCastEngine::AddAudioTrack(AudioTrack* source) {
		m_tracks.push_back(source);
	}

	int ProdCastEngine::AudioCallback(float* outputBuffer, float* inputBuffer, unsigned long framesCount) {
		if (!isInit)
			return 0;
		float* out = (float*)outputBuffer;
		float* in = (float*)inputBuffer;
		unsigned long i;
		
		lockAudioMutex();
		m_ringBuffer->Read(out);

		for (i = 0; i < framesCount * m_audioSettings.outputChannels; i++) {
			
			out[i] = std::min(std::max(out[i] * m_masterGain, -1.0f), 1.0f);
		}
		unlockAudioMutex();

		for (i = 0; i < m_tracks.size(); i++) {
			m_threadPool->addJob(m_tracks[i]);
		}

		return 0;
	}

	// Setters / Getters

	void ProdCastEngine::setMasterGain(float gain) {
		lockAudioMutex();
		m_masterGain = gain; 
		unlockAudioMutex();
	}
}