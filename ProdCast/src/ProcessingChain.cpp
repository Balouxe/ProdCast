#include "ProcessingChain.h"

namespace ProdCast {

	ProcessingChain::ProcessingChain() {

	}

	ProcessingChain::~ProcessingChain() {

	}

	void ProcessingChain::AddEffect(Effect* effect) {
		m_effects.push_back(effect);
	}

	void ProcessingChain::RemoveEffect(Effect* effect) {

	}

	void ProcessingChain::ProcessBuffer(float* buffer, unsigned int bufferSize, unsigned int numChannels) {
		if (m_effects.empty()) {
			return;
		}
		for (int i = 0; i < m_effects.size(); i++) {
			if (m_effects[i]) {
				m_effects[i]->ProcessBuffer(buffer, bufferSize, numChannels);
			}
		}
	}
	
}