#include "ProcessingChain.h"
#include "Engine.h"

namespace ProdCast {

	ProcessingChain::ProcessingChain(ProdCastEngine* engine) {
		m_engine = engine;
		m_settings = engine->GetAudioSettings();

		for (int i = 0; i < PC_MAX_EFFECTS; i++) {
			m_effects[i] = nullptr;
		}

		m_buffer = new float[std::max(m_settings->bufferSize * m_settings->outputChannels, m_settings->bufferSize * m_settings->inputChannels)];
		for (unsigned int i = 0; i < std::max(m_settings->bufferSize * m_settings->outputChannels, m_settings->bufferSize * m_settings->inputChannels); i++) {
			m_buffer[i] = 0.0f;
		}
	}

	ProcessingChain::~ProcessingChain() {
		delete[] m_buffer;
	}

	void ProcessingChain::AddEffect(Effect* effect, uint8_t pos) {
		m_effects[pos] = effect;
	}

	void ProcessingChain::RemoveEffect(Effect* effect, uint8_t pos) {
		m_effects[pos] = nullptr; // 13.02.2024: Haven't looked at this code for a bit but there is no way in hell that doesn't cause a memory leak??
		// TODO: investigate
	}

	void ProcessingChain::SwapEffects(uint8_t from, uint8_t to) {
		Effect* temp = m_effects[to];
		m_effects[to] = m_effects[from];
		m_effects[from] = temp;
	}

	Effect* ProcessingChain::operator[](int pos) {
		return m_effects[pos];
	}

	void ProcessingChain::ProcessEffects(float* buffer, unsigned int bufferSize, unsigned int numChannels) {
		unsigned int i;
		float mix;

		for (i = 0; i < PC_MAX_EFFECTS; i++) {
			if (!m_effects[i]) {
				return;
			}
			memcpy(m_buffer, buffer, bufferSize * numChannels * sizeof(float));

			m_effects[i]->ProcessEffect(buffer, bufferSize, numChannels);
			mix = m_effects[i]->getMix();

			
			for (unsigned int j = 0; j < bufferSize * numChannels; j++) {
				buffer[i] = (m_buffer[i] * (1.0f - mix)) + (buffer[i] * (mix));
			}
			
		}
	}
	
}