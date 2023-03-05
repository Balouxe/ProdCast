#include "AudioSources/AudioFile.h"
#include "Engine.h"
#include "Logger.h"

#include "AudioSources/Wav.h"
#include "AudioSources/Mp3.h"
#include "AudioSources/Flac.h"

namespace ProdCast {
	AudioFile::AudioFile(ProdCastEngine* engine, uint32_t streamHandle) {
		m_engine = engine;
		m_settings = engine->getAudioSettings(streamHandle);
		Init();
	}

	AudioFile::~AudioFile() {
		delete[] m_audioData;
	}

	void AudioFile::GetNextSamples(float* buffer, unsigned int nbSamples, unsigned int nbChannels) {
		if (!m_isLoaded) {
			return;
		}

		if (!m_isPlaying || m_isMuted) {
			return;
		}

		memset(m_buffer, 0, nbSamples * nbChannels * sizeof(float));

		unsigned int i;

		for (i = 0; i < nbSamples * nbChannels; i += m_numChannels) {
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

			if ((m_position == m_endLoopPoint) && m_isLooping) {
				m_position = m_startLoopPoint;
			}
			else if (m_position >= m_length * m_numChannels) {
				m_position = 0;
				m_isPlaying = false;
				break;
			}
		}

		if (m_processingChain)
			m_processingChain->ProcessEffects(m_buffer, nbSamples, nbChannels);

		for (i = 0; i < nbSamples * nbChannels; i++) {
			buffer[i] += m_buffer[i];
		}
	}

	void AudioFile::LoadFile(std::filesystem::path path) {
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

	void AudioFile::LoadFileCustom(std::filesystem::path path, bool (*loader)(std::filesystem::path&, float**, unsigned int*, unsigned int*, uint64_t*)) {
		if (loader(path, &m_audioData, &m_numChannels, &m_sampleRate, &m_length)) {
			m_isLoaded = true;
		}
	}

	void AudioFile::Play(){
		if(m_isLoaded)
			m_isPlaying = true;
	}

	void AudioFile::PlayFromStart() {
		if (m_isLoaded) {
			m_position = 0;
			m_isPlaying = true;
		}
	}

	void AudioFile::Stop() {
		m_isPlaying = false;
	}

#undef GetCurrentTime

	float AudioFile::GetCurrentTime() {
		return float(m_position / (m_sampleRate * m_numChannels));
	}

	uint64_t AudioFile::GetCurrentPosition() {
		return m_position / m_numChannels;
	}

	void AudioFile::SetCurrentTime(float time) {
		uint64_t pos = uint64_t(time * m_sampleRate);
		SetCurrentPosition(pos);
	}

	void AudioFile::SetCurrentPosition(uint64_t pos) {
		if (pos * m_numChannels >= m_length) {
			PC_WARN("Tried to set audio source time after its end ! Stopping source.");
			m_position = 0;
			m_isPlaying = false;
			return;
		}
		m_position = pos * m_numChannels;
	}

	float AudioFile::GetTotalTime() {
		return float(m_length / (m_sampleRate * m_numChannels));
	}

	uint64_t AudioFile::GetTotalLength() {
		return m_length / m_numChannels;
	}

	void AudioFile::SetStartLoopPointFrames(uint64_t frames) {
		if (frames * m_numChannels > m_length) {
			PC_WARN("Cannot set loop point after end of audio source! Disabling loop point.");
			m_isLooping = false;
			m_startLoopPoint = 0;
			return;
		}
		m_startLoopPoint = frames * m_numChannels;
	}

	void AudioFile::SetStartLoopPointSeconds(float seconds) {
		uint64_t frames = uint64_t(seconds * m_sampleRate);
		SetStartLoopPointFrames(frames);
	}

	float AudioFile::GetStartLoopPointSeconds() {
		return float((m_startLoopPoint / m_numChannels) * m_sampleRate);
	}

	uint64_t AudioFile::GetStartLoopPointFrames() {
		return m_startLoopPoint / m_numChannels;
	}

	void AudioFile::SetEndLoopPointFrames(uint64_t frames) {
		if (frames * m_numChannels > m_length) {
			PC_WARN("Cannot set loop point after end of audio source! Disabling loop point.");
			m_isLooping = false;
			m_endLoopPoint = 0;
			return;
		}
		m_endLoopPoint = frames * m_numChannels;
	}

	void AudioFile::SetEndLoopPointSeconds(float seconds) {
		uint64_t frames = uint64_t(seconds * m_sampleRate);
		SetEndLoopPointFrames(frames);
	}

	float AudioFile::GetEndLoopPointSeconds() {
		return float((m_endLoopPoint / m_numChannels) * m_sampleRate);
	}

	uint64_t AudioFile::GetEndLoopPointFrames() {
		return m_endLoopPoint / m_numChannels;
	}

	uint32_t AudioFile::GetSampleRate() {
		return m_sampleRate;
	}

	unsigned int AudioFile::GetNumChannels() {
		return m_numChannels;
	}

	void AudioFile::SetLoopPointActive(bool active) {
		if(!(m_startLoopPoint == 0 && m_endLoopPoint == 0))
			m_isLooping = active;
	}
}