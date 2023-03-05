#include "Logger.h"
#ifndef PC_NLOG
std::shared_ptr<spdlog::logger> ProdCast::Logger::m_Logger;
#endif