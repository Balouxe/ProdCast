#pragma once
#include "Core.h"
#include "AudioThread.h"


namespace ProdCast {
	class ProdCastEngine;

	class PC_API AudioTrack : public ThreadableJob {
	public:
		AudioTrack();
		~AudioTrack();

		void setParent(AudioTrack* parent);

		inline float* GetBuffer() { return m_buffer; };

		void setVolume(float volume);
		float getVolume();

		void setPan(float pan);
		float getPan();

		virtual void Play();
		void Stop();

		virtual void Process() = 0;
	protected:
		AudioTrack* m_parent;

		ProdCastEngine* m_engine;
		AudioSettings* m_settings;

		float* m_buffer;

		float m_volume;
		float m_pan;
		float m_gainLeft;
		float m_gainRight;

		bool m_isPlaying;
	};
}