#include "Engine.h"
#include "Logger.h"
#include "Backends/SDLBackend.h"
#include "Backends/PortAudioBackend.h"

#include <iostream>

namespace ProdCast {

	ProdCastEngine::ProdCastEngine(AudioSettings& settings) {
		Logger::Init();
		m_masterGain = 1.0f;

		switch (settings.audioBackend) {
		case(BE_SDL):
			// m_backend = new SDLBackend();
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
		m_threadPool = new ThreadPool();
		m_threadPool->InitPool(1);
		m_backend->Init(&m_mainStream.settings, this);
		m_mainStream.InitStream(this, m_mainStream.settings, 0u);

		isInit = true;
	}

	ProdCastEngine::~ProdCastEngine() {
		m_threadPool->DeInitPool();
		delete m_threadPool;
		m_backend->DeInit();
		delete m_backend;
	}

	int ProdCastEngine::AudioCallback(float* outputBuffer, float* inputBuffer, unsigned long framesCount, uint32_t auxHandle) {
		if (!isInit)
			return 0;
		float* out = (float*)outputBuffer;
		float* in = (float*)inputBuffer;

		Stream& stream = GetStream(auxHandle);

		if (stream.inputCallback) {
			stream.inputCallback(in, stream.settings.bufferSize, stream.settings.inputChannels);
		}
		
		stream.ringBuffer->Read(out);

		m_threadPool->addJob(stream.bus);
		
		return 0;
	}

	void ProdCastEngine::RenderBuffer(uint32_t streamHandle) {
		unsigned long i;

		Stream* stream;
		if (streamHandle == 0) {
			stream = &m_mainStream;
		}
		else {
			stream = &m_auxStreams[streamHandle];
		}

		float* buffer = stream->bus->GetBuffer();

		if(stream->bus->getProcessingChain())
			stream->bus->getProcessingChain()->ProcessEffects(buffer, stream->settings.bufferSize, stream->settings.outputChannels);

		for (i = 0; i < stream->settings.bufferSize * stream->settings.outputChannels; i++) {
			stream->outputBuffer[i] = std::min(std::max(buffer[i] * m_masterGain, -1.0f), 1.0f);
		}

		stream->ringBuffer->Write(stream->outputBuffer);
	}

	uint32_t ProdCastEngine::OpenAuxiliaryStream(AudioSettings& settings) {
		Stream stream = Stream(this, settings, m_auxHandleCount);
		stream.settings = settings;
		m_backend->OpenAuxiliaryStream(m_auxHandleCount, settings);
		m_auxStreams[m_auxHandleCount] = stream;
		m_auxHandleCount++;
		return m_auxHandleCount - 1;
	}

	void ProdCastEngine::CloseAuxiliaryStream(uint32_t handle) {
		m_backend->CloseAuxiliaryStream(handle);
		m_auxStreams.erase(handle);
	}

	void ProdCastEngine::setMasterGain(float gain) {
		m_masterGain = gain; 
	}

	void ProdCastEngine::setTempo(uint16_t tempo) {
		// todo
	}
	ProdCastEngine::Stream& ProdCastEngine::GetStream(uint32_t streamHandle) {
		if (streamHandle == 0) {
			return m_mainStream;
		}
		else {
			return m_auxStreams[streamHandle];
		}
	}

	AudioSettings* ProdCastEngine::getAudioSettings(uint32_t streamHandle) {
		return &GetStream(streamHandle).settings;
	}

	void ProdCastEngine::SetInputCallback(void(*inputCallback)(float*, unsigned int, unsigned int), uint32_t streamHandle) {
		GetStream(streamHandle).inputCallback = inputCallback;
	}

	// Stream

	ProdCastEngine::Stream::Stream(ProdCastEngine* engine, AudioSettings& settings, uint32_t handle) {
		InitStream(engine, settings, handle);
	}

	ProdCastEngine::Stream::~Stream() {
		delete bus;
		delete ringBuffer;
		delete[] outputBuffer;
		delete[] inputBuffer;
	}

	void ProdCastEngine::Stream::InitStream(ProdCastEngine* engine, AudioSettings& settings, uint32_t handle) {
		streamHandle = handle;
		bus = new AudioBus(engine, streamHandle, true);
		ringBuffer = new RingBuffer(&settings);
		outputBuffer = new float[settings.outputChannels * settings.bufferSize];
		inputBuffer = new float[settings.inputChannels * settings.bufferSize];
		memset(outputBuffer, 0, settings.bufferSize * settings.outputChannels * sizeof(float));
		memset(inputBuffer, 0, settings.bufferSize * settings.outputChannels * sizeof(float));
	}
}