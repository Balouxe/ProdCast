#include "Wav.h"
#include "Prodcast/Engine.h"
#include "Prodcast/utils/Logger.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

namespace ProdCast {

	bool Wav::LoadWavFile(std::filesystem::path path, float** data, unsigned int* channels, unsigned int* sampleRate, uint64_t* length) {
		PC_TRACE("Loading file {0}", path.string());
		*data = drwav_open_file_and_read_pcm_frames_f32((const char*)path.generic_string().c_str(), channels, sampleRate, length, NULL);
		if (*data == NULL) {
			// Error opening and reading WAV file.
			PC_ERROR("Error loading wav file!");
			return false;
		}
		PC_TRACE("Loaded file successfully");

		PC_TRACE("Number of channels : {0}", *channels);
		PC_TRACE("Sample Rate : {0}", *sampleRate);
		PC_TRACE("Number of Frames : {0}", *length);

		return true;
	}
}