#include "RingBuffer.h"
#include "Utils/Logger.h"

namespace ProdCast {

	RingBuffer::RingBuffer(AudioSettings* settings) {
		m_readPos = 0;
		m_writePos = settings->headRoom;
		m_sampleSize = settings->bufferSize * settings->outputChannels;
		m_bufferCapacity = int(settings->sampleRate / m_sampleSize);
		
		m_buffer = new float[m_bufferCapacity * m_sampleSize];
		for (unsigned int i = 0; i < m_bufferCapacity * m_sampleSize; i++) {
			m_buffer[i] = 0.0f;
		}
		PC_TRACE("Ring buffer created");
	}

	RingBuffer::~RingBuffer(){
		delete[] m_buffer;
		PC_TRACE("Ring buffer deleted");
	}

	void RingBuffer::Read(float* readBuffer) {
		std::scoped_lock lock{ m_mutex };
		unsigned int i;
		for (i = 0; i < m_sampleSize; i++) {
			readBuffer[i] = m_buffer[m_readPos * m_sampleSize + i];
			// m_buffer[m_readPos * m_sampleSize + i] = 0.0f;
		}
		m_readPos = m_readPos == (m_bufferCapacity - 1) ? 0 : m_readPos + 1;
	}

	void RingBuffer::Write(float* writeBuffer) {
		std::scoped_lock lock{ m_mutex };

		if (m_simulateCrash) return;

		unsigned int i;
		for (i = 0; i < m_sampleSize; i++) {
			m_buffer[m_writePos * m_sampleSize + i] = writeBuffer[i];
		}
		m_writePos = m_writePos == (m_bufferCapacity - 1) ? 0 : m_writePos + 1;
	}

	void RingBuffer::SimulateCrash() {
		std::scoped_lock lock{ m_mutex };
		m_simulateCrash = true;
	}
}