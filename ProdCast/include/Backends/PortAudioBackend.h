#pragma once
#include "Core.h"
#include "AudioBackend.h"

#include "portaudio.h"

#include <unordered_map>

namespace ProdCast {

	class ProdCastEngine;

	class PortAudioBackend : public AudioBackend {
	public:
		struct PortAudioStreamParameters {
			PaStreamParameters input;
			PaStreamParameters output;
		};

		PCError Init(AudioSettings* settings, ProdCastEngine* engine);
		PCError DeInit();

		static int RenderPortAudioMain(const void* /*input*/,
			void* output,
			unsigned long frameCount,
			const PaStreamCallbackTimeInfo* /*timeInfo*/,
			PaStreamCallbackFlags /*statusFlags*/,
			void* userData);

		static int RenderPortAudioAux(const void* /*input*/,
			void* output,
			unsigned long frameCount,
			const PaStreamCallbackTimeInfo* /*timeInfo*/,
			PaStreamCallbackFlags /*statusFlags*/,
			void* userData);

		PCError OpenAuxiliaryStream(uint32_t handle, AudioSettings& settings);
		PCError CloseAuxiliaryStream(uint32_t handle);

		std::unordered_map<uint32_t, DeviceInfo> GetInputDevices();
		std::unordered_map<uint32_t, DeviceInfo> GetOutputDevices();

		void ChangeOutputDevice(uint32_t streamHandle, uint32_t deviceId);
		void ChangeInputDevice(uint32_t streamHandle, uint32_t deviceId);

	private:
		PaStream* GetStream(uint32_t handle);
		PaStreamParameters& GetStreamInputParameters(uint32_t streamHandle = 0u);
		PaStreamParameters& GetStreamOutputParameters(uint32_t streamHandle = 0u);

		ProdCastEngine* m_engine;

		PaStream* m_mainStream;
		PortAudioStreamParameters m_mainParameters;

		std::unordered_map<uint32_t, PaStream*> m_auxStreams;
		std::unordered_map<uint32_t, PortAudioStreamParameters> m_auxParameters;
	};

}