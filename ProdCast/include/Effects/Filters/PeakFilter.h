#pragma once
#include "Core.h"
#include "IIRFilter.h"

namespace ProdCast {
	class ProdCastEngine;

	class PC_API PeakFilter : public IIRFilter {
	public:
		PeakFilter(ProdCastEngine* engine);
		PeakFilter(ProdCastEngine* engine, float frequency, float Q, float gain);

		void calculateIntermediates();
	};
}