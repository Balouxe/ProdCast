#include <ProdCast.h>
#include "AudioSource.h"
#include "ProcessingChain.h"
#include "Effects/Filters/IIRFilter.h"
#include <thread>
#include <iostream>

int main() {
	ProdCast::ProdCastEngine* engine = new ProdCast::ProdCastEngine(128, 48000, ProdCast::Backends::BE_PORTAUDIO);
	engine->setMasterGain(1.0f);

	ProdCast::AudioSource file1(engine);
	file1.LoadFile("F:/Dev/Projets/ProdCast/bin/Debug-x64/FrogCast/tests/flactest.flac");
	file1.setVolume(0.5f);

	ProdCast::ProcessingChain* proc = new ProdCast::ProcessingChain(engine);
	file1.ApplyProcessingChain(proc);
	file1.Play();
	
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(90s);
	delete engine;
	return 0;
}