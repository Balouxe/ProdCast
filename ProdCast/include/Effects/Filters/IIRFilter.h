#pragma once
#include "Core.h"
#include "Effects/Effect.h"

namespace ProdCast {

	class PC_API IIRFilter : public Effect {
	public:
		enum FilterType {
			Peak,
			Notch,
			LowPass,
			HighPass,
			LowShelf,
			HighShelf
		};

		IIRFilter(ProdCastEngine* engine, FilterType type);
		IIRFilter(ProdCastEngine* engine, FilterType type, uint32_t frequency, float gain, float Q);
		~IIRFilter();

		virtual void calculateIntermediates();

		void setFrequency(uint32_t frequency);
		uint32_t getFrequency();

		void setGain(float gain);
		float getGain();

		void setQ(float Q);
		float getQ();

		void setType(FilterType type);
		FilterType getType();

		void ProcessBuffer(float* buffer, unsigned int samplesToGo, unsigned int nbChannels);

	protected:
		struct pastValues {
			float x;
			float xnm1;
			float xnm2;
			float y;
			float ynm1;
			float ynm2;
		};

		FilterType m_type;

		float m_Q;
		float m_gain;
		uint32_t m_frequency;

		pastValues* m_pastValues;

		float a0, a1, a2, b0, b1, b2; // intermediate values

		// check https://www.diva-portal.org/smash/get/diva2:1031081/FULLTEXT01.pdf for more info
	};
}