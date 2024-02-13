#include "Backends/PortAudioBackend.h"
#include "Engine.h"

#include "Utils/Logger.h"

#include <iostream>
#include <memory>

namespace ProdCast {
	PCError PortAudioBackend::Init(AudioSettings* settings, ProdCastEngine* engine) {
		m_engine = engine;

		Pa_Initialize();
			
		m_mainParameters.output.device = Pa_GetDefaultOutputDevice();
		m_mainParameters.output.channelCount = 2; // to fix
		settings->outputChannels = m_mainParameters.output.channelCount;
		m_mainParameters.output.hostApiSpecificStreamInfo = NULL;
		m_mainParameters.output.sampleFormat = paFloat32;
		m_mainParameters.output.suggestedLatency = Pa_GetDeviceInfo(m_mainParameters.output.device)->defaultLowOutputLatency;
		settings->headRoom = (settings->sampleRate / settings->bufferSize) * m_mainParameters.output.suggestedLatency;
		
		m_mainParameters.input.device = Pa_GetDefaultInputDevice();
		m_mainParameters.input.channelCount = Pa_GetDeviceInfo(m_mainParameters.input.device)->maxInputChannels;
		settings->inputChannels = m_mainParameters.input.channelCount;
		m_mainParameters.input.hostApiSpecificStreamInfo = NULL;
		m_mainParameters.input.sampleFormat = paFloat32;
		m_mainParameters.input.suggestedLatency = Pa_GetDeviceInfo(m_mainParameters.input.device)->defaultLowInputLatency;

		if (Pa_OpenStream(&m_mainStream, &m_mainParameters.input, &m_mainParameters.output, settings->sampleRate, settings->bufferSize, paClipOff, RenderPortAudioMain, (void*)engine) != 0)
		{
			PC_ERROR("Couldn't open stream !");
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
		outputParameters.suggestedLatency = Pa_GetDeviceInfo(m_mainParameters.output.device)->defaultLowOutputLatency;
		
		settings.outputChannels = m_mainParameters.output.channelCount;
		settings.headRoom = (settings.sampleRate / settings.bufferSize) * m_mainParameters.output.suggestedLatency;

		inputParameters.device = Pa_GetDefaultInputDevice();
		inputParameters.channelCount = Pa_GetDeviceInfo(m_mainParameters.input.device)->maxInputChannels;
		inputParameters.hostApiSpecificStreamInfo = NULL;
		inputParameters.sampleFormat = paFloat32;
		inputParameters.suggestedLatency = Pa_GetDeviceInfo(m_mainParameters.input.device)->defaultLowInputLatency;

		settings.inputChannels = m_mainParameters.input.channelCount;

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

	std::unordered_map<uint32_t, DeviceInfo> PortAudioBackend::GetInputDevices() {
		std::unordered_map<uint32_t, DeviceInfo> map;
		int deviceCount = Pa_GetDeviceCount();
		const PaDeviceInfo* temp;
		PC_TRACE("Input devices:");
		for (int i = 0; i < deviceCount; i++) {
			temp = Pa_GetDeviceInfo(i);
			if (temp->maxInputChannels > 0) {
				map[i] = DeviceInfo(i ,temp->name);
				PC_TRACE("- {0} ({1})", temp->name, i);
			}
		}
		return map;
	}

	std::unordered_map<uint32_t, DeviceInfo> PortAudioBackend::GetOutputDevices() {
		std::unordered_map<uint32_t, DeviceInfo> map;
		int deviceCount = Pa_GetDeviceCount();
		const PaDeviceInfo* temp;
		PC_TRACE("Output devices:");
		for (int i = 0; i < deviceCount; i++) {
			temp = Pa_GetDeviceInfo(i);
			if (temp->maxOutputChannels > 0) {
				map[i] = DeviceInfo(i, temp->name);
				PC_TRACE("- {0} ({1})", temp->name, i);
			}
		}
		return map;
	}

	PaStream* PortAudioBackend::GetStream(uint32_t handle) {
		if (handle == 0u) {
			return m_mainStream;
		}
		else {
			return m_auxStreams[handle];
		}
	}

	PaStreamParameters& PortAudioBackend::GetStreamInputParameters(uint32_t handle) {
		if (handle == 0u) {
			return m_mainParameters.input;
		}
		else {
			return m_auxParameters[handle].input;
		}
	}

	PaStreamParameters& PortAudioBackend::GetStreamOutputParameters(uint32_t handle) {
		if (handle == 0u) {
			return m_mainParameters.output;
		}
		else {
			return m_auxParameters[handle].output;
		}
	}

	void PortAudioBackend::ChangeOutputDevice(uint32_t streamHandle, uint32_t deviceId) {
		PaStream* stream = GetStream(streamHandle);

		Pa_StopStream(stream);
		Pa_CloseStream(stream);

		PaStreamParameters newStreamParameters;
		newStreamParameters.device = deviceId;
		newStreamParameters.channelCount = 2; // to fix
		newStreamParameters.hostApiSpecificStreamInfo = NULL;
		newStreamParameters.sampleFormat = paFloat32;
		newStreamParameters.suggestedLatency = Pa_GetDeviceInfo(m_mainParameters.output.device)->defaultLowOutputLatency;

		AudioSettings& settings = m_engine->GetStream(streamHandle).GetAudioSettings();

		settings.outputChannels = newStreamParameters.channelCount;
		settings.headRoom = (settings.sampleRate / settings.bufferSize) * newStreamParameters.suggestedLatency;

		if (streamHandle == 0u) {
			m_mainParameters.output = newStreamParameters;

			if (Pa_OpenStream(&stream, &GetStreamInputParameters(), &GetStreamOutputParameters(), settings.sampleRate, settings.bufferSize, paClipOff, RenderPortAudioMain, (void*)m_engine) != 0)
			{
				Pa_Terminate();
				PC_ERROR("Couldn't start PortAudio stream !");
				return;
			}
		}
		else {
			std::shared_ptr<ProdCastEngine::AuxStreamUserData> userData = std::make_shared<ProdCastEngine::AuxStreamUserData>();
			userData->engine = m_engine;
			userData->handle = streamHandle;

			m_auxParameters[streamHandle].output = newStreamParameters;

			if (Pa_OpenStream(&stream, &GetStreamInputParameters(streamHandle), &GetStreamOutputParameters(streamHandle), settings.sampleRate, settings.bufferSize, paClipOff, RenderPortAudioAux, (void*)userData.get()) != 0)
			{
				Pa_Terminate();
				PC_ERROR("Couldn't start PortAudio stream !");
				return;
			}
		}

		Pa_StartStream(m_mainStream);
	}

	void PortAudioBackend::ChangeInputDevice(uint32_t streamHandle, uint32_t deviceId) {
		PaStream* stream = GetStream(streamHandle);

		Pa_StopStream(stream);
		Pa_CloseStream(stream);

		PaStreamParameters newStreamParameters;
		newStreamParameters.device = deviceId;
		newStreamParameters.channelCount = 2; // to fix
		newStreamParameters.hostApiSpecificStreamInfo = NULL;
		newStreamParameters.sampleFormat = paFloat32;
		newStreamParameters.suggestedLatency = Pa_GetDeviceInfo(m_mainParameters.input.device)->defaultLowInputLatency;

		AudioSettings& settings = m_engine->GetStream(streamHandle).GetAudioSettings();

		settings.inputChannels = newStreamParameters.channelCount;
		settings.headRoom = (settings.sampleRate / settings.bufferSize) * newStreamParameters.suggestedLatency;

		if (streamHandle == 0u) {
			m_mainParameters.input = newStreamParameters;

			if (Pa_OpenStream(&stream, &GetStreamInputParameters(), &GetStreamOutputParameters(), settings.sampleRate, settings.bufferSize, paClipOff, RenderPortAudioMain, (void*)m_engine) != 0)
			{
				Pa_Terminate();
				PC_ERROR("Couldn't start PortAudio stream !");
				return;
			}
		}
		else {
			m_auxParameters[streamHandle].input = newStreamParameters;

			std::shared_ptr<ProdCastEngine::AuxStreamUserData> userData = std::make_shared<ProdCastEngine::AuxStreamUserData>();
			userData->engine = m_engine;
			userData->handle = streamHandle;

			if (Pa_OpenStream(&stream, &GetStreamInputParameters(streamHandle), &GetStreamOutputParameters(streamHandle), settings.sampleRate, settings.bufferSize, paClipOff, RenderPortAudioAux, (void*)userData.get()) != 0)
			{
				Pa_Terminate();
				PC_ERROR("Couldn't start PortAudio stream !");
				return;
			}
		}

		Pa_StartStream(m_mainStream);
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