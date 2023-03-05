#include "Effects/Effect.h"
#include "Logger.h"

namespace ProdCast {

	Effect::Effect() {
		m_engine = nullptr;
		m_settings = nullptr;
		m_buffer = nullptr;
		m_mix = 1.0f;
		m_effectName = "";
	}

	Effect::~Effect() {
		delete[] m_buffer;
	}

	void Effect::Init() {
		m_buffer = new float[m_settings->bufferSize * m_settings->outputChannels];
	}

	void Effect::ProcessEffect(float* buffer, unsigned int bufferSize, unsigned int numChannels) {
		if (!m_buffer) {
			PC_WARN("Effect buffer uninitialized! Cannot process.");
			return;
		}

		if (m_bypassed) {
			return;
		}

		memcpy(m_buffer, buffer, bufferSize * numChannels * sizeof(float));

		ProcessBuffer(buffer, bufferSize, numChannels);
		float mix = m_mix * 2.0f;

		for (unsigned int j = 0; j < bufferSize * numChannels; j++) {
			buffer[j] = ((buffer[j] * mix) + (m_buffer[j] * (2.0f - mix))) / 2;
		}
	}

	void Effect::Bypassed(bool state) {
		m_bypassed = state;
	}
}