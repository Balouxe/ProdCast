#pragma once
#include "Core.h"
#include "Effects/Effect.h"

#include <vector>

namespace ProdCast {
	class ProdCastEngine;

	class PC_API ProcessingChain {
	public:
		ProcessingChain(ProdCastEngine* engine);
		~ProcessingChain();

		void AddEffect(Effect* effect, uint8_t pos);
		void RemoveEffect(Effect* effect, uint8_t pos);

		void SwapEffects(uint8_t from, uint8_t to);

		void ProcessEffects(float* buffer, unsigned int bufferSize, unsigned int numChannels);

		Effect* operator[](int pos);
	private:
		ProdCastEngine* m_engine;
		AudioSettings* m_settings;

		Effect* m_effects[PC_MAX_EFFECTS];
		float* m_buffer;
	};

}