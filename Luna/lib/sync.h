#ifndef LUNA_SYNC_H_INCLUDED
#define LUNA_SYNC_H_INCLUDED

namespace luna{
	class Sync
	{
	public:
		virtual ~Sync() = 0;

		virtual void lock() = 0;
		virtual void unlock() = 0;
	};

	class Mutex : public Sync
	{
	public:
		Mutex()
			: Sync()
#if LUNA_WINDOWS
			, mCS()
#endif
		{
#if LUNA_WINDOWS
			InitializeCriticalSection(&mCS);
#endif
		}

		virtual ~Mutex()
		{
#if LUNA_WINDOWS
			DeleteCriticalSection(&mCS);
#endif
		}

		void lock() override
		{
#if LUNA_WINDOWS
			EnterCriticalSection(&mCS);
#endif
		}

		void unlock() override
		{
#if LUNA_WINDOWS
			LeaveCriticalSection(&mCS);
#endif
		}

	private:
#if LUNA_WINDOWS
		CRITICAL_SECTION mCS;
#endif
	};

	class ScopedLock
	{
	public:
		ScopedLock(Sync& mutex)
			: mMutex(mutex)
		{
			mMutex.lock();
		}

		~ScopedLock()
		{
			mMutex.unlock();
		}

	private:
		Sync& mMutex;
	};
}

#endif // LUNA_SYNC_H_INCLUDED
