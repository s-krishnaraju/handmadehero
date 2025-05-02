#include <SDL.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>


#define internal static
#define global_variable static

// TODO: Maybe make these not global 
global_variable bool Running;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable void* BitmapMemory;
global_variable int BitmapSize;
global_variable SDL_Texture* Texture;
const int BYTES_PER_PIXEL = 4; // 3 bytes for RGB + 1 for alignment 


internal void RenderWeirdGradient(int BlueOffset, int GreenOffset) {
    int Pitch = BitmapWidth * BYTES_PER_PIXEL;
    Uint8* Row = (Uint8*)BitmapMemory;
    for (int Y = 0;Y < BitmapHeight;Y++) {

        Uint32* Pixel = (Uint32*)Row;
        for (int X = 0; X < BitmapWidth; X++) {
            Uint8 Blue = (X + BlueOffset);
            Uint8 Green = (Y + GreenOffset);
            *Pixel = ((Green << 8) | Blue);
            Pixel += 1;
        }
        
        Row += Pitch;
    }
}

internal void SDLResizeTexture(SDL_Renderer* Renderer, int Width, int Height) {

    if (BitmapMemory) {
        munmap(BitmapMemory, BitmapWidth * BitmapHeight * BYTES_PER_PIXEL);
    }

    if (Texture) {
        SDL_DestroyTexture(Texture);
    }

    Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, Width, Height);

    int NumPixels = Width * Height;
    BitmapMemory = mmap(0, NumPixels * BYTES_PER_PIXEL, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    BitmapWidth = Width;
    BitmapHeight = Height;
}

internal void SDLUpdateWindow(SDL_Renderer* Renderer) {
    SDL_UpdateTexture(Texture, 0, BitmapMemory, BitmapWidth * BYTES_PER_PIXEL);
    SDL_RenderCopy(Renderer, Texture, 0, 0);
    SDL_RenderPresent(Renderer);
}

internal void HandleWindowEvent(SDL_WindowEvent Event) {
    SDL_Window* Window = SDL_GetWindowFromID(Event.windowID);
    SDL_Renderer* Renderer = SDL_GetRenderer(Window);
    switch (Event.event) {
    case SDL_WINDOWEVENT_EXPOSED:
    {
        SDLUpdateWindow(Renderer);
    }
    break;

    case SDL_WINDOWEVENT_SIZE_CHANGED:
    {
        SDLResizeTexture(Renderer, Event.data1, Event.data2);
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

    default:
    {
    }
    break;
    }
}

internal void StartEventLoop(SDL_Window* Window, SDL_Renderer* Renderer) {
    Running = true;

    int Width, Height;
    SDL_GetWindowSize(Window, &Width, &Height);
    SDLResizeTexture(Renderer, Width, Height);

    int XOffset = 0;
    int YOffset = 0;

    while (Running) {
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
            HandleEvent(Event);
        }
        RenderWeirdGradient(XOffset, YOffset);
        SDLUpdateWindow(Renderer);
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
