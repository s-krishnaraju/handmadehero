#if !defined(HANDMADE_H)

#define internal static
#define global_variable static

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#if HANDMADE_SLOW
#define Assert(Expression)                                                     \
    if (!(Expression)) {                                                       \
        *(int *)0 = 0;                                                         \
    }
#else
#define Assert(Expression)
#endif

typedef float real32;
typedef double real64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

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