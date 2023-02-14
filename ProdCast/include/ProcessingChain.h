#pragma once
#include "Core.h"
#include "Effect.h"

#include <vector>

namespace ProdCast {

	class PC_API ProcessingChain {
	public:
		ProcessingChain();
		~ProcessingChain();

		void AddEffect(Effect* effect);
		void RemoveEffect(Effect* effect);

		void ProcessBuffer(float* buffer, unsigned int bufferSize, unsigned int numChannels);
	private:
		std::vector<Effect*> m_effects;
	};

}