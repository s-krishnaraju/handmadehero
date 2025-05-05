#if !defined(HANDMADE_H)
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
  int16* Samples; 
};





#define internal static
#define global_variable static

#define HANDMADE_H
#endif