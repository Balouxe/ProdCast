#pragma once
#include "Core.h"
#include "backends/AudioBackend.h"
#include "AudioThread.h"
#include "AudioTrack.h"
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

		RingBuffer* getRingBuffer() { return m_ringBuffer; };
		inline ThreadPool* getPool() const { return m_threadPool; }

		int AudioCallback(float* outputBuffer, float* inputBuffer, unsigned long frameCount);

		inline void lockAudioMutex() {	m_audioMutex.lock(); }
		inline void unlockAudioMutex() { m_audioMutex.unlock(); }

	private:
		std::mutex m_audioMutex;
		ThreadPool* m_threadPool;
		AudioBackend* m_backend;
		
		AudioSettings m_audioSettings;
		unsigned int m_inputChannels;
		unsigned int m_outputChannels;

		RingBuffer* m_ringBuffer;
		std::vector<AudioTrack*> m_tracks;

		float m_masterGain;	

		std::string m_audioDeviceName;

		bool isInit = false;
	};
}

