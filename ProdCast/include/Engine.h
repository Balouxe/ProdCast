#pragma once
#include "Core.h"
#include "Backends/AudioBackend.h"
#include "AudioThread.h"
#include "AudioTrack.h"
#include "AudioBus.h"
#include "RingBuffer.h"

#include <string>
#include <mutex>
#include <vector>
#include <unordered_map>

namespace ProdCast {

	class PC_API ProdCastEngine {
	public:
		// PUBLIC INTERFACE

		ProdCastEngine(AudioSettings& settings);
		~ProdCastEngine();

		struct Stream {
			Stream() = default;
			Stream(ProdCastEngine* engine, AudioSettings& settings, uint32_t streamHandle = 0);
			~Stream();
			void InitStream(ProdCastEngine* engine, AudioSettings& settings, uint32_t streamHandle = 0);

			AudioSettings settings;
			AudioBus* bus = nullptr;
			RingBuffer* ringBuffer = nullptr;
			void(*inputCallback)(float*, unsigned int, unsigned int) = nullptr;
			float* outputBuffer = nullptr;
			float* inputBuffer = nullptr;
			uint32_t streamHandle;
		};

		struct AuxStreamUserData {
			ProdCastEngine* engine;
			uint32_t handle;
		};
		
		void setMasterGain(float gain);
		inline float getMasterGain() const { return m_masterGain; }
		
		void setTempo(uint16_t tempo);

		AudioSettings* getAudioSettings(uint32_t streamHandle = 0);
		inline AudioBus* getMasterBus() { return m_mainStream.bus; }

		// Returns the internal input buffer
		float* getInputBuffer() const { return m_mainStream.inputBuffer; }		

		// Sets a function that will get called every time an input buffer is given.
		// This function is called directly in the main audio callback so it must be quickly executed.
		// Arguments are the buffer, the size of the buffer (divided by the number of channels) and the channel count
		void SetInputCallback(void(*inputCallback)(float*, unsigned int, unsigned int), uint32_t streamHandle = 0);

		uint32_t OpenAuxiliaryStream(AudioSettings& settings);
		void CloseAuxiliaryStream(uint32_t handle);

		// PRIVATE INTERFACE

		inline Stream getStreamData() { return m_mainStream; }
		inline ThreadPool* getPool() const { return m_threadPool; }
		Stream& GetStream(uint32_t streamHandle = 0);
		
		int AudioCallback(float* outputBuffer, float* inputBuffer, unsigned long frameCount, uint32_t auxHandle = 0);

		void RenderBuffer(uint32_t streamHandle = 0);

	private:
		std::mutex m_audioMutex;
		ThreadPool* m_threadPool = nullptr;
		AudioBackend* m_backend = nullptr;
		Stream m_mainStream;
		
		bool isInit = false;

		float m_masterGain = 1.0f;

		std::string m_inputDeviceName;
		std::string m_outputDeviceName;

		uint32_t m_auxHandleCount = 1;
		std::unordered_map<uint32_t, Stream> m_auxStreams;
	};
}

