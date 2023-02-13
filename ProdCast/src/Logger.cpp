#include "Logger.h"
#ifndef PC_NLOG

namespace ProdCast {

	std::shared_ptr<spdlog::logger> Logger::m_Logger;

	void Logger::Init() {
		spdlog::set_pattern("%^[%T.%e] %n: %v%$");
		m_Logger = spdlog::stdout_color_mt("PRODCAST");
		m_Logger->set_level(spdlog::level::trace);
		m_Logger->info("Initialized logger.");
	}
}

#endif