#include "AudioSource.h"
#include "Prodcast/Engine.h"

#include "wav/Wav.h"
#include "mp3/Mp3.h"
#include "flac/Flac.h"

namespace ProdCast {
	AudioSource::AudioSource(ProdCastEngine* engine) {
		m_engine = engine;
		m_settings = engine->getAudioSettings();
		m_parent = engine->getMasterBus();
		m_audioData = nullptr;
		m_length = 0;
		m_position = 0;
		m_numChannels = 0;
		m_sampleRate = 0;
		m_isLoaded = false;
		m_isPlaying = false;

		m_buffer = new float[m_settings->bufferSize * m_settings->outputChannels];

		for (int i = 0; i < m_settings->bufferSize * m_settings->outputChannels; i++) {
			m_buffer[i] = 0.0f;
		}
		m_busHandle = m_parent->AddTrack(this);
	}

	void AudioSource::Play(){
		if(m_isLoaded)
			m_isPlaying = true;
	}

	void AudioSource::Process() {
		if (!m_isPlaying)
			return;

		float* buffer = m_parent ? m_parent->GetBuffer() : nullptr;
		GetNextSamples(buffer, m_settings->bufferSize, m_settings->outputChannels);
	}

	void AudioSource::GetNextSamples(float* buffer, unsigned int nbSamples, unsigned int nbChannels) {
		float* temp = m_buffer;
		unsigned int i;

		for (i = 0; i < nbSamples * nbChannels; i += m_numChannels) {
			if (!m_isPlaying) {
				*temp++ = 0.0f;
				continue;
			}

			if (m_numChannels == 1 && nbChannels == 1) {
				m_buffer[i] = m_audioData[m_position++] * m_volume;
			}
			else if (m_numChannels == 1 && nbChannels == 2) {
				m_buffer[i] = m_audioData[m_position] * m_volume * m_gainLeft;
				m_buffer[i + 1] = m_audioData[m_position++] * m_volume * m_gainRight;
			}
			else if (m_numChannels == 2 && nbChannels == 2) {
				m_buffer[i] = m_audioData[m_position++] * m_volume * m_gainLeft;
				m_buffer[i + 1] = m_audioData[m_position++] * m_volume * m_gainRight;
			}
			else if (m_numChannels == 2 && nbChannels == 1) {
				m_buffer[i] = (m_audioData[m_position++] * m_gainLeft + m_audioData[m_position++] * m_gainRight) / 2 * m_volume;
			}

			// m_processingChain->ProcessBuffer(buffer, nbSamples, nbChannels); or something
			// m_position += m_numChannels;

			if (m_position > m_length) {
				m_position = 0;
				m_isPlaying = false;
			}
		}

		for (i = 0; i < nbSamples * nbChannels; i++) {
			buffer[i] += m_buffer[i];
		}
	}

	void AudioSource::LoadFile(std::filesystem::path path) {
		std::filesystem::path extension = path.extension();
		if (extension == ".wav") {
			if (Wav::LoadWavFile(path, &m_audioData, &m_numChannels, &m_sampleRate, &m_length)) {
				m_isLoaded = true;
			}
		}
		else if (extension == ".mp3") {
			if (Mp3::LoadMp3File(path, &m_audioData, &m_numChannels, &m_sampleRate, &m_length)) {
				m_isLoaded = true;
			}
		}
		else if (extension == ".flac") {
			if (Flac::LoadFlacFile(path, &m_audioData, &m_numChannels, &m_sampleRate, &m_length)) {
				m_isLoaded = true;
			}
		}
	}

	AudioSource::~AudioSource() {
		delete[] m_audioData;
	}
}