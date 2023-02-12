#include "PortAudioBackend.h"

#include <iostream>

namespace ProdCast {
	PCError PortAudioBackend::Init(AudioSettings* settings, ProdCastEngine* engine) {
		Pa_Initialize();

		m_outputParameters.device = Pa_GetDefaultOutputDevice();
		m_outputParameters.channelCount = Pa_GetDeviceInfo(m_outputParameters.device)->maxOutputChannels;
		settings->outputChannels = m_outputParameters.channelCount;
		m_outputParameters.hostApiSpecificStreamInfo = NULL;
		m_outputParameters.sampleFormat = paFloat32;
		m_outputParameters.suggestedLatency = Pa_GetDeviceInfo(m_outputParameters.device)->defaultLowOutputLatency;
		settings->headRoom = (settings->sampleRate / settings->bufferSize) * m_outputParameters.suggestedLatency;
		
		m_inputParameters.device = Pa_GetDefaultInputDevice();
		m_inputParameters.channelCount = Pa_GetDeviceInfo(m_inputParameters.device)->maxInputChannels;
		settings->inputChannels = m_inputParameters.channelCount;
		m_inputParameters.hostApiSpecificStreamInfo = NULL;
		m_inputParameters.sampleFormat = paFloat32;
		m_inputParameters.suggestedLatency = Pa_GetDeviceInfo(m_inputParameters.device)->defaultLowInputLatency;

		if (Pa_OpenStream(&m_stream, &m_inputParameters, &m_outputParameters, settings->sampleRate, settings->bufferSize, 0, RenderPortAudio, (void*)engine) != 0)
		{
			Pa_Terminate();
			return E_UNKNOWNERROR;
		}

		Pa_StartStream(m_stream);

		return E_SUCCESS;
	}

	PCError PortAudioBackend::DeInit() {
		Pa_StopStream(m_stream);
		Pa_CloseStream(m_stream);
		Pa_Terminate();
		return E_SUCCESS;
	}

	int PortAudioBackend::RenderPortAudio(const void* input,
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
}