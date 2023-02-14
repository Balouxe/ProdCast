#pragma once
#include "Core.h"
#include "Effect.h"

namespace ProdCast {

	class PC_API IIRFilter : public Effect {
	public:
		IIRFilter();
		~IIRFilter();

		virtual void calculateIntermediates() = 0; // a0, a1, a2, b0, b1, b2

		void setFrequency(float frequency);
		float getFrequency();

		void setGain(float gain);
		float getGain();

		void setQ(float Q);
		float getQ();

		void ProcessBuffer(float* buffer, unsigned int samplesToGo, unsigned int nbChannels);
	protected:
		float m_Q;
		float m_gain;
		uint32_t m_frequency;


		float x_left, xnm1_left, xnm2_left, y_left, ynm1_left, ynm2_left; // past values
		float x_right, xnm1_right, xnm2_right, y_right, ynm1_right, ynm2_right; // past values
		float a0, a1, a2, b0, b1, b2; // intermediate values

		// check https://www.diva-portal.org/smash/get/diva2:1031081/FULLTEXT01.pdf for more info
	};

}