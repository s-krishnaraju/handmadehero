#include "handmade.h"
#include <math.h>

global_variable const real32 PI32 = 3.14159265358979f;

internal int16 GetSineSampleValue(int SampleIndex, int ToneVolume, int WavePeriod) {
    real32 Time = (real32)SampleIndex / (real32)WavePeriod;
    real32 RadAngle = 2.0f * PI32 * Time;
    real32 SampleVal = sinf(RadAngle) * ToneVolume;
    return (int16)SampleVal;
}

void OutputGameSound(game_sound_output_buffer *SoundBuffer) {
    global_variable int ToneHz = 256;
    global_variable int ToneVolume = 3000;
    global_variable int SampleIndex = 0;
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16 *Buffer = SoundBuffer->Samples;
    for (int i = 0; i < SoundBuffer->SampleCount; i++) {
        int SampleValue = GetSineSampleValue(SampleIndex, ToneVolume, WavePeriod);
        *Buffer++ = SampleValue;
        *Buffer++ = SampleValue;
        SampleIndex++;
    }
}

internal void RenderWeirdGradient(game_offscreen_buffer *Buffer) {
    global_variable int BlueOffset;
    global_variable int GreenOffset;

    uint8 *Row = (uint8 *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; Y++) {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer->Width; X++) {
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);
            *Pixel = ((Green << 8) | Blue);
            Pixel++;
        }

        Row += Buffer->Pitch;
    }

    BlueOffset += 2;
    GreenOffset += 1;
}

// Needs controller/keyboard input, timing, sound buffer, backbuffer/bitmap for drawing
void GameUpdateAndRender(game_offscreen_buffer *BackBuffer) {
    RenderWeirdGradient(BackBuffer);
}