//
// Stopwatch
//

#ifndef LUNA_STOPWATCH_H_INCLUDED
#define LUNA_STOPWATCH_H_INCLUDED

#include "lib/type.h"

namespace luna{
	class StopWatch{
	public:
		StopWatch()
		{
			Start();
		}

		void Start()
		{
			mBegin = getCounter();
			mStopped = false;
		}

		void Reset()
		{
			Start();
		}

		void Stop()
		{
			mEnd = getCounter();
			mStopped = true;
		}

		f32 getElapsedMilliseconds()
		{
			return getElapsedSeconds() * 1000.f;
		}

		f32 getElapsedSeconds()
		{
			const s64 endRef = mStopped ? mEnd : getCounter();
			return ((endRef - mBegin) / static_cast<f32>(getFreq()));
		}

	private:
		static s64 getFreq();
		static s64 getCounter();

	private:
		s64 mBegin;
		s64 mEnd;
		bool mStopped;
	};
}

#endif // LUNA_STOPWATCH_H_INCLUDED
