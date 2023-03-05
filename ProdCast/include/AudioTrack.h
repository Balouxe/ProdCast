#pragma once
#include "Core.h"
#include "AudioThread.h"
#include "ProcessingChain.h"

namespace ProdCast {
	class ProdCastEngine;
	class AudioBus;

	class PC_API AudioTrack : public ThreadableJob {
	public:
		AudioTrack();
		~AudioTrack();
		
		void AddParent(AudioBus* parent, float mix = 1.0f);
		void RemoveParent(AudioBus* parent);
		void SetParentWeight(AudioBus* parent, float mix);

		inline float* GetBuffer() { return m_buffer; };

		void ApplyProcessingChain(ProcessingChain* processingChain);
		ProcessingChain* getProcessingChain();

		void setVolume(float volume);
		float getVolume();

		void setPan(float pan);
		float getPan();

		void Mute();
		void Unmute();

		void SetRMSRefreshRate(uint32_t refreshRate);
		float* GetRMS();

		void Process();
		virtual void GetNextSamples(float* buffer, unsigned int bufferSize, unsigned int numChannels) = 0;
	protected:
		void Init();
		void InitWithoutParent();

		void CalculateRMS();

		std::mutex m_mutex;
		AudioBus* m_parent = nullptr;
		uint16_t m_busHandle;

		ProdCastEngine* m_engine = nullptr;
		AudioSettings* m_settings = nullptr;

		ProcessingChain* m_processingChain = nullptr;

		float* m_buffer = nullptr;

		float m_volume = 1.0f;
		float m_pan = 0.0f;
		float m_gainLeft;
		float m_gainRight;

		bool m_isMuted = false;

		// RMS
		uint32_t m_RMSState = 0;
		uint32_t m_RMSRate = 735;
		std::vector<float> m_RMSValues;
		float* m_RMS;
	};
}