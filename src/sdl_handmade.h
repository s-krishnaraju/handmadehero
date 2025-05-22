

struct sdl_window_dimension {
    int Width;
    int Height;
};

struct circular_audio_buffer {
    uint8 *Buffer;
    int Size;
    int BytesPerSample;
    int WriteCursor;
    int ReadCursor;
    int LatencySampleCount;
    int SamplesPerSecond;
    SDL_AudioDeviceID DeviceID;
};

struct debug_sound_cursor { 
    int PlayCursor; 
    int WriteCursor; 
};

