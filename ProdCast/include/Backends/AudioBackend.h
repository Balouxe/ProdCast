#pragma once
#include "Core.h"

#include <unordered_map>

namespace ProdCast {
	class ProdCastEngine;

	class PC_API AudioBackend {
	public:
		virtual PCError Init(AudioSettings* settings, ProdCastEngine* engine) = 0;
		virtual PCError DeInit() = 0;
		virtual PCError OpenAuxiliaryStream(uint32_t handle, AudioSettings& settings) = 0;
		virtual PCError CloseAuxiliaryStream(uint32_t handle) = 0;

		virtual std::unordered_map<int, std::string> GetInputDevices() = 0;
		virtual std::unordered_map<int, std::string> GetOutputDevices() = 0;
	};

}