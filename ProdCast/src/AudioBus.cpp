#include "AudioBus.h"
#include "Logger.h"

namespace ProdCast {
	AudioBus::AudioBus(ProdCastEngine* engine) {
		m_engine = engine;
		m_settings = engine->getAudioSettings();

		m_buffer = new float[m_settings->bufferSize * m_settings->outputChannels];
		for (int i = 0; i < m_settings->bufferSize * m_settings->outputChannels; i++) {
			m_buffer[i] = 0.0f;
		}
	}

	AudioBus::AudioBus(ProdCastEngine* engine, bool isMaster) {
		m_engine = engine;
		m_settings = engine->getAudioSettings();
		m_isMaster = isMaster;

		m_buffer = new float[m_settings->bufferSize * m_settings->outputChannels];
		for (int i = 0; i < m_settings->bufferSize * m_settings->outputChannels; i++) {
			m_buffer[i] = 0.0f;
		}
	}

	AudioBus::~AudioBus() {

	}

	void AudioBus::Process(){
		std::lock_guard lock(m_mutex);
		float* buffer;
		if (!m_isMaster)
			buffer = m_parent->GetBuffer();
		else
			buffer = nullptr;
		GetNextSamples(buffer, m_settings->bufferSize, m_settings->outputChannels);

	}

	void AudioBus::GetNextSamples(float* buffer, unsigned int samplesToGo, unsigned int nbChannels) {
		if (m_isMuted)
			return;
		for (auto& track : m_tracks) {
			track.second->Process();
		}

		if (buffer) {
			if (m_processingChain) {
				m_processingChain->ProcessBuffer(buffer, samplesToGo, nbChannels);
			}

			for (unsigned int i = 0; i < samplesToGo * nbChannels; i += nbChannels) {
				if (nbChannels == 1) {
					buffer[i] = m_buffer[i] * m_volume;
				}
				else if (nbChannels == 2) {
					buffer[i] = m_buffer[i] * m_volume * m_gainLeft;
					buffer[i + 1] = m_buffer[i + 1] * m_volume * m_gainRight;
				}

				for (int i = 0; i < m_settings->bufferSize * m_settings->outputChannels; i++) {
					m_buffer[i] = 0.0f;
				}
			}
		}
		else {
			m_engine->RenderBuffer();
			for (int i = 0; i < m_settings->bufferSize * m_settings->outputChannels; i++) {
				m_buffer[i] = 0.0f;
			}
		}
	}

	uint16_t AudioBus::AddTrack(AudioTrack* track) {
		m_mapPosition++;
		m_tracks[m_mapPosition] = track;
		return m_mapPosition;
	}

	void AudioBus::RemoveTrack(uint16_t handle) {
		m_tracks.erase(handle);
	}
}