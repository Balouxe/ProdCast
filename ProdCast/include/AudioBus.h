#pragma once
#include "Core.h"
#include "AudioTrack.h"
#include "Engine.h"

#include <mutex>
#include <unordered_map>

namespace ProdCast {

	class PC_API AudioBus : public AudioTrack {
	public:
		AudioBus(ProdCastEngine* engine, uint32_t streamHandle = 0, bool isMaster = false);
		~AudioBus();

		std::unordered_map<uint16_t, AudioTrack*>& GetTracks();

		void GetNextSamples(float* buffer, unsigned int samplesToGo, unsigned int nbChannels);

		friend class AudioTrack;
		friend class AudioFile;
	private:
		uint16_t AddTrack(AudioTrack* track);
		void RemoveTrack(uint16_t handle);

		std::unordered_map<uint16_t, AudioTrack*> m_tracks;
		uint16_t m_mapPosition = 1; // 0 will be when a track has no parent

		bool m_isMaster = false;
	};

}