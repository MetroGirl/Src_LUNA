//
// platform specific main fucntion.
//

#include "stdafx.h"
#include "entryPoint.h"

#if LUNA_WINDOWS
int WINAPI WinMain(HINSTANCE, HINSTANCE, char*, int)
{
	return luna::entryPoint(__argc, __argv);
}
#else
#error implement main method here.
#endif

