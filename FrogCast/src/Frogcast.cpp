#include <ProdCast.h>
#include <thread>

int main() {
	ProdCast::ProdCastEngine* engine = new ProdCast::ProdCastEngine(128, 48000, ProdCast::Backends::BE_PORTAUDIO);
	engine->setMasterGain(0.5f);

	ProdCast::AudioSource file1(engine);

	file1.LoadFile("F:/Dev/Projets/ProdCast/bin/Debug-x64/FrogCast/tests/flactest.flac");
	file1.setVolume(1.0f);
	file1.Play();
	
	using namespace std::chrono_literals;

	std::this_thread::sleep_for(90s);
	delete engine;
	return 0;
}