#pragma once
#include "ProdCast/Core.h"
#include "Prodcast/AudioSources/AudioSource.h"

namespace ProdCast {

	class Mp3{
	public:
		static bool LoadMp3File(std::filesystem::path& path, float** data, unsigned int* channels, unsigned int* sampleRate, uint64_t* length);

		// TODO: static bool SaveMp3File()
	};

}