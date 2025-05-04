#include <SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define internal static
#define global_variable static
typedef float real32;
typedef double real64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct sdl_window_dimension {
    int Width;
    int Height;
};

struct sdl_offscreen_buffer {
    SDL_Texture *Texture;
    void *Memory;
    int Width;
    int Height;
};

struct sdl_audio_data {
    int SamplesPerSecond;
    int ToneHz;
    int ToneVolume;
    int BytesPerSample;
    Uint32 RunningSampleIndex;
};

// Constants
global_variable const int BYTES_PER_PIXEL = 4; // 3 bytes for RGB + 1 for alignment
global_variable const int MAX_CONTROLLERS = 4;
global_variable const real32 PI32 = 3.14159265358979f;
// TODO: Maybe make these not global
global_variable sdl_offscreen_buffer GlobalBackBuffer;
global_variable sdl_audio_data SDLAudioData;
global_variable bool Running;
// These pointers get initialized to zero since global
global_variable SDL_GameController *ControllerHandles[MAX_CONTROLLERS];
global_variable SDL_Haptic *HapticHandles[MAX_CONTROLLERS];

internal void RenderWeirdGradient(sdl_offscreen_buffer Buffer, int BlueOffset, int GreenOffset) {
    int Pitch = Buffer.Width * BYTES_PER_PIXEL;
    Uint8 *Row = (Uint8 *)Buffer.Memory;
    for (int Y = 0; Y < Buffer.Height; Y++) {

        Uint32 *Pixel = (Uint32 *)Row;
        for (int X = 0; X < Buffer.Width; X++) {
            Uint8 Blue = (X + BlueOffset);
            Uint8 Green = (Y + GreenOffset);
            *Pixel = ((Green << 8) | Blue);
            Pixel++;
        }

        Row += Pitch;
    }
}

internal void SDLResizeTexture(sdl_offscreen_buffer *Buffer, SDL_Renderer *Renderer, sdl_window_dimension WindowDimension) {

    if (Buffer->Memory) {
        munmap(Buffer->Memory, Buffer->Width * Buffer->Height * BYTES_PER_PIXEL);
    }

    if (Buffer->Texture) {
        SDL_DestroyTexture(Buffer->Texture);
    }

    Buffer->Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WindowDimension.Width, WindowDimension.Height);

    int NumPixels = WindowDimension.Width * WindowDimension.Height;
    Buffer->Memory = mmap(0, NumPixels * BYTES_PER_PIXEL, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    Buffer->Width = WindowDimension.Width;
    Buffer->Height = WindowDimension.Height;
}

internal void SDLUpdateWindow(sdl_offscreen_buffer Buffer, SDL_Renderer *Renderer) {
    SDL_UpdateTexture(Buffer.Texture, 0, Buffer.Memory, Buffer.Width * BYTES_PER_PIXEL);
    SDL_RenderCopy(Renderer, Buffer.Texture, 0, 0);
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

internal void HandleKeyboardEvent(SDL_KeyboardEvent Event) {
    SDL_Keycode Keycode = Event.keysym.sym;
    bool WasDown = false;
    if (Event.state == SDL_RELEASED || Event.repeat != 0) {
        WasDown = true;
    }

    switch (Keycode) {
    case SDLK_F4: {
        bool AltKeyWasDown = Event.keysym.mod & KMOD_ALT;
        if (AltKeyWasDown) {
            Running = false;
        }
    } break;
    case SDLK_ESCAPE: {
    } break;
    case SDLK_SPACE: {
    } break;
    case SDLK_RIGHT: {
    } break;
    case SDLK_LEFT: {
    } break;
    case SDLK_DOWN: {
        SDLAudioData.ToneHz -= 10; 

    } break;
    case SDLK_UP: {
        SDLAudioData.ToneHz += 10; 
    } break;
    case SDLK_e: {
    } break;
    case SDLK_q: {
    } break;
    case SDLK_d: {
    } break;
    case SDLK_s: {
    } break;
    case SDLK_w: {
    } break;
    case SDLK_a: {
        printf("A WAS PRESSED\n");
    } break;
    }
}

internal void HandleEvent(SDL_Event Event) {
    switch (Event.type) {
    case SDL_WINDOWEVENT: {
        HandleWindowEvent(Event.window);
    } break;

    case SDL_KEYDOWN:
    case SDL_KEYUP: {
        HandleKeyboardEvent(Event.key);
    } break;

    case SDL_QUIT: {
        printf("SDL QUIT\n");
        Running = false;
    } break;
    }
}

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

    for (int i = 0; i < MAX_CONTROLLERS; i++) {
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
    for (int i = 0; i < MAX_CONTROLLERS; i++) {
        if (ControllerHandles[i]) {
            SDL_GameControllerClose(ControllerHandles[i]);
        }
        if (HapticHandles[i]) {
            SDL_HapticClose(HapticHandles[i]);
        }
    }
}

internal void PollGameControllers() {
    for (int i = 0; i < MAX_CONTROLLERS; i++) {
        SDL_GameController *Controller = ControllerHandles[i];
        SDL_Haptic *Haptic = HapticHandles[i];
        if (Controller == 0 || !SDL_GameControllerGetAttached(Controller)) {
            // Controller not initialized or plugged in
            continue;
        }

        bool Up = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
        bool Down = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        bool Left = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        bool Right = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        bool AButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_A);
        bool BButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_B);
        bool XButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_X);
        bool YButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_Y);
        bool Start = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_START);
        bool Back = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_BACK);
        bool LeftShoulder = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        bool RightShoulder = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        int16 StickX = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTX);
        int16 StickY = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTY);

        // Do haptic on B button
        if (BButton) {
            if (Haptic) {
                SDL_HapticRumblePlay(Haptic, 0.8f, 2000);
            }
        }
    }
}

internal int16 GetSineSampleValue(sdl_audio_data *AudioData) {
    int WavePeriod = AudioData->SamplesPerSecond/ AudioData->ToneHz;
    real32 Time = (real32)AudioData->RunningSampleIndex / (real32)WavePeriod;
    real32 RadAngle = 2.0f * PI32 * Time;
    real32 SampleVal = sinf(RadAngle) * AudioData->ToneVolume;
    return (int16)SampleVal;
}

internal void SDLAudioCallback(void *UserData, Uint8 *Stream, int Length) {
    memset(Stream, 0, Length);
    sdl_audio_data *AudioData = (sdl_audio_data *)UserData;
    int16 *OutputStream = (int16 *)Stream;
    int NumSamples = Length / AudioData->BytesPerSample;

    // Maybe move this to main event loop
    // We want Audio to be tied to individual frames 
    // So keep a buffer and just memcpy in Stream here

    for (int i = 0; i < NumSamples; i++) {
        int16 SampleValue = GetSineSampleValue(AudioData);
        // Set left and right channels values
        *OutputStream = SampleValue;
        OutputStream++;
        *OutputStream = SampleValue;
        OutputStream++;
        AudioData->RunningSampleIndex++;
    }
}

internal SDL_AudioDeviceID InitSDLAudio() {
    // Placeholder, assume we are running 60 fps
    int FramesPerSecond = 60;
    SDLAudioData.ToneHz = 256;
    SDLAudioData.ToneVolume = 3000;
    SDLAudioData.SamplesPerSecond = 48000;
    // period is number of samples per cycle
    SDLAudioData.BytesPerSample = sizeof(int16) * 2;
    // Num samples per frame (left and right channel samples)
    int NumSamples = SDLAudioData.SamplesPerSecond / FramesPerSecond;

    SDL_AudioSpec AudioSpec = {0};
    SDL_AudioSpec DesiredAudioSpec = {0};
    DesiredAudioSpec.freq = SDLAudioData.SamplesPerSecond;
    // Samples are signed 16bit integers in little endian order
    DesiredAudioSpec.format = AUDIO_S16;
    DesiredAudioSpec.callback = &SDLAudioCallback;
    DesiredAudioSpec.channels = 2;
    // Divide b/c 2 channels
    DesiredAudioSpec.samples = NumSamples / 2;
    DesiredAudioSpec.userdata = &SDLAudioData;

    SDL_AudioDeviceID DeviceID = SDL_OpenAudioDevice(NULL, 0, &DesiredAudioSpec, &AudioSpec, 0);

    if (AudioSpec.format != AUDIO_S16) {
        printf("Obtained SDL Spec doesn't match requested format\n");
        SDL_CloseAudioDevice(DeviceID);

    } else {
        // Unpause device
        SDL_PauseAudioDevice(DeviceID, 0);
    }
    return DeviceID;
}

internal void StartEventLoop(SDL_Window *Window, SDL_Renderer *Renderer) {
    Running = true;
    sdl_window_dimension WindowDimensions = GetWindowDimensions(Window);
    SDLResizeTexture(&GlobalBackBuffer, Renderer, WindowDimensions);

    int XOffset = 0;
    int YOffset = 0;

    InitGameControllers();
    SDL_AudioDeviceID AudioDeviceID = InitSDLAudio();

    while (Running) {

        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
            HandleEvent(Event);
        }

        // Handle sine wave generation for audio, write to buffer
        PollGameControllers();
        RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
        SDLUpdateWindow(GlobalBackBuffer, Renderer);

        XOffset++;
        YOffset += 2;
    }

    SDL_CloseAudioDevice(AudioDeviceID);
    CloseGameControllers();
}

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO) != 0) {
        // TODO: do something on error
    }

    SDL_Window *Window = SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
    if (!Window)
        return 0;
    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
    if (!Renderer)
        return 0;

    StartEventLoop(Window, Renderer);

    SDL_Quit();
    return 0;
}
