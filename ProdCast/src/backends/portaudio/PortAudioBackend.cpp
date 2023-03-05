#include "Backends/PortAudioBackend.h"

#include "Logger.h"

#include <iostream>
#include <memory>

namespace ProdCast {
	PCError PortAudioBackend::Init(AudioSettings* settings, ProdCastEngine* engine) {
		m_engine = engine;

		Pa_Initialize();

		m_mainOutputParameters.device = Pa_GetDefaultOutputDevice();
		m_mainOutputParameters.channelCount = 2; // to fix
		settings->outputChannels = m_mainOutputParameters.channelCount;
		m_mainOutputParameters.hostApiSpecificStreamInfo = NULL;
		m_mainOutputParameters.sampleFormat = paFloat32;
		m_mainOutputParameters.suggestedLatency = Pa_GetDeviceInfo(m_mainOutputParameters.device)->defaultLowOutputLatency;
		settings->headRoom = (settings->sampleRate / settings->bufferSize) * m_mainOutputParameters.suggestedLatency;
		
		m_mainInputParameters.device = Pa_GetDefaultInputDevice();
		m_mainInputParameters.channelCount = Pa_GetDeviceInfo(m_mainInputParameters.device)->maxInputChannels;
		settings->inputChannels = m_mainInputParameters.channelCount;
		m_mainInputParameters.hostApiSpecificStreamInfo = NULL;
		m_mainInputParameters.sampleFormat = paFloat32;
		m_mainInputParameters.suggestedLatency = Pa_GetDeviceInfo(m_mainInputParameters.device)->defaultLowInputLatency;

		if (Pa_OpenStream(&m_mainStream, &m_mainInputParameters, &m_mainOutputParameters, settings->sampleRate, settings->bufferSize, paClipOff, RenderPortAudioMain, (void*)engine) != 0)
		{
			Pa_Terminate();
			return E_UNKNOWNERROR;
		}

		Pa_StartStream(m_mainStream);

		return E_SUCCESS;
	}

	PCError PortAudioBackend::DeInit() {
		Pa_StopStream(m_mainStream);
		Pa_CloseStream(m_mainStream);
		for (auto& stream : m_auxStreams) {
			Pa_StopStream(stream.second);
			Pa_CloseStream(stream.second);
		}

		Pa_Terminate();
		return E_SUCCESS;
	}

	PCError PortAudioBackend::OpenAuxiliaryStream(uint32_t handle, AudioSettings& settings) {
		PaStreamParameters outputParameters, inputParameters;
		outputParameters.device = Pa_GetDefaultOutputDevice();
		outputParameters.channelCount = 2; // to fix
		outputParameters.hostApiSpecificStreamInfo = NULL;
		outputParameters.sampleFormat = paFloat32;
		outputParameters.suggestedLatency = Pa_GetDeviceInfo(m_mainOutputParameters.device)->defaultLowOutputLatency;
		
		settings.outputChannels = m_mainOutputParameters.channelCount;
		settings.headRoom = (settings.sampleRate / settings.bufferSize) * m_mainOutputParameters.suggestedLatency;

		inputParameters.device = Pa_GetDefaultInputDevice();
		inputParameters.channelCount = Pa_GetDeviceInfo(m_mainInputParameters.device)->maxInputChannels;
		inputParameters.hostApiSpecificStreamInfo = NULL;
		inputParameters.sampleFormat = paFloat32;
		inputParameters.suggestedLatency = Pa_GetDeviceInfo(m_mainInputParameters.device)->defaultLowInputLatency;

		settings.inputChannels = m_mainInputParameters.channelCount;

		std::shared_ptr<ProdCastEngine::AuxStreamUserData> userData = std::make_shared<ProdCastEngine::AuxStreamUserData>();
		userData->engine = m_engine;
		userData->handle = handle;

		PaStream* stream;
		if (Pa_OpenStream(&stream, &inputParameters, &outputParameters, settings.sampleRate, settings.bufferSize, 0, nullptr, (void*)userData.get()) != 0) {
			return E_UNKNOWNERROR;
		}

		m_auxStreams[handle] = stream;
		PortAudioStreamParameters par;
		par.input = inputParameters;
		par.output = outputParameters;
		m_auxParameters[handle] = par;

		Pa_StartStream(stream);

		return E_SUCCESS;
	}

	PCError PortAudioBackend::CloseAuxiliaryStream(uint32_t handle) {
		if (!(m_auxStreams.contains(handle) && m_auxParameters.contains(handle))) {
			return E_UNKNOWNERROR;
		}

		PaStream* stream = m_auxStreams[handle];
		Pa_StopStream(stream);
		Pa_CloseStream(stream);
		m_auxStreams.erase(handle);
		m_auxParameters.erase(handle);
		return E_SUCCESS;
	}

	std::unordered_map<int, std::string> PortAudioBackend::GetInputDevices() {
		std::unordered_map<int, std::string> map;
		int deviceCount = Pa_GetDeviceCount();
		const PaDeviceInfo* temp;
		PC_TRACE("Input devices:");
		for (int i = 0; i < deviceCount; i++) {
			temp = Pa_GetDeviceInfo(i);
			if (temp->maxInputChannels > 0) {
				map[i] = temp->name;
				PC_TRACE("- {0} ({1})", temp->name, i);
			}
		}
		return map;
	}

	std::unordered_map<int, std::string> PortAudioBackend::GetOutputDevices() {
		std::unordered_map<int, std::string> map;
		int deviceCount = Pa_GetDeviceCount();
		const PaDeviceInfo* temp;
		PC_TRACE("Output devices:");
		for (int i = 0; i < deviceCount; i++) {
			temp = Pa_GetDeviceInfo(i);
			if (temp->maxOutputChannels > 0) {
				map[i] = temp->name;
				PC_TRACE("- {0} ({1})", temp->name, i);
			}
		}
		return map;
	}

	int PortAudioBackend::RenderPortAudioMain(const void* input,
		void* output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* /*timeInfo*/,
		PaStreamCallbackFlags statusFlags,
		void* userData) {

		if (statusFlags)
			std::cout << "Input/Output Underflow!\n";
		
		ProdCastEngine* engine = (ProdCastEngine*)userData;

		engine->AudioCallback((float*)output,(float*)input, frameCount);

		return 0;
	}

	int PortAudioBackend::RenderPortAudioAux(const void* input,
		void* output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* /*timeInfo*/,
		PaStreamCallbackFlags statusFlags,
		void* userData) {

		if (statusFlags)
			std::cout << "Input/Output Underflow!\n";

		ProdCastEngine::AuxStreamUserData* data = (ProdCastEngine::AuxStreamUserData *)userData;

		data->engine->AudioCallback((float*)output, (float*)input, frameCount);

		return 0;
	}
}