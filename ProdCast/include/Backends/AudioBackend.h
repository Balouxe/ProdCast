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

		virtual std::unordered_map<uint32_t, DeviceInfo> GetInputDevices() = 0;
		virtual std::unordered_map<uint32_t, DeviceInfo> GetOutputDevices() = 0;

		virtual void ChangeOutputDevice(uint32_t streamHandle, uint32_t deviceId) = 0;
		virtual void ChangeInputDevice(uint32_t streamHandle, uint32_t deviceId) = 0;
	};

}