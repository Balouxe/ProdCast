#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include<memory>
class Log
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; };

private:
	static std::shared_ptr<spdlog::logger> s_Logger;
};

#define PC_TRACE(...) ::Log::GetLogger()->trace(__VA_ARGS__)
#define PC_INFO(...) ::Log::GetLogger()->info(__VA_ARGS__)
#define PC_WARN(...) ::Log::GetLogger()->warn(__VA_ARGS__)
#define PC_ERROR(...) ::Log::GetLogger()->error(__VA_ARGS__)
#define PC_FATAL(...) ::Log::GetLogger()->fatal(__VA_ARGS__)