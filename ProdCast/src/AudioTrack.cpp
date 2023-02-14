#include "AudioTrack.h"

#include "AudioBus.h"

#include <math.h>

namespace ProdCast {
	AudioTrack::AudioTrack() {
		m_parent = nullptr;
		m_busHandle = 0;
		m_settings = nullptr;
		m_engine = nullptr;
		m_volume = 1.0f;
		m_pan = 0.0f;
		setPan(0.0f);
		m_isMuted = false;
		m_processingChain = nullptr;
	}

	AudioTrack::~AudioTrack() {
		delete[] m_buffer;
	}

	void AudioTrack::ApplyProcessingChain(ProcessingChain* processingChain) {
		m_processingChain = processingChain;
	}

	ProcessingChain* AudioTrack::getProcessingChain() {
		return m_processingChain;
	}

	void AudioTrack::setParent(AudioBus* parent) {
		m_parent->RemoveTrack(m_busHandle);
		m_parent = parent;
		m_busHandle = m_parent->AddTrack(this);
	}

	void AudioTrack::setVolume(float volume) {
		m_volume = volume;
	}

	float AudioTrack::getVolume() {
		return m_volume;
	}

	void AudioTrack::setPan(float pan) {
		m_pan = pan;
		m_gainLeft = cos((3.14 * (pan + 1)) / 4);
		m_gainRight = sin((3.14 * (pan + 1)) / 4);
	}

	float AudioTrack::getPan() {
		return m_pan;
	}

	void AudioTrack::Mute() {
		m_isMuted = true;
	}

	void AudioTrack::Unmute() {
		m_isMuted = false;
	}
}