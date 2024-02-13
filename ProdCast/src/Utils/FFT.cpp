#include "Utils/FFT.h"

namespace ProdCast {

	void FFT::InitFFTStereo(uint16_t numSamples, uint16_t sampleRate) {
		m_numSamples = numSamples;
		m_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * numSamples);
		m_in = new double[numSamples * 2];
		m_plan = fftw_plan_dft_r2c_1d(numSamples, m_in, m_out, FFTW_ESTIMATE);
		m_sampleRate = sampleRate;
	}

	void FFT::UninitFFT() {
		fftw_destroy_plan(m_plan);
		delete[] m_in;
		fftw_free(m_out);
	}

	std::unordered_map<float, float> FFT::ComputeFFT(float* inputBuffer) {
		for (uint16_t i = 0; i < m_numSamples * 2; i++) {
			m_in[i] = (double)inputBuffer[i];
		}

		fftw_execute(m_plan);

		std::unordered_map<float, float> result;

		float Fs = m_sampleRate / m_numSamples;
	

	    return result;
	}

}