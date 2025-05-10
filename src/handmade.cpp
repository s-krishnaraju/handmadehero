#include "handmade.h"
#include <stdio.h>

// global_variable int ToneHz = 256;
// global_variable int ToneVolume = 3000;
// global_variable int SampleIndex = 0;
// global_variable int BlueOffset = 0;
// global_variable int GreenOffset = 0;

internal int16 GetSineSampleValue(int SampleIndex, int ToneVolume,
                                  int WavePeriod) {
    real32 Time = (real32)SampleIndex / (real32)WavePeriod;
    real32 RadAngle = 2.0f * Pi32 * Time;
    real32 SampleVal = sinf(RadAngle) * ToneVolume;
    return (int16)SampleVal;
}

internal void OutputGameSound(game_sound_output_buffer *SoundBuffer, int ToneHz, int ToneVolume) {
    local_persist int SoundSampleIndex = 0; 
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;
    int16 *Buffer = SoundBuffer->Samples;
    for (int i = 0; i < SoundBuffer->SampleCount; i++) {
        int SampleValue =
            GetSineSampleValue(SoundSampleIndex, ToneVolume, WavePeriod);
        *Buffer++ = SampleValue;
        *Buffer++ = SampleValue;
        SoundSampleIndex++;
    }
}

internal void RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset) {

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
}

// Needs controller/keyboard input, timing, sound buffer, backbuffer/bitmap for
// drawing
internal void GameUpdateAndRender(game_memory *Memory,
                                  game_offscreen_buffer *BackBuffer,
                                  game_sound_output_buffer *SoundBuffer,
                                  game_input *Input) {
    game_state *GameState = (game_state *)Memory;
    
    if (!Memory->IsInitialized) {
        GameState->BlueOffset = 0;
        GameState->GreenOffset = 0;
        GameState->ToneHz = 256;
        GameState->ToneVolume = 3000;
        Memory->IsInitialized = true;
    }

    for (int i = 0; i < ArrayCount(Input->Controllers); i++) {
        game_controller_input *Controller = GetGameController(Input, i);
        if (Controller->IsAnalog) {
            //NOTE: Use analog movement tuning
            GameState->ToneHz = 256.0f + (128.0f*Controller->StickAverageY);
            GameState->BlueOffset += (int)(4.0f*Controller->StickAverageX);
        } else { 
            // Handle digital movements
            if (Controller->ActionLeft.EndedDown) {
                GameState->BlueOffset -= 2;
            }
            if (Controller->ActionRight.EndedDown) {
                GameState->BlueOffset += 2;
            }
        
            if (Controller->ActionUp.EndedDown) {
                GameState->GreenOffset -= 2;
            }
        
            if (Controller->ActionDown.EndedDown) {
                GameState->GreenOffset += 2;
            }
        }
    }


    OutputGameSound(SoundBuffer, GameState->ToneHz, GameState->ToneVolume );
    RenderWeirdGradient(BackBuffer, GameState->BlueOffset, GameState->GreenOffset);
    // TODO: Handle game controller input
}
