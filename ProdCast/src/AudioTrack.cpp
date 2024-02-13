#include "AudioTrack.h"

#include "AudioBus.h"

#include <math.h>

namespace ProdCast {
	AudioTrack::AudioTrack() {
		setPan(0.0f);
	}

	AudioTrack::~AudioTrack() {
		delete[] m_buffer;
		delete[] m_RMS;
	}

	void AudioTrack::Init() {
		m_buffer = new float[m_settings->bufferSize * m_settings->outputChannels];
		memset(m_buffer, 0, m_settings->bufferSize * m_settings->outputChannels * sizeof(float));

		m_RMS = new float[m_settings->outputChannels];
		m_RMSValues.resize(m_RMSRate * m_settings->outputChannels);

		m_parent = m_engine->GetMasterBus();
		m_busHandle = m_parent->AddTrack(this);
	}

	void AudioTrack::InitWithoutParent() {
		m_buffer = new float[m_settings->bufferSize * m_settings->outputChannels];
		memset(m_buffer, 0, m_settings->bufferSize * m_settings->outputChannels * sizeof(float));

		m_RMS = new float[m_settings->outputChannels];
		m_RMSValues.resize(m_RMSRate * m_settings->outputChannels);
	}

	void AudioTrack::Process() {
		std::unique_lock lock{m_mutex};

		if (m_isMuted) {
			return;
		}

		float* buffer = m_parent ? m_parent->GetBuffer() : nullptr;

		GetNextSamples(buffer, m_settings->bufferSize, m_settings->outputChannels);

		uint32_t sizeToCopy = std::min(m_settings->bufferSize * m_settings->outputChannels, (m_RMSRate * m_settings->outputChannels) - m_RMSState);
		float* fromRMS = m_RMSValues.data() + m_settings->outputChannels;
		float* fromBuffer = m_buffer + (m_settings->bufferSize * m_settings->outputChannels - sizeToCopy);
		memcpy(fromRMS, fromBuffer, sizeof(float) * sizeToCopy);
		m_RMSState += sizeToCopy;
		if (m_RMSState >= m_RMSRate * m_settings->outputChannels) {
			CalculateRMS();
		}
	}

	void AudioTrack::ApplyProcessingChain(ProcessingChain* processingChain) {
		std::unique_lock lock{ m_mutex };
		m_processingChain = processingChain;
	}

	ProcessingChain* AudioTrack::getProcessingChain() {
		return m_processingChain;
	}

	void AudioTrack::AddParent(AudioBus* parent, float mix) {
		std::unique_lock lock{ m_mutex };

		if (m_parent) {
			m_parent->RemoveTrack(m_busHandle);
		}
		m_parent = parent;
		m_busHandle = m_parent->AddTrack(this);
	}

	void AudioTrack::RemoveParent(AudioBus* parent) {

	}

	void AudioTrack::SetParentWeight(AudioBus* parent, float mix) {
		std::unique_lock lock{ m_mutex };

		if (m_parent) {
			m_parent->RemoveTrack(m_busHandle);
		}
		m_parent = parent;
		m_busHandle = m_parent->AddTrack(this);
	}

	void AudioTrack::setVolume(float volume) {
		std::unique_lock lock{ m_mutex };

		m_volume = volume;
	}

	float AudioTrack::getVolume() {
		return m_volume;
	}

	void AudioTrack::setPan(float pan) {
		std::unique_lock lock{ m_mutex };
		m_pan = pan;
		m_gainLeft = cosf((3.14f * (pan + 1.0f)) / 4.0f);
		m_gainRight = sinf((3.14f * (pan + 1.0f)) / 4.0f);
	}

	float AudioTrack::getPan() {
		return m_pan;
	}

	void AudioTrack::Mute() {
		std::unique_lock lock{ m_mutex };
		m_isMuted = true;
	}

	void AudioTrack::Unmute() {
		std::unique_lock lock{ m_mutex };
		m_isMuted = false;
	}

	void AudioTrack::SetRMSRefreshRate(uint32_t refreshRate) {
		std::unique_lock lock{ m_mutex };
		if (m_RMSState >= refreshRate) {
			m_RMSState = 0;
		}
		m_RMSRate = refreshRate;
		m_RMSValues.resize(refreshRate * m_settings->outputChannels);
	}

	void AudioTrack::CalculateRMS() {
		double rms;
		for (int i = 0; i < m_settings->outputChannels; i++) {
			rms = 0.0;
			for (unsigned int j = 0; j < m_RMSRate; j++) {
				rms += m_RMSValues[j * m_settings->outputChannels + i] * m_RMSValues[j * m_settings->outputChannels + i];
			}
			m_RMS[i] = sqrtf((float)rms / m_RMSRate);
		}
		m_RMSState = 0;
		m_RMSValues.assign(m_RMSValues.size(), 0);
	}

	float* AudioTrack::GetRMS() {
		return m_RMS;
	}

	void AudioTrack::Enable3D(bool enabled) {
		if (m_is3D != enabled) {
			m_is3D = enabled;
			
			Calculate3D();
		}

		if (m_is3D = false) {
			m_3DVolumeMultiplier = 1.0f;
		}
	}

	void AudioTrack::Calculate3D() {
		Vec3 listenerLookingAt = m_engine->Get3DListenerLookingAt();
		Vec3 listenerPos = m_engine->Get3DListenerPosition();
		Vec3 upDirection = m_engine->Get3DUpDirection();

		Vec3 side = Cross(upDirection, listenerLookingAt);
		Normalize(side);
		float x = Dot(m_speakerPosition - listenerPos, side);
		float z = Dot(m_speakerPosition - listenerPos, listenerLookingAt);
		float angle = atan2f(x, z);
		float pan = sinf(angle);

		Vec3 relativePos = m_speakerPosition - listenerPos;
		float distance = sqrtf((relativePos.X * relativePos.X) + (relativePos.Y * relativePos.Y) + (relativePos.Z * relativePos.Z));
		float thing = 300.0f - distance;
		float volume = 0.0f;
		if (thing > 0) {
			volume = thing / 300.0f;
		}

		setPan(pan);
		m_3DVolumeMultiplier = volume;
	}

	void AudioTrack::Set3DPosition(Vec3 pos) {
		m_speakerPosition = pos;
		Calculate3D();
	}
}