#pragma once

//basic types
typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef float f32;
typedef double f64;

#ifdef X86
typedef u32 unat;
#endif

#ifdef X64
typedef u64 unat;
#endif

typedef wchar_t wchar;