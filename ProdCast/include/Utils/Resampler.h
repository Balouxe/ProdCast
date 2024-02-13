#pragma once
#include "Core.h"

#include "samplerate.h"

namespace ProdCast {

	class Resampler {
	public:
		static void InitResampler(int numChannels);
		static void UninitResampler();

		static void Resample(float* sampleIn, float* sampleOut, uint16_t sampleRateIn, uint16_t samplRateOut, uint32_t inputSampleSize, uint32_t outputSampleSize);
	private:
		static SRC_STATE* m_handle;
	};
}