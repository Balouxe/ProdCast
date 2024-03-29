#pragma once
#include "Core.h"

#include <shared_mutex>

namespace ProdCast {

	// maybe do template one day ?
	class RingBuffer {
	public:
		RingBuffer(AudioSettings* settings);
		~RingBuffer();

		void Write(float* writeBuffer);
		void Read(float* readBuffer);

		void SimulateCrash();
	private:
		float* m_buffer;
		unsigned int m_readPos;
		unsigned int m_writePos;
		unsigned int m_bufferCapacity;
		unsigned int m_sampleSize;

		std::shared_mutex m_mutex;

		bool m_simulateCrash = false;
	};

}