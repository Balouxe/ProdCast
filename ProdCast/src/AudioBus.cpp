#include "AudioBus.h"
#include "Logger.h"

namespace ProdCast {

	AudioBus::AudioBus(ProdCastEngine* engine, uint32_t streamHandle ,bool isMaster) {
		m_engine = engine;
		m_settings = engine->getAudioSettings();
		m_isMaster = isMaster;
		m_tracks.clear();
		if (isMaster) {
			InitWithoutParent();
		}
		else {
			Init();
		}
	}

	AudioBus::~AudioBus() {
		// m_buffer deleted by AudioTrack (might change that some day)
	}

	void AudioBus::GetNextSamples(float* buffer, unsigned int samplesToGo, unsigned int nbChannels) {
		memset(m_buffer, 0, m_settings->bufferSize * m_settings->outputChannels * sizeof(float)); // clear last buffer

		for (auto& track : m_tracks) {
			track.second->Process();
		}

		if (m_isMuted)
			return;

		if (buffer) {
			if (m_processingChain) {
				m_processingChain->ProcessEffects(buffer, samplesToGo, nbChannels);
			}

			for (unsigned int i = 0; i < samplesToGo * nbChannels; i += nbChannels) {
				if (nbChannels == 1) {
					buffer[i] = m_buffer[i] * m_volume;
				}
				else if (nbChannels == 2) {
					buffer[i] = m_buffer[i] * m_volume * m_gainLeft;
					buffer[i + 1] = m_buffer[i + 1] * m_volume * m_gainRight;
				}
			}
		}
		else if (m_engine->getMasterBus() == this) {
			m_engine->RenderBuffer();
		}
		else {
			return;
		}
	}

	uint16_t AudioBus::AddTrack(AudioTrack* track) {
		m_mapPosition++;
		m_tracks[m_mapPosition] = track;
		return m_mapPosition;
	}

	void AudioBus::RemoveTrack(uint16_t handle) {
		if (m_tracks.empty()) {
			return;
		}
		if (m_tracks.contains(handle)) {
			m_tracks.erase(handle);
		}
	}
}