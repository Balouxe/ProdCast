#include "RingBuffer.h"
#include "Utils/Logger.h"

namespace ProdCast {

	RingBuffer::RingBuffer(unsigned int sampleSize, unsigned int sampleRate) {
		m_readPos = 0;
		m_writePos = 2;
		m_sampleSize = sampleSize;
		m_bufferCapacity = int(sampleRate / sampleSize);
		
		m_buffer = new float[m_bufferCapacity * m_sampleSize];
		for (int i = 0; i < m_bufferCapacity * m_sampleSize; i++) {
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
			m_buffer[m_readPos * m_sampleSize + i] = 0.0f;
		}
		m_readPos = m_readPos == (m_bufferCapacity - 1) ? 0 : m_readPos + 1;
	}

	void RingBuffer::Write(float* writeBuffer) {
		std::scoped_lock lock{ m_mutex };
		unsigned int i;
		for (i = 0; i < m_sampleSize; i++) {
			m_buffer[m_writePos * m_sampleSize + i] = writeBuffer[i];
		}
		m_writePos = m_writePos == (m_bufferCapacity - 1) ? 0 : m_writePos + 1;
	}
}