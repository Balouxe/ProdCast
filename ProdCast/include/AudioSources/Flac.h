#pragma once
#include "Core.h"
#include "AudioFile.h"

namespace ProdCast {

	namespace Flac{
		bool LoadFlacFile(std::filesystem::path& path, float** data, unsigned int* channels, unsigned int* sampleRate, uint64_t* length);

		// TODO: static bool SaveFlacFile()
	};

}