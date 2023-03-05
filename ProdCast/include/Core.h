#pragma once
#include <chrono>
#include <iostream>

#define PC_MAX_EFFECTS 10

#ifdef PC_PLATFORM_WINDOWS
	#if defined(PC_BUILD_DLL)
		#define PC_API __declspec(dllexport)
	#elif defined(PC_USE_DLL)
		#define PC_API __declspec(dllimport)
	#else
		#define PC_API
	#endif
#else
	#error ProdCast only supported on windows (currently)
#endif

namespace ProdCast {
	enum PC_API PCError {
		E_SUCCESS,
		E_UNKNOWNERROR,
		E_AUDIODEVICEFAILED
	};

	enum PC_API Backends {
		BE_SDL,
		BE_PORTAUDIO,
		BE_RTAUDIO,
		BE_NONE
	};

	struct AudioSettings {
		// Desired sample rate, user defined.
		unsigned int sampleRate = 44100;
		// Desired buffer size, user defined.
		unsigned int bufferSize = 512;
		// Desired audio backend, user defined. Will only have effect for the main stream.
		Backends audioBackend = BE_NONE;
		// Tempo of the audio stream, only useful for VST plugins, user defined.
		uint16_t tempo = 130;
		// Number of input channels of main stream, internally defined.
		int inputChannels = -1;
		// Number of output channels of main stream, internally defined.
		int outputChannels = -1;
		// Number of buffers pre-rendered, internally defined.
		unsigned int headRoom = 4;
		// ID of desired output device (Optional)
		int inputDevice = -1;
		// ID of desired input device (Optional)
		int outputDevice = -1;
	};

	// debug, temporary
#ifdef PC_DEBUG
	struct Timer {
		std::chrono::time_point<std::chrono::steady_clock> start, end;
		std::chrono::duration<float> duration;

		Timer() {
			start = std::chrono::high_resolution_clock::now();
		}

		~Timer() {
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;

			std::cout << "Timer took " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << ".\n";
		}
	};
#endif
}