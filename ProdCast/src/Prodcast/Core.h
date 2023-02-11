#pragma once
#include <chrono>
#include <iostream>

#ifdef PC_PLATFORM_WINDOWS
	#ifdef PC_BUILD_DLL
		#define PC_API __declspec(dllexport)
	#else
		#define PC_API __declspec(dllimport)
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
		unsigned int sampleRate = 44100;
		unsigned int bufferSize = 512;
		int inputChannels = -1;
		int outputChannels = -1;
		Backends audioBackend = BE_PORTAUDIO;
	};

	// debug, temporary
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
}