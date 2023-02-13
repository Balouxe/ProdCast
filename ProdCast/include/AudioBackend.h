#pragma once
#include "Core.h"

namespace ProdCast {
	class ProdCastEngine;

	class PC_API AudioBackend {
	public:
		virtual PCError Init(AudioSettings* settings, ProdCastEngine* engine) = 0;
		virtual PCError DeInit() = 0;
	};

}