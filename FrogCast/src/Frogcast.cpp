#include <ProdCast.h>
#include "AudioSource.h"
#include "ProcessingChain.h"
#include "Effects/Filters/PeakFilter.h"
#include <thread>
#include <iostream>

int main() {
	ProdCast::ProdCastEngine* engine = new ProdCast::ProdCastEngine(128, 48000, ProdCast::Backends::BE_PORTAUDIO);
	engine->setMasterGain(1.0f);

	ProdCast::AudioSource file1(engine);
	file1.LoadFile("F:/Dev/Projets/ProdCast/bin/Debug-x64/FrogCast/tests/flactest.flac");
	file1.setVolume(1.0f);

	ProdCast::ProcessingChain* proc = new ProdCast::ProcessingChain();
	ProdCast::PeakFilter* peakFilter = new ProdCast::PeakFilter(engine, 100.0f, 0.1f, 0.0f);
	proc->AddEffect(peakFilter);
	file1.ApplyProcessingChain(proc);
	std::cout << file1.getVolume();
	file1.Play();
	
	using namespace std::chrono_literals;

	std::this_thread::sleep_for(90s);
	delete engine;
	return 0;
}