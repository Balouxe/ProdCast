#include "Effects/Filters/PeakFilter.h"
#include "Engine.h"

#include <math.h>

#define M_PI 3.14

namespace ProdCast {

	PeakFilter::PeakFilter(ProdCastEngine* engine) {
		m_engine = engine;
		m_settings = engine->getAudioSettings();
		m_frequency = 0.0f;
		m_Q = 1.0f;
		m_gain = 0.0f;
		calculateIntermediates();
	}

	PeakFilter::PeakFilter(ProdCastEngine* engine, float frequency, float Q, float gain) {
		m_frequency = frequency;
		m_Q = Q;
		m_gain = gain;
		m_engine = engine;
		m_settings = engine->getAudioSettings();
		calculateIntermediates();
	}

	void PeakFilter::calculateIntermediates() {
		float A, omega, sn, cs, alpha;

		A = powf(10, m_gain / 40.0f);
		omega = (2 * M_PI * m_frequency) / m_settings->sampleRate;
		sn = sinf(omega);
		cs = cosf(omega);
		alpha = sn / (2.0f * m_Q);

		b0 = 1 + (alpha * A);
		b1 = -2 * cs;
		b2 = 1 - alpha * A;
		a0 = 1 + (alpha / (float)A);
		a1 = -2 * cs;
		a2 = 1 - (alpha / (float)A);
	}

}