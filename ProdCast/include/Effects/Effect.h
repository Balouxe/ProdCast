#pragma once
#include "Core.h"

namespace ProdCast {

	class ProdCastEngine;

	class PC_API Effect {
	public:
		Effect();
		~Effect();

		void ProcessEffect(float* buffer, unsigned int bufferSize, unsigned int numChannels);

		inline float getMix() { return m_mix; };
		inline void setMix(float mix) { m_mix = mix; };

		void Bypassed(bool state);
	protected:
		virtual void ProcessBuffer(float* buffer, unsigned int bufferSize, unsigned int numChannels) = 0;

		// Init function called in constructors of class implementations.
		// Has to be called after setting m_settings.
		void Init();

		ProdCastEngine* m_engine;
		AudioSettings* m_settings;

		float m_mix;
		bool m_bypassed = false;

		const char* m_effectName;
	private:
		float* m_buffer;
};

}