// 
// build configuration abstraction layer. 
//

#ifndef LUNA_BAL_H_INCLUDED
#define LUNA_BAL_H_INCLUDED

#if defined(_DEBUG)
#define LUNA_DEBUG   (1)
#define LUNA_PUBLISH (0)
#elif defined(PUBLISH)
#define LUNA_DEBUG   (0)
#define LUNA_PUBLISH (1)
#else
#define LUNA_DEBUG   (0)
#define LUNA_PUBLISH (0)
#endif

#endif // LUNA_BAL_H_INCLUDED