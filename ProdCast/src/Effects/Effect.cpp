#include "Effect.h"

namespace ProdCast {

	Effect::Effect() {
		m_engine = nullptr;
		m_settings = nullptr;
		m_buffer = nullptr;
		m_mix = 1.0f;
	}

	Effect::~Effect() {
		delete[] m_buffer;
	}
}