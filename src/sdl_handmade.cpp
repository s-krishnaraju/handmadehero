#include <SDL.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>


#define internal static
#define global_variable static

// TODO: Maybe make these not global 
global_variable bool Running;


struct sdl_offscreen_buffer {
    SDL_Texture* Texture;
    void* Memory;
    int Width;
    int Height;
};

struct sdl_window_dimension {
    int Width;
    int Height;
};

global_variable const int BYTES_PER_PIXEL = 4; // 3 bytes for RGB + 1 for alignment 
global_variable sdl_offscreen_buffer GlobalBackBuffer;


internal void RenderWeirdGradient(sdl_offscreen_buffer Buffer, int BlueOffset, int GreenOffset) {
    int Pitch = Buffer.Width * BYTES_PER_PIXEL;
    Uint8* Row = (Uint8*)Buffer.Memory;
    for (int Y = 0;Y < Buffer.Height;Y++) {

        Uint32* Pixel = (Uint32*)Row;
        for (int X = 0; X < Buffer.Width; X++) {
            Uint8 Blue = (X + BlueOffset);
            Uint8 Green = (Y + GreenOffset);
            *Pixel = ((Green << 8) | Blue);
            Pixel += 1;
        }

        Row += Pitch;
    }
}

internal void SDLResizeTexture(sdl_offscreen_buffer* Buffer, SDL_Renderer* Renderer, sdl_window_dimension WindowDimension) {

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

internal void SDLUpdateWindow(sdl_offscreen_buffer Buffer, SDL_Renderer* Renderer) {
    SDL_UpdateTexture(Buffer.Texture, 0, Buffer.Memory, Buffer.Width * BYTES_PER_PIXEL);
    SDL_RenderCopy(Renderer, Buffer.Texture, 0, 0);
    SDL_RenderPresent(Renderer);
}

internal void HandleWindowEvent(SDL_WindowEvent Event) {
    SDL_Window* Window = SDL_GetWindowFromID(Event.windowID);
    SDL_Renderer* Renderer = SDL_GetRenderer(Window);
    switch (Event.event) {
    case SDL_WINDOWEVENT_EXPOSED:
    {
        SDLUpdateWindow(GlobalBackBuffer, Renderer);
    }
    break;

    case SDL_WINDOWEVENT_SIZE_CHANGED:
    {

    }
    break;
    }

}

internal void HandleEvent(SDL_Event Event) {
    switch (Event.type) {
    case SDL_WINDOWEVENT:
    {
        HandleWindowEvent(Event.window);
    }
    break;

    case SDL_QUIT:
    {
        printf("SDL QUIT\n");
        Running = false;
    }
    break;

    }
}

internal sdl_window_dimension GetWindowDimensions(SDL_Window* Window) {
    sdl_window_dimension dimension;
    SDL_GetWindowSize(Window, &dimension.Width, &dimension.Height);
    return dimension;
}

internal void StartEventLoop(SDL_Window* Window, SDL_Renderer* Renderer) {
    Running = true;
    sdl_window_dimension WindowDimensions = GetWindowDimensions(Window);
    SDLResizeTexture(&GlobalBackBuffer, Renderer, WindowDimensions);

    int XOffset = 0;
    int YOffset = 0;

    while (Running) {
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
            HandleEvent(Event);
        }
        RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
        SDLUpdateWindow(GlobalBackBuffer, Renderer);
        ++XOffset;
        YOffset += 2;
    }
}

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        // TODO: do something on error
    }

    SDL_Window* Window = SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
    if (!Window)
        return 0;
    SDL_Renderer* Renderer = SDL_CreateRenderer(Window, -1, 0);
    if (!Renderer)
        return 0;

    StartEventLoop(Window, Renderer);

    SDL_Quit();
    return 0;
}
