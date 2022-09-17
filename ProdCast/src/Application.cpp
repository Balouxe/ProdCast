#include "InitializeImGui.h"
#include "Application.h"
#include "Log.h"
#include "Core/AudioIO.h"
#include<iostream>
#include<chrono>

struct Timer {
private:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;

public:

	Timer() {
		start = std::chrono::high_resolution_clock::now();
	}
	~Timer() {
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		float ms = duration.count() * 1000.0f;
		std::cout << "Took " << ms << " ms.";
	}
};

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif


int main(int, char**)
{
	Log::Init();
	AudioSettings settings;
	AudioEngine();
}