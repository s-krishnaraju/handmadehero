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

#define ArrayCount(arr) ((int)(sizeof(arr) / sizeof(arr[0])))

// Constants
extern const int NUM_KEYBOARD_CONTROLLERS = 1;
extern const int NUM_GAME_CONTROLLERS = 4;

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
internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize,
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
    bool32 EndedDown;
    int HalfTransition;
};

struct game_controller_input {
    bool32 IsConnected; 
    bool32 IsAnalog;
    real32 StickAverageX;
    real32 StickAverageY;


    // gives access through both ControllerInput.Buttons or
    // ControllerInput.MoveUp
    union { 
        game_button_state Buttons[12];
        struct {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;

            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;

            game_button_state LeftShoulder;
            game_button_state RightShoulder;

            game_button_state Back;
            game_button_state Start;

            // All buttons must be above this line 
            game_button_state Terminator;
        };
    };
};

struct game_input {
    game_controller_input
        Controllers[NUM_KEYBOARD_CONTROLLERS + NUM_GAME_CONTROLLERS];
};

inline game_controller_input *GetGameController(game_input *Input,
                                                int ControllerIndex) {
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    return &Input->Controllers[ControllerIndex];
}

struct game_memory {
    bool32 IsInitialized;
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