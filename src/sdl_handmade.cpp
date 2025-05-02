#include <SDL.h>

void DrawOnWindow(Uint32 WindowID){ 
    SDL_Window *Window = SDL_GetWindowFromID(WindowID); 
    SDL_Renderer *Renderer = SDL_GetRenderer(Window); 
    static bool IsWhite = true; 
    if(IsWhite){ 
        SDL_SetRenderDrawColor(Renderer, 255,255,255,255); 
        IsWhite = false; 
    } else { 
        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        IsWhite = true;  
    }

    SDL_RenderClear(Renderer); 
    SDL_RenderPresent(Renderer); 
}

bool HandleEvent(SDL_Event *Event)
{
    bool ShouldQuit = false;

    switch (Event->type)
    {
    case SDL_WINDOWEVENT:
    {
        SDL_WindowEvent window = Event->window;

        switch (window.event)
        {
        case SDL_WINDOWEVENT_EXPOSED:
        {
            DrawOnWindow(window.windowID); 
        }
        break;

        case SDL_WINDOWEVENT_RESIZED:
        {
            printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", window.data1, window.data2);
        }
        break;
        }
    }
    break;

    case SDL_QUIT:
    {
        printf("SDL QUIT\n");
        ShouldQuit = true;
    }
    break;

    default:
    {
    }
    break;
    }
    return ShouldQuit;
}

int main(int argc, char *argv[])
{

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        // TODO: do something on error
    }

    SDL_Window *Window;
    Window = SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);

    if (Window)
    {
        SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
        if (Renderer)
        {
            while (true) // Main event loop
            {
                SDL_Event Event;
                SDL_WaitEvent(&Event);
                if (HandleEvent(&Event))
                {
                    break; // Quit event loop
                }
            }
        }
    }

    SDL_Quit();
    return (0);
}
