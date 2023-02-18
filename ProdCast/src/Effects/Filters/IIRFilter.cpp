#include "Effects/Filters/IIRFilter.h";
#include "Logger.h"
#include "Engine.h"

#define M_PI 3.14

namespace ProdCast {

	IIRFilter::IIRFilter(ProdCastEngine* engine, FilterType type) {
		m_engine = engine;
		m_settings = engine->getAudioSettings();
		m_type = type;
		m_frequency = 0.0f;
		m_Q = 1.0f;
		m_gain = 0.0f;
		calculateIntermediates();
		m_pastValues = new pastValues[m_settings->outputChannels];
		for (int i = 0; i < m_settings->outputChannels; i++) {
			m_pastValues[i].x = 0.0f;
			m_pastValues[i].xnm1 = 0.0f;
			m_pastValues[i].xnm2 = 0.0f;
			m_pastValues[i].y = 0.0f;
			m_pastValues[i].ynm1 = 0.0f;
			m_pastValues[i].ynm2 = 0.0f;
		}
	}
	IIRFilter::IIRFilter(ProdCastEngine* engine, FilterType type, float frequency, float gain, float Q) {
		m_engine = engine;
		m_settings = engine->getAudioSettings();
		m_type = type;
		m_frequency = frequency;
		m_Q = Q;
		m_gain = gain;
		calculateIntermediates();
		m_pastValues = new pastValues[m_settings->outputChannels];
		for (int i = 0; i < m_settings->outputChannels; i++) {
			m_pastValues[i].x = 0.0f;
			m_pastValues[i].xnm1 = 0.0f;
			m_pastValues[i].xnm2 = 0.0f;
			m_pastValues[i].y = 0.0f;
			m_pastValues[i].ynm1 = 0.0f;
			m_pastValues[i].ynm2 = 0.0f;
		}
	}


	IIRFilter::~IIRFilter() {
		delete[] m_pastValues;
	}

	void IIRFilter::ProcessBuffer(float* buffer, unsigned int samplesToGo, unsigned int nbChannels) {

		for (int i = 0; i < samplesToGo * nbChannels; i+= nbChannels) {
			for (int j = 0; j < nbChannels; j++) {
				m_pastValues[j].x = buffer[i + j];

				m_pastValues[j].y = (b0 * m_pastValues[j].x
					+ b1 * m_pastValues[j].xnm1
					+ b2 * m_pastValues[j].xnm2
					- a1 * m_pastValues[j].ynm1
					- a2 * m_pastValues[j].ynm2) / a0;

				m_pastValues[j].xnm2 = m_pastValues[j].xnm1;
				m_pastValues[j].xnm1 = m_pastValues[j].x;
				m_pastValues[j].ynm2 = m_pastValues[j].ynm1;
				m_pastValues[j].ynm1 = m_pastValues[j].y;

				buffer[i + j] = m_pastValues[j].y;
			}
		}
	}

	void IIRFilter::calculateIntermediates() {
		float A = pow(10, m_gain / 40);
		float omega = 2 * M_PI * m_frequency / m_settings->sampleRate;
		float sn = sin(omega);
		float cs = cos(omega);
		float alpha = sn / (2 * m_Q);
		float beta = sqrt(A + A);

		switch (m_type) {
		case(LowPass):
			b0 = (1.0f - cs) / 2.0f;
			b1 = 1.0f - cs;
			b2 = (1.0f - cs) / 2.0f;
			a0 = 1.0f + alpha;
			a1 = -2.0f * cs;
			a2 = 1.0f - alpha;
			break;
		case(HighPass):
			b0 = (1.0f + cs) / 2.0f;
			b1 = -(1.0f + cs);
			b2 = (1.0f + cs) / 2.0f;
			a0 = 1.0f + alpha;
			a1 = -2.0f * cs;
			a2 = 1.0f - alpha;
			break;
		case(LowShelf):
			b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
			b1 = 2 * A * ((A - 1) - (A + 1) * cs);
			b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
			a0 = (A + 1) + (A - 1) * cs + beta * sn;
			a1 = -2 * ((A - 1) + (A + 1) * cs);
			a2 = (A + 1) + (A - 1) * cs - beta * sn;
			break;
		case(HighShelf):
			b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
			b1 = -2 * A * ((A - 1) + (A + 1) * cs);
			b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
			a0 = (A + 1) - (A - 1) * cs + beta * sn;
			a1 = 2 * ((A - 1) - (A + 1) * cs);
			a2 = (A + 1) - (A - 1) * cs - beta * sn;
			break;
		case(Peak):
			b0 = 1 + (alpha * A);
			b1 = -2 * cs;
			b2 = 1 - (alpha * A);
			a0 = 1 + (alpha / A);
			a1 = -2 * cs;
			a2 = 1 - (alpha / A);
			break;
		case(Notch):
			b0 = 1;
			b1 = -2 * cs;
			b2 = 1;
			a0 = 1 + alpha;
			a1 = -2 * cs;
			a2 = 1 - alpha;
			break;
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

	void IIRFilter::setType(FilterType type) {
		m_type = type;
		calculateIntermediates();
	}

	IIRFilter::FilterType IIRFilter::getType() {
		return m_type;
	}

}