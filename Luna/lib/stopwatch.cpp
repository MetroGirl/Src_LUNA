//
// Stopwatch
// 
//

#include "stdafx.h"
#include "stopwatch.h"

namespace luna{
	s64 StopWatch::getFreq()
	{
#if LUNA_WINDOWS
		LARGE_INTEGER mBaseFreq;
		QueryPerformanceFrequency(&mBaseFreq);
		return mBaseFreq.QuadPart;
#else
		return 0;
#endif
	}

	s64 StopWatch::getCounter()
	{
#if LUNA_WINDOWS
		LARGE_INTEGER mTickTime;
		QueryPerformanceCounter(&mTickTime);
		return mTickTime.QuadPart;
#else
		return 0;
#endif
	}
}
