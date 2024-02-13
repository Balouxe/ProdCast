# ProdCast
Welcome to the ProdCast engine, audio engine focused primarly on DAWs/Audio Softwares; however will probably support stuff for games as well.
This is a personal project that I'm doing in my free time so the development is slow.

## State of Dev
This audio engine currently supports:
- WAV, MP3, FLAC files loading
- Simultaneous audio sources and buses playing
- Processing chains and VST 3

In development :
- 3D audio
- Resampling

Planned :
- Audio file streaming (currently only preloading)

## Why should you use ProdCast ?
Currently, no reason. Later down the line, the main benefit of this engine will be simplicity of its code (at least that's my goal).

## Repo structure
The `ProdCast` folder contains the engine source code. The `FrogCast` folder contains a test Visual Studio solution

## Acknowledgements
Thanks to spdlog, portaudio, dr_libs for their wonderful libraries.
Thanks to @hotwatermorning for providing an example of a VST 3 hoster implementation, even if he uses his own data types >:(
