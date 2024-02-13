#pragma once

#include "AudioTrack.h"
#include <filesystem>

namespace ProdCast {

	class PC_API AudioFile : public AudioTrack {
	public:
		AudioFile(ProdCastEngine* engine, uint32_t streamHandle = 0);
		~AudioFile();

		// Start playing sample from its current position
		void Play();
		// Start playing sample from start
		void PlayFromStart();
		// Stop playing sample
		void Stop();

		// Load audio file of supported formats (mp3, wav, flac)
		void LoadFile(std::filesystem::path path);
		// Load audio file with your custom 
		void LoadFileCustom(std::filesystem::path path, bool (*loader)(std::filesystem::path&, float**, unsigned int*, unsigned int*, uint64_t*));
#undef GetCurrentTime
		// Get current time in seconds
		float GetCurrentTime();
		// Get current position in frames
		uint64_t GetCurrentPosition();
		// Set current time in seconds
		void SetCurrentTime(float time);
		// Set current position in frames
		void SetCurrentPosition(uint64_t position);
		// Get total time in seconds
		float GetTotalTime();
		// Get total length in frames
		uint64_t GetTotalLength();

		// Get start loop point in seconds
		float GetStartLoopPointSeconds();
		// Get start loop point in frames
		uint64_t GetStartLoopPointFrames();
		// Set start loop point in seconds
		void SetStartLoopPointSeconds(float loopInSeconds);
		// Set start loop point in frames
		void SetStartLoopPointFrames(uint64_t loopInFrames);
		// Get end loop point in seconds
		float GetEndLoopPointSeconds();
		// Get end loop point in frames
		uint64_t GetEndLoopPointFrames();
		// Set end loop point in seconds
		void SetEndLoopPointSeconds(float loopInSeconds);
		// Set end loop point in frames
		void SetEndLoopPointFrames(uint64_t loopInFrames);
		// Set state of loop point (active)
		void SetLoopPointActive(bool active = true);

		// Get the sample rate of the audio file
		uint32_t GetSampleRate();
		// Get the number of channels of the audio file
		unsigned int GetNumChannels();

		void GetNextSamples(float* buffer, unsigned int samplesToGo, unsigned int nbChannels);

	private:
		float* m_audioData = nullptr;
		uint64_t m_length = 0;
		uint64_t m_position = 0;

		uint64_t m_startLoopPoint = 0;
		uint64_t m_endLoopPoint = 0;
		bool m_isLooping = false;

		unsigned int m_numChannels = 0;
		uint32_t m_sampleRate = 0;
		float* m_unResampledBuffer = nullptr;

		bool m_isLoaded = false;
		bool m_isPlaying = false;
	};

}