#pragma once
#include "Core.h"
#include "AudioBackend.h"
#include "AudioThread.h"
#include "AudioTrack.h"
#include "AudioBus.h"
#include "RingBuffer.h"

#include <string>
#include <mutex>
#include <vector>
#include <unordered_map> // for audio sources

namespace ProdCast {
	class PC_API ProdCastEngine {
	public:
		// PUBLIC INTERFACE

		ProdCastEngine(unsigned int bufferSize, unsigned int sampleRate, Backends backend);
		~ProdCastEngine();

		void AddAudioTrack(AudioTrack* source);
		
		void setMasterGain(float gain);
		inline float getMasterGain() const { return m_masterGain; }

		inline AudioSettings* getAudioSettings() { return &m_audioSettings; }

		// PRIVATE INTERFACE

		inline RingBuffer* getRingBuffer() { return m_ringBuffer; }
		inline ThreadPool* getPool() const { return m_threadPool; }
		inline AudioBus* getMasterBus() { return m_masterBus; }

		int AudioCallback(float* outputBuffer, float* inputBuffer, unsigned long frameCount);

		void RenderBuffer();

		inline void lockAudioMutex() {	m_audioMutex.lock(); }
		inline void unlockAudioMutex() { m_audioMutex.unlock(); }

	private:
		std::mutex m_audioMutex;
		ThreadPool* m_threadPool;
		AudioBackend* m_backend;
		AudioBus* m_masterBus;
		
		AudioSettings m_audioSettings;

		RingBuffer* m_ringBuffer;
		float* m_buffer;

		float m_masterGain;	

		std::string m_audioDeviceName;

		bool isInit = false;
	};
}

