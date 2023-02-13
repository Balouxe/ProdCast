#pragma once

#include "AudioTrack.h"
#include <filesystem>
#include <functional>

namespace ProdCast {

	class PC_API AudioSource : public AudioTrack {
	public:
		AudioSource(ProdCastEngine* engine);
		~AudioSource();

		void Play();
		void Stop();

		void Process();
		void GetNextSamples(float* buffer, unsigned int samplesToGo, unsigned int nbChannels);

		void LoadFile(std::filesystem::path path);
		void LoadFileCustom(std::filesystem::path path, bool (*loader)(std::filesystem::path&, float**, unsigned int*, unsigned int*, uint64_t*));
	protected:
		float* m_audioData;
		uint64_t m_length;
		size_t m_position;

		unsigned int m_numChannels;
		uint32_t m_sampleRate;

		bool m_isLoaded;
		bool m_isPlaying;
	};

}