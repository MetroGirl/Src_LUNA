// 
// compiler abstraction layer. 
//

#ifndef LUNA_CAL_H_INCLUDED
#define LUNA_CAL_H_INCLUDED

#if defined(_MSC_VER)
#define LUNA_COMPILER_MSC (1)
#else
#error define something.
#endif

#if defined(_M_IX86)
#define LUNA_BITWIDTH_32 (1)
#define LUNA_BITWIDTH_64 (0)
#elif defined(_M_AMD64)
#define LUNA_BITWIDTH_32 (0)
#define LUNA_BITWIDTH_64 (1)
#endif

#endif // LUNA_CAL_H_INCLUDED