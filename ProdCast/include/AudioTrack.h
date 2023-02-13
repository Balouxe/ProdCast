#pragma once
#include "Core.h"
#include "AudioThread.h"

namespace ProdCast {
	class ProdCastEngine;
	class AudioBus;

	class PC_API AudioTrack : public ThreadableJob {
	public:
		AudioTrack();
		~AudioTrack();
		
		void setParent(AudioBus* parent);

		inline float* GetBuffer() { return m_buffer; };

		void setVolume(float volume);
		float getVolume();

		void setPan(float pan);
		float getPan();

		void Mute();
		void Unmute();

		virtual void Process() = 0;
	protected:
		AudioBus* m_parent;
		uint16_t m_busHandle;

		ProdCastEngine* m_engine;
		AudioSettings* m_settings;

		float* m_buffer;

		float m_volume;
		float m_pan;
		float m_gainLeft;
		float m_gainRight;

		bool m_isMuted;
	};
}