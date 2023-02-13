#include "AudioSources/Mp3.h"
#include "Engine.h"
#include "Logger.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

namespace ProdCast {

	bool Mp3::LoadMp3File(std::filesystem::path& path, float** data, unsigned int* channels, unsigned int* sampleRate, uint64_t* length) {
		PC_TRACE("Loading file {0}", path.string());
		drmp3_config config;
		*data = drmp3_open_file_and_read_pcm_frames_f32(path.string().c_str(), &config, length, NULL);
		if (*data == NULL) {
			// Error opening and reading WAV file.
			PC_ERROR("Error opening or reading mp3 file.");
			return false;
		}
		*channels = config.channels;
		*sampleRate = config.sampleRate;

		PC_TRACE("Loaded file successfully");
		PC_TRACE("Number of channels : {0}", config.channels);
		PC_TRACE("Sample Rate : {0}", config.sampleRate);
		PC_TRACE("Number of Frames : {0}", *length);

		return true;
	}
}