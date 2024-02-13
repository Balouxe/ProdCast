#pragma once
#include "Core.h"

#include "fftw3.h"

#include <unordered_map>

namespace ProdCast {

	// For stereo only at the moment TODO: Mono and stuff
	class FFT {
	public:
		void InitFFTStereo(uint16_t numSamples, uint16_t sampleRate);
		void UninitFFT();

		std::unordered_map<float, float> ComputeFFT(float* in);
		 
	private:
		double* m_in;
		fftw_complex* m_out;
		fftw_plan m_plan;

		uint16_t m_numSamples;
		uint16_t m_sampleRate;
	};

}