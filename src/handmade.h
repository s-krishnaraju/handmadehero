/*
    NOTE:

    HANDMADE_INTERNAL
        0 = Build for public release
        1 = Build for developer only

    HANDMADE_SLOW
        0 = No slow code allowed
        1 = Slow code allowed!
*/

#if !defined(HANDMADE_H)

#if HANDMADE_SLOW

#define Assert(Expression)                                                     \
    if (!(Expression)) {                                                       \
        *(int *)0 = 0;                                                         \
    }
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

inline uint32 SafeTrunacateUint64(uint64 Value) {
    // check if less than 4GB
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return Result;
}

struct debug_read_file_result {
    uint32 ContentSize;
    void *Contents;
};
#if HANDMADE_INTERNAL
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal void DEBUGPlatformFreeFileMemory(void *Memory);
internal bool DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize,
                                           void *Memory);
#endif

struct game_offscreen_buffer {
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct game_sound_output_buffer {
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};

struct game_button_state {
    bool EndedDown;
    int HalfTransition;
};

struct game_controller_input {
    bool IsAnalog;
    real32 StartX;
    real32 EndX;
    real32 MinX;
    real32 MaxX;
    real32 StartY;
    real32 EndY;
    real32 MinY;
    real32 MaxY;

    // gives access through both ControllerInput.Buttons or ControllerInput.Up
    union {
        game_button_state Buttons[6];
        struct {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
        };
    };
};

struct game_input {
    game_controller_input Controllers[4];
};

struct game_memory {
    bool IsInitialized;
    uint64 PermanentStorageSize;
    void *PermanentStorage;
    uint64 TransientStorageSize;
    void *TransientStorage;
};

internal void GameUpdateAndRender(game_memory *Memory,
                                  game_offscreen_buffer *BackBuffer,
                                  game_sound_output_buffer *Sound,
                                  game_input *Input);

struct game_state {
    int SampleIndex;
    int BlueOffset;
    int GreenOffset;
    int ToneHz;
    int ToneVolume;
};

#define HANDMADE_H
#endif