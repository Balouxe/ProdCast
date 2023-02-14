#pragma once
#include "Core.h"

namespace ProdCast {

	class ProdCastEngine;

	class PC_API Effect {
	public:
		Effect();
		~Effect();

		virtual void ProcessBuffer(float* buffer, unsigned int samplesToGo, unsigned int nbChannels) = 0;

		inline float getMix() { return m_mix; };
		inline void setMix(float mix) { m_mix = mix; };
	protected:
		ProdCastEngine* m_engine;
		AudioSettings* m_settings;

		float* m_buffer;
		float m_mix;

		const char* m_effectName;
	};


}