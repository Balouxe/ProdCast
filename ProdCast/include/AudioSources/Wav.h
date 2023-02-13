#pragma once
#include "Core.h"
#include "AudioSource.h"

namespace ProdCast {

	class Wav{
	public:
		static bool LoadWavFile(std::filesystem::path& path, float** data, unsigned int* channels, unsigned int* sampleRate, uint64_t* length);

		// TODO: static bool SaveWavFile()
	};

}