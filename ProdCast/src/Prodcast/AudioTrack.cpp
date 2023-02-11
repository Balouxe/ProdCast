#include "AudioTrack.h"
#include <math.h>

namespace ProdCast {
	AudioTrack::AudioTrack() {
		m_parent = nullptr;
		m_settings = nullptr;
		m_engine = nullptr;
		m_volume = 1.0f;
		m_pan = 0.0f;
		setPan(0.0f);
		m_isPlaying = false;
	}

	AudioTrack::~AudioTrack() {
		delete[] m_buffer;
	}

	void AudioTrack::setParent(AudioTrack* parent) {
		m_parent = parent;
		// will definitely have to do something to notify to the parent "hey! i'm not yours anymore!"
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

	void AudioTrack::Play() {
		m_isPlaying = true;
	}

	void AudioTrack::Stop() {
		m_isPlaying = false;
	}
}