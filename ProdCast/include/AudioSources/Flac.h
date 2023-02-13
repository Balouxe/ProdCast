#pragma once
#include "Core.h"
#include "AudioSource.h"

namespace ProdCast {

	class Flac{
	public:
		static bool LoadFlacFile(std::filesystem::path& path, float** data, unsigned int* channels, unsigned int* sampleRate, uint64_t* length);

		// TODO: static bool SaveFlacFile()
	};

}