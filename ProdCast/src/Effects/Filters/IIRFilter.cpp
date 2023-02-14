#include "Effects/Filters/IIRFilter.h";

namespace ProdCast {

	IIRFilter::IIRFilter() {
		xnm1_left = 0.0f;
		xnm2_left = 0.0f;
		ynm1_left = 0.0f;
		ynm2_left = 0.0f;
		xnm1_right = 0.0f;
		xnm2_right = 0.0f;
		ynm1_right = 0.0f;
		ynm2_right = 0.0f;
		m_Q = 0.0f;
		m_gain = 0.0f;
		m_frequency = 0.0f;
	}

	IIRFilter::~IIRFilter() {
		// nothing
	}

	void IIRFilter::ProcessBuffer(float* buffer, unsigned int samplesToGo, unsigned int nbChannels) {
		for (int i = 0; i < samplesToGo * nbChannels; i += nbChannels) {
			x_left = buffer[i];
			y_left = (b0 * x_left + b1 * xnm1_left + b2 * xnm2_left - a1 * ynm1_left - a2 * ynm2_left) / a0;
			xnm2_left = xnm1_left;
			xnm1_left = x_left;
			ynm2_left = ynm1_left;
			ynm1_left = y_left; 
			buffer[i] = y_left;

			x_right = buffer[i+1];
			y_right = (b0 * x_right + b1 * xnm1_right + b2 * xnm2_right - a1 * ynm1_right - a2 * ynm2_right) / a0;
			xnm2_right = xnm1_right;
			xnm1_right = x_right;
			ynm2_right = ynm1_right;
			ynm1_right = y_right;
			buffer[i+1] = y_right;
		}
	}

	void IIRFilter::setFrequency(float frequency) {
		m_frequency = frequency;
		calculateIntermediates();
	}

	float IIRFilter::getFrequency() {
		return m_frequency;
	}

	void IIRFilter::setGain(float gain) {
		m_gain = gain;
		calculateIntermediates();
	}

	float IIRFilter::getGain() {
		return m_gain;
	}

	void IIRFilter::setQ(float Q) {
		m_Q = Q;
		calculateIntermediates();
	}

	float IIRFilter::getQ() {
		return m_Q;
	}

}