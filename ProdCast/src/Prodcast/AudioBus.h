#pragma once
#include "ProdCast/Core.h"
#include "Prodcast/AudioTrack.h"
#include "Prodcast/Engine.h"

#include <mutex>
#include <vector>

namespace ProdCast {

	class PC_API AudioBus : public AudioTrack {
	public:
		AudioBus(ProdCastEngine* engine);
		~AudioBus();

		void setParent(AudioTrack* parent);

		bool LoadWavFile(const char* file);
		void Play();

		void GetNextSamples(unsigned int samplesToGo, unsigned int nbChannels);

		// Necessary for ThreadableJob
		void Process();
	private:
		AudioTrack* m_parent;

		std::vector<AudioTrack*> m_tracks;

		unsigned int m_numChannels;
		uint32_t m_sampleRate;

		AudioSettings* m_settings;
		ProdCastEngine* m_engine;
	};

}