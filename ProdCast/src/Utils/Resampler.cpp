#include "Utils/Resampler.h"
#include "Utils/Logger.h"

namespace ProdCast {
	SRC_STATE* Resampler::m_handle = nullptr;

	void Resampler::InitResampler(int numChannels) {
		int error;
		m_handle = src_new(SRC_LINEAR, numChannels, &error);
	}

	void Resampler::UninitResampler() {
		src_delete(m_handle);
	}

	void Resampler::Resample(float* sampleIn, float* sampleOut, uint16_t sampleRateIn, uint16_t sampleRateOut, uint32_t inputSampleSize, uint32_t outputSampleSize) {
		int error;

		SRC_DATA data = SRC_DATA();
		data.data_in = sampleIn;
		data.data_out = sampleOut;
		data.input_frames = inputSampleSize;
		data.output_frames = outputSampleSize;
		data.src_ratio = (double)sampleRateOut / (double)sampleRateIn;
		data.end_of_input = 0;

		error = src_process(m_handle, &data);

		/*for (uint32_t i = 0; i < outputSampleSize * 2; i++) {
			sampleOut[i] = sampleIn[i];
		}*/

		// error checking

		if (error != 0) {
			PC_ERROR("Failed to resample, error code {0} !", error);
			PC_WARN("Error message: {0}", src_strerror(error));
		}

		src_reset(m_handle);
	}
}