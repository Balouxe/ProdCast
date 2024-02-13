#pragma once
#ifndef PC_NLOG

#include <iostream>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

/* Thanks Cherno for the logging tutorial lol
Gonna have file logging one day but for now this will do
*/

namespace ProdCast {
	class Logger {
	public:
		static void Init() {
			spdlog::set_pattern("%^[%T.%e] %n: %v%$");
			m_Logger = spdlog::stdout_color_mt("PRODCAST");
			m_Logger->set_level(spdlog::level::trace);
			m_Logger->info("Initialized logger.");
		}	

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return m_Logger; };
	private:
		static std::shared_ptr<spdlog::logger> m_Logger;
	};
}

#define PC_TRACE(...) ::ProdCast::Logger::GetLogger()->trace(__VA_ARGS__)
#define PC_INFO(...) ::ProdCast::Logger::GetLogger()->info(__VA_ARGS__)
#define PC_WARN(...) ::ProdCast::Logger::GetLogger()->warn(__VA_ARGS__)
#define PC_ERROR(...) ::ProdCast::Logger::GetLogger()->error(__VA_ARGS__)
#define PC_FATAL(...) ::ProdCast::Logger::GetLogger()->critical(__VA_ARGS__)

#else
#define PC_TRACE(...)
#define PC_INFO(...)
#define PC_WARN(...)
#define PC_ERROR(...)
#define PC_FATAL(...)
#endif