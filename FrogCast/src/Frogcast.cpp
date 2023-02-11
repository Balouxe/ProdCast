#include <ProdCast.h>
#include <thread>

int main() {
	ProdCast::ProdCastEngine* engine = new ProdCast::ProdCastEngine(16, 48000, ProdCast::Backends::BE_PORTAUDIO);
	engine->setMasterGain(0.5f);

	ProdCast::AudioSource wav(engine);
	engine->AddAudioTrack(&wav);
	wav.LoadFile("F:/Dev/Projets/ProdCast/bin/Debug-x64/FrogCast/test2.wav");
	wav.Play();
	wav.setVolume(1.0f);
	using namespace std::chrono_literals;

	std::this_thread::sleep_for(5s);
	wav.setPan(0.5f);
	std::this_thread::sleep_for(5s);
	wav.setPan(-0.5f);
	std::this_thread::sleep_for(5s);

	delete engine;

	return 0;
}