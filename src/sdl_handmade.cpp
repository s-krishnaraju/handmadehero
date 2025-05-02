#include <SDL.h>
#include <stdlib.h>

#define internal static
#define global_variable static

global_variable bool Running;
global_variable int TextureWidth;
global_variable SDL_Texture* Texture;
global_variable void* Pixels;
const int PIXEL_SIZE = 4;

internal void SDLResizeTexture(SDL_Renderer* Renderer, int Width, int Height) {
    if (Texture) {
        SDL_DestroyTexture(Texture);
    }

    if (Pixels) {
        free(Pixels);
    }

    SDL_Texture* Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, Width, Height);
    int NumPixels = Width * Height;
    void* Pixels = malloc(NumPixels * PIXEL_SIZE); //  Width*Height is num of pixels
    TextureWidth = Width;
}

internal bool HandleEvent(SDL_Event* Event) {
    switch (Event->type) {
    case SDL_WINDOWEVENT:
    {
        SDL_WindowEvent WindowEvent = Event->window;
        SDL_Window* window = SDL_GetWindowFromID(WindowEvent.windowID);
        switch (WindowEvent.event) {
        case SDL_WINDOWEVENT_EXPOSED:
        {

        }
        break;

        case SDL_WINDOWEVENT_RESIZED:
        {
            printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", WindowEvent.data1, WindowEvent.data2);
        }
        break;
        }
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

internal void StartEventLoop() {
    while (Running) // Main event loop
    {
        SDL_Event Event;
        SDL_WaitEvent(&Event);
        HandleEvent(&Event);
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

    StartEventLoop();

    SDL_Quit();
    return 0;
}
