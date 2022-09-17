#include "PortAudio.h"

#include <stdio.h>
#include <math.h>
#include "../Log.h"
#include <portaudio.h>
#include <iostream>

#define FRAMES_PER_BUFFER  (64)

#define TABLE_SIZE   (200)


PortAudioStream::PortAudioStream(): stream(0), left_phase(0), right_phase(0)
{
}

bool PortAudioStream::open(AudioSettings settings)
{
    PaStreamParameters outputParameters;
    outputParameters.device = settings.deviceIndex;
    if (outputParameters.device == paNoDevice) {
        return false;
    }

    outputParameters.channelCount = settings.channelCount;
    outputParameters.sampleFormat = settings.sampleFormat;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = settings.apiSpecificInfo;

    PaError err = Pa_OpenStream(
        &stream,
        NULL, /* no input */
        &outputParameters,
        settings.sampleRate,
        settings.framesPerBuffer,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        &PortAudioStream::paCallback,
        this       /* Using 'this' for userData so we can cast to Sine* in paCallback method */
    );

    if (err != paNoError)
    {
        /* Failed to open stream to device !!! */
        std::cout << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    err = Pa_SetStreamFinishedCallback(stream, &PortAudioStream::paStreamFinished);

    if (err != paNoError)
    {
        Pa_CloseStream(stream);
        stream = 0;

        return false;
    }

    return true;
}

bool PortAudioStream::close()
{
    if (stream == 0)
        return false;

    PaError err = Pa_CloseStream(stream);
    stream = 0;

    return (err == paNoError);
}

bool PortAudioStream::start()
{
    if (stream == 0)
        return false;

    PaError err = Pa_StartStream(stream);

    return (err == paNoError);
}

bool PortAudioStream::stop()
{
    if (stream == 0)
        return false;

    PaError err = Pa_StopStream(stream);

    return (err == paNoError);
}