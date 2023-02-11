#pragma once

#include "ProdCast/AudioTrack.h"
#include <filesystem>

namespace ProdCast {

	class PC_API AudioSource : public AudioTrack {
	public:
		AudioSource(ProdCastEngine* engine);
		~AudioSource();

		void Play() override;

		virtual void Process();
		void GetNextSamples(float* buffer, unsigned int samplesToGo, unsigned int nbChannels);

		void LoadFile(std::filesystem::path path);
	protected:
		float* m_audioData;
		uint64_t m_length;
		size_t m_position;

		unsigned int m_numChannels;
		uint32_t m_sampleRate;

		bool m_isLoaded;
	};

}