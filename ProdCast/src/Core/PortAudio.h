#include <portaudio.h>
#include "AudioIO.h"
#include <stdio.h>
#include <math.h>
#include "../Log.h"
#include <portaudio.h>
#include <iostream>

#define SAMPLE_RATE   (48000)
#define FRAMES_PER_BUFFER  (64)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (200)

class PortAudioStream
{
public:
    PortAudioStream();

    bool open(AudioSettings settings);
    bool close();
    bool start();
    bool stop();

private:
    int paCallbackMethod(const void* inputBuffer, void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags)
    {
        float* out = (float*)outputBuffer;
        unsigned long i;

        (void)timeInfo;
        (void)statusFlags;
        (void)inputBuffer;
        return paContinue;

    }

    static int paCallback(const void* inputBuffer, void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData)
    {
        /* Here we cast userData to Sine* type so we can call the instance method paCallbackMethod, we can do that since
           we called Pa_OpenStream with 'this' for userData */
        return ((PortAudioStream*)userData)->paCallbackMethod(inputBuffer, outputBuffer,
            framesPerBuffer,
            timeInfo,
            statusFlags);
    }

    void paStreamFinishedMethod()
    {
        printf("Stream Completed: %s\n", message);
    }

    static void paStreamFinished(void* userData)
    {
        return ((PortAudioStream*)userData)->paStreamFinishedMethod();
    }

    PaStream* stream;
    int left_phase;
    int right_phase;
    char message[20];
};