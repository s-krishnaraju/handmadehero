/*
NOT FINAL PLATFORM LAYER
We maybe want
 - Saved game location
 - Handle to our exe
 - Asset loading path
 - Threading
 - Raw input
 - Sleep/ timeBeginPeriod
 - FullScreen support
 - Blit Speed improvements (do we need this? )
 - Hardware Acceleration (OpenGL)
*/

#include <math.h>
#include <stdint.h>

#define local_persist static
#define internal static
#define global_variable static
#define Pi32 3.14159265359f

typedef float real32;
typedef double real64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#include "handmade.cpp"

#include <SDL.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "sdl_handmade.h"

// TODO: Maybe make these not global
global_variable SDL_Texture *GlobalSDLTexture;
global_variable game_offscreen_buffer GlobalBackBuffer;
global_variable bool32 GlobalRunning;
// These pointers get initialized to zero since global
global_variable SDL_GameController *ControllerHandles[NUM_GAME_CONTROLLERS];
global_variable SDL_Haptic *HapticHandles[NUM_GAME_CONTROLLERS];

internal void DEBUGPlatformFreeFileMemory(void *Memory) { free(Memory); }

internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize,
                                             void *Memory) {
    int FileHandle = open(Filename, O_WRONLY | O_CREAT,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (FileHandle == -1) {
        return false;
    }

    uint32 BytesToWrite = MemorySize;
    uint8 *NextByteLocation = (uint8 *)Memory;
    while (BytesToWrite) {
        int BytesWritten = write(FileHandle, NextByteLocation, BytesToWrite);
        if (BytesWritten == -1) {
            close(FileHandle);
            return false;
        }

        NextByteLocation += BytesWritten;
        BytesToWrite -= BytesWritten;
    }

    close(FileHandle);
    return true;
}

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename) {
    debug_read_file_result Result = {};
    int FileHandle = open(Filename, O_RDONLY);
    if (FileHandle == -1) {
        return Result;
    }

    struct stat FileStatus;
    if (fstat(FileHandle, &FileStatus) == -1) {
        return Result;
    }

    Result.ContentSize = SafeTrunacateUint64(FileStatus.st_size);
    Result.Contents = malloc(Result.ContentSize);
    if (!Result.Contents) {
        Result.ContentSize = 0;
        close(FileHandle);
        return Result;
    }

    uint32 BytesToRead = Result.ContentSize;
    uint8 *NextByteLocation = (uint8 *)Result.Contents;
    while (BytesToRead) {
        int ReadBytes = read(FileHandle, NextByteLocation, BytesToRead);
        if (ReadBytes == -1) {
            free(Result.Contents);
            Result.Contents = 0;
            Result.ContentSize = 0;
            close(FileHandle);
            return Result;
        }
        BytesToRead -= ReadBytes;
        NextByteLocation += ReadBytes;
    }

    close(FileHandle);
    return Result;
}

internal void SDLResizeTexture(game_offscreen_buffer *Buffer,
                               SDL_Renderer *Renderer,
                               sdl_window_dimension WindowDimension) {
    int BytesPerPixel = 4; // 3 bytes for RGB + 1 for alignment

    if (Buffer->Memory) {
        munmap(Buffer->Memory, Buffer->Width * Buffer->Height * BytesPerPixel);
    }

    if (GlobalSDLTexture) {
        SDL_DestroyTexture(GlobalSDLTexture);
    }

    int NumPixels = WindowDimension.Width * WindowDimension.Height;
    GlobalSDLTexture = SDL_CreateTexture(
        Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        WindowDimension.Width, WindowDimension.Height);
    Buffer->Memory = mmap(0, NumPixels * BytesPerPixel, PROT_READ | PROT_WRITE,
                          MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    Buffer->Width = WindowDimension.Width;
    Buffer->Height = WindowDimension.Height;
    Buffer->Pitch = BytesPerPixel * Buffer->Width;
}

internal void SDLUpdateWindow(game_offscreen_buffer Buffer,
                              SDL_Renderer *Renderer) {
    SDL_RenderClear(Renderer);
    SDL_UpdateTexture(GlobalSDLTexture, 0, Buffer.Memory, Buffer.Pitch);
    SDL_RenderCopy(Renderer, GlobalSDLTexture, 0, 0);
    SDL_RenderPresent(Renderer);
}

internal void HandleWindowEvent(SDL_WindowEvent Event) {
    SDL_Window *Window = SDL_GetWindowFromID(Event.windowID);
    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
    switch (Event.event) {
    case SDL_WINDOWEVENT_EXPOSED: {
        SDLUpdateWindow(GlobalBackBuffer, Renderer);
    } break;

    case SDL_WINDOWEVENT_SIZE_CHANGED: {

    } break;
    }
}

internal void SDLProcessKeyPress(game_button_state *NewState, bool32 isDown) {
    NewState->EndedDown = isDown;
    NewState->HalfTransition++;
}

internal void HandleKeyboardEvent(SDL_KeyboardEvent Event, game_controller_input* Controller) {
    SDL_Keycode Keycode = Event.keysym.sym;
    bool32 IsDown = Event.state == SDL_PRESSED;
    bool32 WasDown = false;
    if (Event.state == SDL_RELEASED || Event.repeat != 0) {
        WasDown = true;
    }


    if (Keycode == SDLK_F4) {
        bool32 AltKeyWasDown = Event.keysym.mod & KMOD_ALT;
        if (AltKeyWasDown) {
            GlobalRunning = false;
        }
        return;
    }

    if (Event.repeat != 0) {
        return;
    }

    switch (Keycode) {
    case SDLK_w: {
        SDLProcessKeyPress(&Controller->MoveUp, IsDown);
    } break;

    case SDLK_a: {
        SDLProcessKeyPress(&Controller->MoveLeft, IsDown);
    } break;
    case SDLK_s: {
        SDLProcessKeyPress(&Controller->MoveDown, IsDown);
    } break;
    case SDLK_d: {
        SDLProcessKeyPress(&Controller->MoveRight, IsDown);
    } break;
    case SDLK_q: {
        SDLProcessKeyPress(&Controller->LeftShoulder, IsDown);
    } break;
    case SDLK_e: {
        SDLProcessKeyPress(&Controller->RightShoulder, IsDown);
    } break;
    case SDLK_UP: {
        SDLProcessKeyPress(&Controller->ActionUp, IsDown);
    } break;
    case SDLK_LEFT: {
        SDLProcessKeyPress(&Controller->ActionLeft, IsDown);
    } break;
    case SDLK_DOWN: {
        SDLProcessKeyPress(&Controller->ActionDown, IsDown);
    } break;
    case SDLK_RIGHT: {
        SDLProcessKeyPress(&Controller->ActionRight, IsDown);
    } break;
    case SDLK_ESCAPE: {
    } break;
    case SDLK_SPACE: {
    } break;
    }
}

internal void HandleEvent(SDL_Event Event) {}

internal sdl_window_dimension GetWindowDimensions(SDL_Window *Window) {
    sdl_window_dimension dimension;
    SDL_GetWindowSize(Window, &dimension.Width, &dimension.Height);
    return dimension;
}

internal void InitGameControllers() {
    int MaxJoysticks = SDL_NumJoysticks();
    if (MaxJoysticks < 0) {
        // TODO: handle error here
        return;
    }

    for (int i = 0; i < NUM_GAME_CONTROLLERS; i++) {
        if (SDL_IsGameController(i)) {
            SDL_GameController *Controller = SDL_GameControllerOpen(i);
            ControllerHandles[i] = Controller;
            SDL_Joystick *Joystick = SDL_GameControllerGetJoystick(Controller);
            SDL_Haptic *Haptic = SDL_HapticOpenFromJoystick(Joystick);
            HapticHandles[i] = Haptic;
            if (SDL_HapticRumbleInit(Haptic) != 0) {
                SDL_HapticClose(Haptic);
                HapticHandles[i] = 0;
            }
        }
    }
}

internal void CloseGameControllers() {
    for (int i = 0; i < NUM_GAME_CONTROLLERS; i++) {
        if (ControllerHandles[i]) {
            SDL_GameControllerClose(ControllerHandles[i]);
        }
        if (HapticHandles[i]) {
            SDL_HapticClose(HapticHandles[i]);
        }
    }
}

internal void SDLProcessButtonState(SDL_GameController *ControllerHandle,
                                    SDL_GameControllerButton Button,
                                    game_button_state *NewState,
                                    game_button_state *OldState) {
    NewState->EndedDown = SDL_GameControllerGetButton(ControllerHandle, Button);
    NewState->HalfTransition = (NewState->EndedDown != OldState->EndedDown);
}

internal void PollGameControllers(game_input *NewInput, game_input *OldInput) {
    for (int i = 0; i < NUM_GAME_CONTROLLERS; i++) {
        SDL_GameController *ControllerHandle = ControllerHandles[i];
        SDL_Haptic *Haptic = HapticHandles[i];
        if (ControllerHandle == 0 ||
            !SDL_GameControllerGetAttached(ControllerHandle)) {
            // Controller not initialized or plugged in
            continue;
        }

        game_controller_input *NewController =
            GetGameController(NewInput, i + NUM_KEYBOARD_CONTROLLERS);
        *NewController = {0}; // Zero Controller

        game_controller_input *OldController =
            GetGameController(OldInput, i + NUM_KEYBOARD_CONTROLLERS);

        SDLProcessButtonState(ControllerHandle, SDL_CONTROLLER_BUTTON_DPAD_UP,
                              &NewController->MoveUp, &OldController->MoveUp);
        SDLProcessButtonState(ControllerHandle, SDL_CONTROLLER_BUTTON_DPAD_DOWN,
                              &NewController->MoveDown,
                              &OldController->MoveDown);
        SDLProcessButtonState(ControllerHandle, SDL_CONTROLLER_BUTTON_DPAD_LEFT,
                              &NewController->MoveLeft,
                              &OldController->MoveLeft);
        SDLProcessButtonState(
            ControllerHandle, SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
            &NewController->MoveRight, &OldController->MoveRight);
        SDLProcessButtonState(
            ControllerHandle, SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
            &NewController->LeftShoulder, &OldController->LeftShoulder);
        SDLProcessButtonState(
            ControllerHandle, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
            &NewController->RightShoulder, &OldController->RightShoulder);
        NewController->StartX = OldController->EndX;
        NewController->StartY = OldController->EndY;

        int16 StickX = SDL_GameControllerGetAxis(ControllerHandle,
                                                 SDL_CONTROLLER_AXIS_LEFTX);
        int16 StickY = SDL_GameControllerGetAxis(ControllerHandle,
                                                 SDL_CONTROLLER_AXIS_LEFTY);

        if (StickX < 0) {
            NewController->EndX = StickX / 32768.0f;
        } else {
            NewController->EndX = StickX / 32767.0f;
        }
        NewController->MinX = NewController->MaxX = NewController->EndX;

        if (StickY < 0) {
            NewController->EndY = StickY / 32768.0f;
        } else {
            NewController->EndY = StickY / 32767.0f;
        }
        NewController->MinY = NewController->MaxY = NewController->EndY;

        // bool32 XButton = SDL_GameControllerGetButton(ControllerHandle,
        // SDL_CONTROLLER_BUTTON_X); bool32 YButton =
        // SDL_GameControllerGetButton(ControllerHandle,
        // SDL_CONTROLLER_BUTTON_Y); bool32 Start =
        // SDL_GameControllerGetButton(ControllerHandle,
        // SDL_CONTROLLER_BUTTON_START); bool32 Back =
        // SDL_GameControllerGetButton(ControllerHandle,
        // SDL_CONTROLLER_BUTTON_BACK);
    }
}

internal int GetSamplesToWrite(circular_audio_buffer *AudioBuffer) {
    int TargetCursor =
        (AudioBuffer->ReadCursor -
         AudioBuffer->LatencySampleCount * AudioBuffer->BytesPerSample) %
        AudioBuffer->Size;
    int RegionOne = TargetCursor - AudioBuffer->WriteCursor;
    int RegionTwo = 0;
    if (TargetCursor < AudioBuffer->WriteCursor) {
        RegionOne = AudioBuffer->Size - AudioBuffer->WriteCursor;
        RegionTwo = TargetCursor;
    }

    int BytesToWrite = RegionOne + RegionTwo;
    return BytesToWrite / AudioBuffer->BytesPerSample;
}

internal void SDLFeedAudioDevice(circular_audio_buffer *AudioBuffer,
                                 game_sound_output_buffer *SoundBuffer) {

    SDL_LockAudioDevice(AudioBuffer->DeviceID);
    int16 *Buffer1;
    int16 *Buffer2 = SoundBuffer->Samples;
    int BytesOffset = AudioBuffer->WriteCursor;

    for (int i = 0; i < SoundBuffer->SampleCount; i++) {
        BytesOffset += AudioBuffer->BytesPerSample;
        BytesOffset = BytesOffset % AudioBuffer->Size;
        Buffer1 = (int16 *)(AudioBuffer->Buffer + BytesOffset);
        *Buffer1++ = *Buffer2++;
        *Buffer1++ = *Buffer2++;
        AudioBuffer->SampleIndex++;
    }

    int BytesWritten = SoundBuffer->SampleCount * AudioBuffer->BytesPerSample;
    AudioBuffer->WriteCursor =
        (AudioBuffer->WriteCursor + BytesWritten) % AudioBuffer->Size;
    SDL_UnlockAudioDevice(AudioBuffer->DeviceID);
}

internal void SDLAudioCallback(void *UserData, uint8 *OutputAudio, int Length) {
    circular_audio_buffer *AudioBuffer = (circular_audio_buffer *)UserData;
    int RegionOne = Length;
    int RegionTwo = 0;
    if (AudioBuffer->ReadCursor + Length > AudioBuffer->Size) {
        RegionOne = AudioBuffer->Size - AudioBuffer->ReadCursor;
        RegionTwo = Length - RegionOne;
    }

    memcpy(OutputAudio, AudioBuffer->Buffer + AudioBuffer->ReadCursor,
           RegionOne);
    memcpy(OutputAudio + RegionOne, AudioBuffer->Buffer, RegionTwo);
    AudioBuffer->ReadCursor =
        (AudioBuffer->ReadCursor + Length) % AudioBuffer->Size;
}

internal void InitSDLAudio(circular_audio_buffer *AudioBuffer,
                           game_sound_output_buffer *SoundOutput) {

    AudioBuffer->SamplesPerSecond = 44100;
    AudioBuffer->BytesPerSample = sizeof(int16) * 2;
    AudioBuffer->Size =
        (AudioBuffer->SamplesPerSecond * AudioBuffer->BytesPerSample);
    AudioBuffer->Buffer = (uint8 *)calloc(AudioBuffer->Size, 1);
    // NOTE: offset write cursor by one sample to fill initial buffer
    AudioBuffer->WriteCursor = AudioBuffer->BytesPerSample;
    AudioBuffer->ReadCursor = 0;
    AudioBuffer->LatencySampleCount = AudioBuffer->SamplesPerSecond / 15;

    SoundOutput->SamplesPerSecond = AudioBuffer->SamplesPerSecond;
    SoundOutput->Samples = (int16 *)calloc(AudioBuffer->Size, 1);

    SDL_AudioSpec DesiredAudioSpec = {};
    SDL_AudioSpec ObtainedAudioSpec = {};

    DesiredAudioSpec.freq = AudioBuffer->SamplesPerSecond;
    DesiredAudioSpec.format = AUDIO_S16;
    DesiredAudioSpec.callback = &SDLAudioCallback;
    DesiredAudioSpec.channels = 2;
    DesiredAudioSpec.samples = 4096;
    DesiredAudioSpec.userdata = AudioBuffer;

    AudioBuffer->DeviceID =
        SDL_OpenAudioDevice(NULL, 0, &DesiredAudioSpec, &ObtainedAudioSpec, 0);

    if (ObtainedAudioSpec.format != DesiredAudioSpec.format) {
        printf("Obtained SDL Spec doesn't match requested format\n");
        GlobalRunning = false;
        return;
    }

    SDL_PauseAudioDevice(AudioBuffer->DeviceID, 0);
}

internal void AllocateGameMemory(game_memory *Memory) {

#if HANDMADE_INTERNAL
    void *BaseAddress = (void *)Terabytes(2);
#else
    void *BaseAddress = (void *)(0);
#endif

    Memory->PermanentStorageSize = Megabytes(64);
    Memory->TransientStorageSize = Gigabytes(4);
    uint64 TotalStorageSize =
        Memory->PermanentStorageSize + Memory->TransientStorageSize;
    Memory->PermanentStorage = mmap(0, TotalStorageSize, PROT_READ | PROT_WRITE,
                                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    Assert(Memory->PermanentStorage);

    Memory->TransientStorage =
        ((uint8 *)Memory->PermanentStorage + Memory->PermanentStorageSize);
}

internal void StartEventLoop(SDL_Window *Window, SDL_Renderer *Renderer) {
    GlobalRunning = true;
    circular_audio_buffer CircularBuffer;
    game_sound_output_buffer GameSoundBuffer;
    game_memory GameMemory = {};
    uint64 CountersPerSecond = SDL_GetPerformanceFrequency();
    uint64 LastCounter = SDL_GetPerformanceCounter();

    game_input Input[2] = {0};
    game_input *NewInput = &Input[1];
    game_input *OldInput = &Input[0];

    AllocateGameMemory(&GameMemory);
    SDLResizeTexture(&GlobalBackBuffer, Renderer, GetWindowDimensions(Window));
    InitGameControllers();
    InitSDLAudio(&CircularBuffer, &GameSoundBuffer);

    if (!GameMemory.PermanentStorage || !GameMemory.TransientStorage ||
        !GameSoundBuffer.Samples) {
        return;
    }

    while (GlobalRunning) {

        // We do this outside HandleKeyboardEvent b/c it only triggers if we get an event
        // Init keyboard controller (first controller) and set to empty
        game_controller_input *NewKeyboardController = GetGameController(NewInput, 0);
        *NewKeyboardController = {0}; // Zero Controller
        game_controller_input *OldKeyboardController = GetGameController(OldInput, 0);
        for (int i = 0; i < ArrayCount(OldKeyboardController->Buttons); i++) {
            NewKeyboardController->Buttons[i].EndedDown =
                OldKeyboardController->Buttons[i].EndedDown;
        }

        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
            switch (Event.type) {
            case SDL_WINDOWEVENT: {
                HandleWindowEvent(Event.window);
            } break;

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                HandleKeyboardEvent(Event.key, NewKeyboardController);
            } break;

            case SDL_QUIT: {
                printf("SDL QUIT\n");
                GlobalRunning = false;
            } break;
            }
        }

        // Init stuff for game call
        PollGameControllers(NewInput, OldInput);
        GameSoundBuffer.SampleCount = GetSamplesToWrite(&CircularBuffer);

        // Main game call (render, input, sound, etc ...)
        GameUpdateAndRender(&GameMemory, &GlobalBackBuffer, &GameSoundBuffer,
                            NewInput);

        // Platform stuff to show results from game
        SDLFeedAudioDevice(&CircularBuffer, &GameSoundBuffer);
        SDLUpdateWindow(GlobalBackBuffer, Renderer);

        int EndCounter = SDL_GetPerformanceCounter();
        int ElapsedCounters = EndCounter - LastCounter;
        int MSPerFrame = (ElapsedCounters * 1000) / CountersPerSecond;
        int FPS = CountersPerSecond / ElapsedCounters;
        LastCounter = EndCounter;

        // Set old input
        game_input *Temp = OldInput;
        OldInput = NewInput;
        NewInput = Temp;
        *NewInput = {0};

    }

    SDL_CloseAudioDevice(CircularBuffer.DeviceID);
    // We might want to free some more ptrs
    free(CircularBuffer.Buffer);
    free(GameSoundBuffer.Samples);
    CloseGameControllers();
}

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC |
                 SDL_INIT_AUDIO) != 0) {
        // TODO: do something on error
    }

    SDL_Window *Window = SDL_CreateWindow(
        "Handmade Hero", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640,
        480, SDL_WINDOW_RESIZABLE);
    if (!Window)
        return 0;
    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
    if (!Renderer)
        return 0;

    StartEventLoop(Window, Renderer);

    SDL_Quit();
    return 0;
}
