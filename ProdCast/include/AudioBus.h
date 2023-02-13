#pragma once
#include "Core.h"
#include "AudioTrack.h"
#include "Engine.h"

#include <mutex>
#include <vector>
#include <unordered_map>

namespace ProdCast {

	class PC_API AudioBus : public AudioTrack {
	public:
		AudioBus(ProdCastEngine* engine);
		AudioBus(ProdCastEngine* engine, bool isMaster);
		~AudioBus();

		void GetNextSamples(float* buffer, unsigned int samplesToGo, unsigned int nbChannels);

		uint16_t AddTrack(AudioTrack* track);
		void RemoveTrack(uint16_t handle);

		void Process();
	private:
		std::unordered_map<uint16_t, AudioTrack*> m_tracks;
		uint16_t m_mapPosition = 0;

		bool m_isMaster = false;

		std::shared_mutex m_mutex;
	};

}