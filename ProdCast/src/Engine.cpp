#include "Engine.h"
#include "Utils/Logger.h"
// #include "Backends/SDLBackend.h"
#include "Backends/PortAudioBackend.h"
#include "Utils/Resampler.h"

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
		m_backend->Init(&settings, this);
		m_mainStream.InitStream(this, settings, 0u);
		m_mainStream.settings = settings;
		
		Resampler::InitResampler(2); // stereo for now

		isInit = true;
	}

	ProdCastEngine::~ProdCastEngine() {
		m_threadPool->DeInitPool();
		delete m_threadPool;
		m_backend->DeInit();
		delete m_backend;
		Resampler::UninitResampler();
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

	void ProdCastEngine::SetMasterGain(float gain) {
		m_masterGain = gain;
	}

	void ProdCastEngine::SetTempo(uint16_t tempo) {
		m_mainStream.settings.tempo = tempo;
	}
	ProdCastEngine::Stream& ProdCastEngine::GetStream(uint32_t streamHandle) {
		if (streamHandle == 0u) {
			return m_mainStream;
		}
		else {
			return m_auxStreams[streamHandle];
		}
	}

	AudioBus* ProdCastEngine::GetMasterBus(uint32_t streamHandle) {
		return GetStream(streamHandle).bus;
	}


	AudioSettings* ProdCastEngine::GetAudioSettings(uint32_t streamHandle) {
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

	void ProdCastEngine::Stream::InitStream(ProdCastEngine* engine, AudioSettings& a_settings, uint32_t handle) {
		settings = a_settings;
		streamHandle = handle;
		ringBuffer = new RingBuffer(&settings);
		outputBuffer = new float[settings.outputChannels * settings.bufferSize];
		inputBuffer = new float[settings.inputChannels * settings.bufferSize];
		memset(outputBuffer, 0, settings.bufferSize * settings.outputChannels * sizeof(float));
		memset(inputBuffer, 0, settings.bufferSize * settings.outputChannels * sizeof(float));
		bus = new AudioBus(engine, streamHandle, true);
	}

	AudioSettings& ProdCastEngine::Stream::GetAudioSettings() {
		return settings;
	}

	std::unordered_map<uint32_t, DeviceInfo> ProdCastEngine::GetOutputDevices() {
		return m_backend->GetOutputDevices();
	}

	std::unordered_map<uint32_t, DeviceInfo> ProdCastEngine::GetInputDevices() {
		return m_backend->GetInputDevices();
	}

	void ProdCastEngine::SetOutputDevice(uint32_t streamHandle, uint32_t deviceId) {
		m_backend->ChangeOutputDevice(streamHandle, deviceId);
	}
	void ProdCastEngine::SetInputDevice(uint32_t streamHandle, uint32_t deviceId) {
		m_backend->ChangeInputDevice(streamHandle, deviceId);
	}

	void ProdCastEngine::Enable3D(bool enabled) {
		if (m_enable3D != enabled) {
			m_enable3D = enabled;

			Set3DListenerPosition(0, 0, 0);
			Set3DListenerLookingAt(1, 0, 0);
			Set3DUpDirection(0, 0, 1);

			// refresh panning and volume multiplier

			for (auto track : m_mainStream.bus->GetTracks()) {
				track.second->Enable3D();
				track.second->Calculate3D();
			}
		}
	}

	void ProdCastEngine::Set3DListenerPosition(float X, float Y, float Z) {
		m_listenerPosition = Vec3(X, Y, Z);
		for (auto track : m_mainStream.bus->GetTracks()) {
			track.second->Calculate3D();
		}
	}
	void ProdCastEngine::Set3DListenerLookingAt(float X, float Y, float Z) {
		m_listenerLookingAt = Vec3(X, Y, Z);
		for (auto track : m_mainStream.bus->GetTracks()) {
			track.second->Calculate3D();
		}
	}

	void ProdCastEngine::Set3DUpDirection(float X, float Y, float Z) {
		m_listenerUpDirection = Vec3(X, Y, Z);
		for (auto track : m_mainStream.bus->GetTracks()) {
			track.second->Calculate3D();
		}
	}

	Vec3& ProdCastEngine::Get3DListenerPosition() {
		return m_listenerPosition;
	}

	Vec3& ProdCastEngine::Get3DListenerLookingAt() {
		return m_listenerLookingAt;
	}

	Vec3& ProdCastEngine::Get3DUpDirection() {
		return m_listenerUpDirection;
	}

	void ProdCastEngine::TestRingBufferCrash() {
		m_mainStream.ringBuffer->SimulateCrash();
	}
}